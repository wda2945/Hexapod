/*
 ============================================================================
 Name        : arbotix.c
 Author      : Martin
 Version     :
 Copyright   : (c) 2013 Martin Lane-Smith
 Description : Exchanges messages with the ArbotixM subsystem
 	 	 	   Receives MOVE messages, and publishes ODOMETRY messages
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#include "mraa.h"
#include "mraa_internal_types.h"

#include "hex_msg.h"

#include "softwareProfile.h"
#include "pubsubdata.h"
#include "pubsub/pubsub.h"
#include "syslog/SysLog.h"
#include "pubsub/notifications.h"
#include "i2c_task/i2c_task.h"
#include "arbotix/arbotix.h"

FILE *arbotixDebugFile;

#ifdef ARBOTIX_DEBUG
#define DEBUGPRINT(...) tprintf( __VA_ARGS__);tfprintf(arbotixDebugFile, __VA_ARGS__);
#else
#define DEBUGPRINT(...) tfprintf(arbotixDebugFile, __VA_ARGS__);
#endif

#define ERRORPRINT(...) tprintf(__VA_ARGS__);tfprintf(arbotixDebugFile, __VA_ARGS__);

int systemPowerState;

//copies of relevant messages
message_t lastStatusMessage, lastFailMessage, lastVoltsMessage;

//copy of last walk command
message_t lastWalkMessage;
int stepsRemaining;					//for this walk
struct timeval lastWalkMessageTime;

void *ArbRxThread(void *a);					//receives messages from ArbotixM

void *ArbTimeoutThread(void *a);			//checks for timeout conditions
time_t actionTimeout, statusTimeout;

void SendGetStatus();						//ask for status
void SendActionCommand(msgType_enum m);		//send a command
void SendGetPose(int leg);					//get a pose back
int SendArbMessage(message_t *arbMsg);		//send message to Arbotix

void CheckBugState();						//compare Arbotix and system states
void ReportArbStatus(ArbotixState_enum s);	//report Arbotix status to system
void SetArbotixPower(bool enable);

int arbUartFD;								//Arbotix uart file descriptor

ArbotixState_enum 	arbState = ARB_OFFLINE;	//latest reported from Arbotix state machine

//names for log messages
static char *servoNames[] = SERVO_NAMES;
static char *msgTypes_L[] = DEBUG_MSG_L;	//Arbotix message types

int lowVoltageError = false;

mraa_uart_context uartContext;
int arbotixFD;

int ArbotixInit() {

	arbotixDebugFile = fopen_logfile("arbotix");
	DEBUGPRINT("Arbotix Logfile opened\n");

	//set up serial port
	uartContext = mraa_uart_init_raw(ARB_UART_DEVICE);
	if (uartContext == 0) {
		ERRORPRINT("mraa_uart_init_raw(%s) fail\n", ARB_UART_DEVICE);
		return -1;
	}

	arbotixFD = uartContext->fd;

	if (mraa_uart_set_baudrate(uartContext, ARB_UART_BAUDRATE) != MRAA_SUCCESS)
	{
		ERRORPRINT("Arbotix mraa_uart_set_baudrate() fail\n");
		return -1;
	}
	if (mraa_uart_set_mode(uartContext, 8, MRAA_UART_PARITY_NONE, 1) != MRAA_SUCCESS)
	{
		ERRORPRINT("Arbotix mraa_uart_set_mode() fail\n");
		return -1;
	}
	if (mraa_uart_set_flowcontrol(uartContext, false, false) != MRAA_SUCCESS)
	{
		ERRORPRINT("Arbotix mraa_uart_set_mode() fail\n");
		return -1;
	}

	DEBUGPRINT("Arbotix uart %s configured\n", ARB_UART_DEVICE);

	//create thread to receive Arbotix messages
	pthread_t thread;
	int s = pthread_create(&thread, NULL, ArbRxThread, NULL);
	if (s != 0) {
		ERRORPRINT("Rx: pthread_create %i %s\n", s, strerror(errno));
		return errno;
	}

	//create thread for timeouts
	s = pthread_create(&thread, NULL, ArbTimeoutThread, NULL);
	if (s != 0) {
		ERRORPRINT("T/O: pthread_create %i %s\n", s, strerror(errno));
		return errno;
	}

	statusTimeout = actionTimeout = -1;
	return 0;
}

//function to handle for messages from the system
void ArbotixProcessMessage(psMessage_t *msg) {

	DEBUGPRINT("Arbotix: %s\n", psLongMsgNames[msg->header.messageType]);

	switch (msg->header.messageType)
	{
	case MOVE:					//from Overmind
		DEBUGPRINT("MOVE msg\n");

		switch (arbState)
		{
		case ARB_READY:
		case ARB_WALKING:
			lastWalkMessage.msgType = MSG_TYPE_WALK;
			lastWalkMessage.xSpeed = (int8_t) msg->threeFloatPayload.xSpeed;
			lastWalkMessage.ySpeed = (int8_t) msg->threeFloatPayload.ySpeed;
			lastWalkMessage.zRotateSpeed = (int8_t) msg->threeFloatPayload.zRotateSpeed;
			SendArbMessage(&lastWalkMessage);
			break;
		case ARB_STOPPING:
			break;
		default:
			ERRORPRINT("MOVE in %s", arbStateNames[arbState]);
			break;
		}

		break;
		case TICK_1S:
			CheckBugState();		//check for action needed
			break;
	}
}

//thread to listen for messages from ArbotixM
void *ArbRxThread(void *a) {
	message_t arbMsg;

	unsigned char c;
	int count;
	int csum;
	int i;

	unsigned char *msgChars = (unsigned char *) &arbMsg.intValues;
	int msgLen = MSG_INTS * 2;

	DEBUGPRINT("Arbotix Rx thread ready\n");

	SendGetStatus();

	while (1) {
		//read next message from Arbotix
		for (i = 0; i < MSG_LEN; i++) {
			msgChars[i] = 0;
		}
		//scan for '@'
		c = 0;
		while (c != MSG_START)
		{
			count = read(arbUartFD, &c, 1);
		}

		DEBUGPRINT("arbotix: message start\n");

		//read message type
		count = 0;
		while (count == 0)
		{
			count = read(arbUartFD, &arbMsg.msgType, 1);
		}
		//read rest
		count = 0;
		while (count < msgLen)
		{
			count += read(arbUartFD, msgChars + count , msgLen - count);
		}
		if (count == msgLen) {
			//valid length
			csum = arbMsg.msgType;
			//calculate checksum
			for (i = 0; i < msgLen; i++) {
				csum += msgChars[i];
			}
			//read checksum
			count = 0;
			while (count == 0)
			{
				count = read(arbUartFD, &c, 1);
			}
			if (count == 1 && c == (csum & 0xff)) {
				//valid checksum
				if (arbMsg.msgType < MSG_TYPE_COUNT)
				{
					DEBUGPRINT("Arbotix Rx: %s\n", msgTypes_L[arbMsg.msgType]);
				}
				else
				{
					ERRORPRINT("bad msgType: %i -", arbMsg.msgType);
					for (i = 0; i < MSG_INTS; i++) {
						ERRORPRINT(" %i", arbMsg.intValues[i]);
					}
					ERRORPRINT("\n");
				}
				switch (arbMsg.msgType) {
				case MSG_TYPE_STATUS:    //Status report payload
					if (arbMsg.state < AX_STATE_COUNT)
					{
						DEBUGPRINT("arbotix: status message: %s\n", arbStateNames[arbMsg.state])
					}
					else
					{
						DEBUGPRINT("arbotix: status message: %u\n", arbMsg.state);
					}
					lastStatusMessage = arbMsg;
					statusTimeout = time(NULL) + arbTimeout;
					ReportArbStatus(arbMsg.state);
					CheckBugState();

					switch(arbState)
					{
					case ARB_STOPPING:		 //in process of stopping
					case ARB_SITTING:        //in process of sitting down
					case ARB_STANDING:       //in process of standing
						actionTimeout = time(NULL) + arbTimeout;
						break;
					default:
						actionTimeout = -1;
						break;
					}
					break;
					case MSG_TYPE_POSE:      //leg position payload
						DEBUGPRINT("Leg %i: X=%i Y=%i Z=%i\n", arbMsg.legNumber, arbMsg.x, arbMsg.y, arbMsg.z );
						break;
					case MSG_TYPE_SERVOS:    //Servo payload
						DEBUGPRINT("Leg %i: Coxa=%i Femur=%i Tibia=%i\n", arbMsg.legNumber, arbMsg.coxa, arbMsg.femur, arbMsg.tibia );
						break;
					case MSG_TYPE_MSG:       //Text message
						DEBUGPRINT("arbotix: %4s\n", arbMsg.text);
						break;
					case MSG_TYPE_ODOMETRY: {
						psMessage_t msg;
						if (stepsRemaining > 0)
							stepsRemaining--;
						psInitPublish(msg, ODOMETRY);
						msg.odometryPayload.xMovement = arbMsg.xMovement;
						msg.odometryPayload.yMovement = arbMsg.yMovement;
						msg.odometryPayload.zRotation = arbMsg.zRotation;

						NewBrokerMessage(&msg);
					}
					break;
					case MSG_TYPE_VOLTS:     //Text message
						lastVoltsMessage = arbMsg;
						DEBUGPRINT("arbotix: batt: %0.1fV\n",
								(float) (arbMsg.volts / 10));
						break;
					case MSG_TYPE_FAIL:
						ERRORPRINT("arb: Fail: %s = %i\n", servoNames[arbMsg.servo], arbMsg.angle);
						break;
					case MSG_TYPE_ERROR:
					{
						if (arbMsg.errorCode >= 0 && arbMsg.errorCode < MSG_TYPE_COUNT)
						{
							if (arbMsg.errorCode < MSG_TYPE_UNKNOWN)
							{
								DEBUGPRINT("Debug: %s\n", msgTypes_L[arbMsg.errorCode]);
							}
							else
							{
								DEBUGPRINT("Error: %s\n", msgTypes_L[arbMsg.errorCode]);
								switch(arbMsg.errorCode)
								{
								case LOW_VOLTAGE:
									lowVoltageError = true;
									CheckBugState();
									break;
								default:
									break;
								}
							}
						}
						else
						{
							DEBUGPRINT("debug: %i?\n", arbMsg.errorCode);
						}
					}
					break;
					default:
						break;
				}
			}
			else
			{
				ERRORPRINT("Bad checksum:\n");
				for (i = 0; i < MSG_LEN; i++) {
					ERRORPRINT(" %i", msgChars[i]);
				}

				ERRORPRINT(". sum=%x.\nExpected %02x, got %02x\n", csum, (csum & 0xff), c);
			}
		}
		else
		{
			if (count < 0)
			{
				ERRORPRINT("Read error %s\n", strerror(errno));
			}
			else
			{
				ERRORPRINT("Read %i bytes\n", count);
			}
		}
	}
	return NULL;
}

//thread to check for ArbotixM command timeouts
void *ArbTimeoutThread(void *a)
{
	DEBUGPRINT("Arbotix Timeout thread started\n");

	while (1)
	{
		//check for timeouts and errors
		switch(arbState)
		{
		case ARB_STOPPING:		 //in process of stopping
		case ARB_SITTING:        //in process of sitting down
		case ARB_STANDING:       //in process of standing
			if (actionTimeout > 0 && actionTimeout < time(NULL))
			{
				ERRORPRINT("Timeout\n");
				ReportArbStatus(ARB_TIMEOUT);
				SendGetStatus();
			}
			break;
			//error states
		case ARB_STATE_UNKNOWN:	//start up
		case ARB_TIMEOUT:		//no response to command
		case ARB_ERROR:			//error reported
			if (statusTimeout < time(NULL))
			{
				DEBUGPRINT("Timeout GetStatus\n");
				SendGetStatus();
			}
			break;
		default:
			break;
		}
		sleep(1);
	}
	return 0;
}

//review Arbotix state versus system state and take action
pthread_mutex_t	bugStateMtx = PTHREAD_MUTEX_INITIALIZER;
void CheckBugState()
{
//	DEBUGPRINT("bugSystemState= %s, arbState= %s\n", powerStateNames[systemPowerState],  arbStateNames[arbState]);

	//critical section
	int s = pthread_mutex_lock(&bugStateMtx);
	if (s != 0)
	{
		ERRORPRINT("bugStateMtx lock %i\n", s);
	}
	switch (arbState) {
	case ARB_WALKING:         	//walking
		if (systemPowerState < POWER_ACTIVE || lowVoltageError) {
			//shouldn't be...
			SendActionCommand(MSG_TYPE_HALT);
		}
		break;
	case ARB_READY:       		//standing up and ready
		if (systemPowerState < POWER_STANDBY || lowVoltageError) {
			//shouldn't be...
			SendActionCommand(MSG_TYPE_SIT);
		}
		break;
	case ARB_TORQUED:           //in transient sitting position, torque on
		if (systemPowerState >= POWER_STANDBY && !lowVoltageError) {
			//shouldn't be...
			SendActionCommand(MSG_TYPE_STAND);
		} else if (systemPowerState < POWER_STANDBY || lowVoltageError) {
			//shouldn't be...
			SetArbotixPower(false);
		}
		break;
	case ARB_RELAXED:        	//torque off
		if (systemPowerState >= POWER_STANDBY && !lowVoltageError) {
			//shouldn't be...
			SendActionCommand(MSG_TYPE_TORQUE);
		}
		break;
	case ARB_OFFLINE:
		if (systemPowerState >= POWER_STANDBY && !lowVoltageError) {
			//shouldn't be...
			SetArbotixPower(true);
		}
		break;
	default:
		if (systemPowerState < POWER_STANDBY || lowVoltageError) {
			//shouldn't be...
			SetArbotixPower(false);
		}
		break;
	}

	s = pthread_mutex_unlock(&bugStateMtx);
	if (s != 0)
	{
		ERRORPRINT("bugStateMtx unlock %i\n", s);
	}
	//end critical section
}

//send message to ArbotixM
pthread_mutex_t	arbMsgMtx = PTHREAD_MUTEX_INITIALIZER;
int SendArbMessage(message_t *arbMsg) {
	int count;
	int result = 0;
	int csum = 0;
	int i;

	int dataLen = MSG_INTS * 2;		//payload only
	int msgLen  = dataLen + 3;		//plus @, msgType, csum
	unsigned char msgChars[msgLen];
	msgChars[0] = MSG_START;
	msgChars[1] = arbMsg->msgType;

	memcpy(&msgChars[2], (void*) &arbMsg->intValues, dataLen);

	//calculate checksum
	for (i = 1; i < dataLen+2; i++) {
		csum += msgChars[i];
	}
	msgChars[msgLen-1] = csum & 0xff;

	DEBUGPRINT("Arb Tx: %s\n", msgTypes_L[arbMsg->msgType]);

	//critical section
	int m = pthread_mutex_lock(&arbMsgMtx);
	if (m != 0)
	{
		ERRORPRINT("arbMsgMtx lock %i\n", m);
	}
	count = MSG_LEN+2;
	while (count > 0)
	{
		int written = write(arbUartFD, msgChars + MSG_LEN + 2 - count, count);
		count -= written;
	}
	m = pthread_mutex_unlock(&arbMsgMtx);
	if (m != 0)
	{
		ERRORPRINT("arbMsgMtx unlock %i\n", m);
	}
	//end critical section
	return result;
}

#define ZERO_MSG(X) {int i;for(i=0;i<MSG_INTS;i++) X.intValues[i]=0;}

//functions to send various messages to the ArbotixM
void SendGetStatus() {
	message_t arbMsg;
	ZERO_MSG(arbMsg);
	arbMsg.msgType = MSG_TYPE_GETSTATUS;
	if (SendArbMessage(&arbMsg) == 0)
	{
		statusTimeout = time(NULL) + 2;		//give it 2 seconds to answer
	}
}
//send command only
void SendActionCommand(msgType_enum m) {
	message_t arbMsg;
	ZERO_MSG(arbMsg);
	arbMsg.msgType = m;
	if (SendArbMessage(&arbMsg) == 0)
	{
		actionTimeout = time(NULL) + arbTimeout;		//give it 5 seconds or so
	}
}
//send GET_POSE command plus legNumber
void SendGetPose(int leg) {
	message_t arbMsg;
	ZERO_MSG(arbMsg);
	arbMsg.legNumber = leg;
	arbMsg.msgType = MSG_TYPE_GET_POSE;
	SendArbMessage(&arbMsg);
}

//report Arbotix Status
pthread_mutex_t	arbStateMtx = PTHREAD_MUTEX_INITIALIZER;
void ReportArbStatus(ArbotixState_enum s) {
	if (arbState != s)
	{
		psMessage_t msg;
		//critical section
		int m = pthread_mutex_lock(&arbStateMtx);
		if (m != 0)
		{
			ERRORPRINT("arbStateMtx lock %i\n", m);
		}

		arbState = s;
		psInitPublish(msg, ARB_STATE);
		msg.bytePayload.value = s;
		NewBrokerMessage(&msg);

		DEBUGPRINT("State %s", arbStateNames[s]);

		switch (arbState)
		{
		case ARB_RELAXED:        //torque off										== BUG_AX_ON
		case ARB_TORQUED:        //just powered on: in sitting position: torque on	== BUG_AX_ON
		case ARB_OFFLINE:        //not powered
			SetCondition(SERVOS_OFFLINE);
			CancelCondition(SERVOS_READY);
			CancelCondition(SERVOS_ERRORS);
			CancelCondition(SERVOS_WALKING);
			break;
		case ARB_READY:          //standing up: stopped and ready 					== BUG_STANDBY
			SetCondition(SERVOS_READY);
			CancelCondition(SERVOS_OFFLINE);
			CancelCondition(SERVOS_ERRORS);
			CancelCondition(SERVOS_WALKING);
			break;
		case ARB_WALKING:        //walking											== BUG_ACTIVE
			SetCondition(SERVOS_WALKING);
			CancelCondition(SERVOS_READY);
			CancelCondition(SERVOS_ERRORS);
			CancelCondition(SERVOS_OFFLINE);
			break;
		case ARB_LOWVOLTAGE:     //low volts shutdown
		case ARB_TIMEOUT:        //no response to command
		case ARB_ERROR:
			SetCondition(SERVOS_ERRORS);
			CancelCondition(SERVOS_WALKING);
			CancelCondition(SERVOS_READY);
			CancelCondition(SERVOS_OFFLINE);
			break;
		case ARB_POSING:         //posing leg control
		case ARB_STOPPING:       //halt sent
		case ARB_SITTING:        //in process of sitting down
		case ARB_STANDING:       //in process of standing
		case ARB_STATE_UNKNOWN:  //start up
		default:
			break;
		}

		m = pthread_mutex_unlock(&arbStateMtx);
		if (m != 0)
		{
			ERRORPRINT("arbStateMtx unlock %i\n", m);
		}
		//end critical section
	}
}

//change of power state
void SetArbotixPower(bool enable)
{
}
