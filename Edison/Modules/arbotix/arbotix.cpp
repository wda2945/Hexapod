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

#include "hexapod.h"

#include "mraa.h"
#include "mraa_internal_types.h"

#include "hex_msg.h"
#include "arbotix/arbotix.hpp"

FILE *arbotixDebugFile;

#ifdef ARBOTIX_DEBUG
#define DEBUGPRINT(...) tprintf( __VA_ARGS__);tfprintf(arbotixDebugFile, __VA_ARGS__);
#else
#define DEBUGPRINT(...) tfprintf(arbotixDebugFile, __VA_ARGS__);
#endif

#define ERRORPRINT(...) tprintf(__VA_ARGS__);tfprintf(arbotixDebugFile, __VA_ARGS__);

//copies of relevant hexapod messages received from Arbotx
message_t lastStatusMessage, lastFailMessage, lastVoltsMessage;

void *ArbRxThread(void *a);					//receives messages from ArbotixM
void *ArbTimeoutThread(void *a);			//checks for timeout conditions
time_t actionTimeout, statusTimeout;

uint16_t movementAbortFlags;

//copy of last walk command sent to Arbotix
message_t lastWalkMessage;
struct timeval lastWalkMessageTime;

int arbotixWalkSpeed = 50;
int arbotixTurnSpeed = 50;

//command helpers
ActionResult_enum ActionTurn(int degrees);
ActionResult_enum ActionTurnTo(int degrees);
ActionResult_enum ActionMove(int cms);

int RandomTurn();
int RandomMove();

void SendGetStatus();						//ask for status
void SendActionCommand(msgType_enum m);		//send a command
void SendGetPose(int leg);					//get a pose back
void SendArbMessage(message_t *arbMsg);		//send message to Arbotix

void ReportHexapodStatus(ArbotixState_enum s);	//report Arbotix status to system

ArbotixState_enum arbotixState = ARBOTIX_OFFLINE;	//latest reported from Arbotix state machine
const char *arbotixStateNames[] = ARBOTIX_STATE_NAMES;

//names for log messages
static const char *servoNames[] = SERVO_NAMES;
static const char *msgTypes_L[] = DEBUG_MSG_L;	//Arbotix message types

mraa_uart_context uartContext;
int arbUartFD;								//Arbotix uart file descriptor

int ArbotixInit() {
	struct termios settings;

	arbotixDebugFile = fopen_logfile("arbotix");

	//set up serial port
#ifdef ARB_UART_RAW
	uartContext = mraa_uart_init_raw(ARB_UART_DEVICE);
	if (uartContext == 0) {
		ERRORPRINT("mraa_uart_init_raw(%s) fail\n", ARB_UART_DEVICE);
		return -1;
	}
#else
	uartContext = mraa_uart_init(ARB_UART_DEVICE);
	if (uartContext == 0) {
		ERRORPRINT("mraa_uart_init(%i) fail\n", ARB_UART_DEVICE);
		return -1;
	}
#endif

	arbUartFD = uartContext->fd;

	if (tcgetattr(arbUartFD, &settings) != 0) {
		ERRORPRINT("tcgetattr: %s\n", strerror(errno));
		return -1;
	}

	//no processing
	settings.c_iflag = 0;
	settings.c_oflag = 0;
	settings.c_lflag = 0;
	settings.c_cflag = CLOCAL | CREAD | CS8;        //no modem, 8-bits

	if (tcsetattr(arbUartFD, TCSANOW, &settings) != 0) {
		ERRORPRINT("uart: tcsetattr error - %s\n", strerror(errno));
		return -1;
	}

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

	DEBUGPRINT("Arbotix uart %s configured\n", uartContext->path);

	ps_registry_add_new("Status", "Arbotix", PS_REGISTRY_TEXT_TYPE, PS_REGISTRY_SRC_WRITE);
	ps_registry_set_text("Status", "Arbotix", arbotixStateNames[arbotixState]);

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

//	ps_subscribe( SYS_ACTION_TOPIC, ArbotixProcessMessage);

	statusTimeout = actionTimeout = -1;
	return 0;
}

ActionResult_enum 	lastResult = ACTION_SUCCESS;
ArbAction_enum		lastAction = HEXAPOD_NULL;

//call from Behavior Tree leaf (see callbacks_arbotix.cpp)
ActionResult_enum HexapodExecuteAction(ArbAction_enum _action)
{
	ActionResult_enum result = ACTION_FAIL;

	//check last result to see if a command is still RUNNING
	switch (lastResult)
	{
	case ACTION_SUCCESS:
	case ACTION_FAIL:
		//expecting a new action
		switch(arbotixState)
		{
		case ARBOTIX_READY:
		case ARBOTIX_ERROR:		//worth a try
		{
			//ready for an action
			switch (_action)
			{
			case HEXAPOD_SIT:
					SendActionCommand(MSG_TYPE_SIT);
					result = ACTION_RUNNING;
					break;
			case HEXAPOD_TURN_LEFT:
				result = ActionTurn(-RandomTurn());
				break;
			case HEXAPOD_TURN_RIGHT:
				result = ActionTurn(RandomTurn());
				break;
			case HEXAPOD_TURN_LEFT_90:
				result = ActionTurn(-90);
				break;
			case HEXAPOD_TURN_RIGHT_90:
				result = ActionTurn(90);
				break;
			case HEXAPOD_TURN_N:
				result = ActionTurnTo(0);
				break;
			case HEXAPOD_TURN_S:
				result = ActionTurnTo(180);
				break;
			case HEXAPOD_TURN_E:
				result = ActionTurnTo(90);
				break;
			case HEXAPOD_TURN_W:
				result = ActionTurnTo(270);
				break;
			case HEXAPOD_MOVE_FORWARD:
				result = ActionMove(RandomMove());
				break;
			case HEXAPOD_MOVE_BACKWARD:
				result = ActionMove(-RandomMove());
				break;
			case HEXAPOD_MOVE_FORWARD_10:
				result = ActionMove(10);
				break;
			case HEXAPOD_MOVE_BACKWARD_10:
				result = ActionMove(-10);
				break;

			case HEXAPOD_FAST_SPEED:
			case HEXAPOD_MEDIUM_SPEED:
			case HEXAPOD_SLOW_SPEED:
				//set speed
				result = ACTION_SUCCESS;
				break;
			default:
				break;
			}
		}
			break;
		case ARBOTIX_RELAXED:
		case ARBOTIX_TORQUED:
			//sitting, only permit stand
			if (_action != HEXAPOD_STAND) result = ACTION_FAIL;
			else
			{
				SendActionCommand(MSG_TYPE_STAND);
				result = ACTION_RUNNING;
			}
			break;
		default:
			result = ACTION_FAIL;
			break;
		}
		break;
	case ACTION_RUNNING:
		//waiting for a command to complete
		//check which, for ending condition
		switch(lastAction)
		{
		case HEXAPOD_STAND:
		{
			//done if ready
			switch(arbotixState)
			{
			case ARBOTIX_STANDING:
			case ARBOTIX_WAITING:
				result = ACTION_RUNNING;
				break;
			case ARBOTIX_READY:
				result = ACTION_SUCCESS;
				break;
			default:
				result = ACTION_FAIL;
				break;
			}
		}
		break;
		case HEXAPOD_SIT:
		{
			//done if sitting
			switch(arbotixState)
			{
			case ARBOTIX_SITTING:
			case ARBOTIX_WAITING:
				result = ACTION_RUNNING;
				break;
			case ARBOTIX_RELAXED:
			case ARBOTIX_TORQUED:
				result = ACTION_SUCCESS;
				break;
			default:
				result = ACTION_FAIL;
				break;
			}
		}
		break;
		default:
		{
			//otherwise, done when ready
			switch(arbotixState)
			{
			case ARBOTIX_WALKING:
			case ARBOTIX_WAITING:
				result = ACTION_RUNNING;
				break;
			case ARBOTIX_READY:
				result = ACTION_SUCCESS;
				break;
			default:
				result = ACTION_FAIL;
				break;
			}
		}
		break;
		}
	}
	lastAction = _action;
	return (lastResult = result);
}


//thread to listen for messages from ArbotixM
void *ArbRxThread(void *a) {
	message_t arbMsg;

	unsigned char c;
	int count;
	int csum;
	unsigned int i;

	DEBUGPRINT("Arbotix Rx thread ready\n");

	SendGetStatus();

	while (1) {
		//read next message from ArbotixM
		unsigned char *msgChars = (unsigned char *) &arbMsg;

		memset(&arbMsg, 0, sizeof(message_t));

		//scan for '@'
		c = 0;
		while (c != MSG_START)
		{
			count = read(arbUartFD, &c, 1);
		}

		DEBUGPRINT("arbotix: message start\n");

		count = sizeof(message_t);
		do {
			int readchars;

			do {
				readchars = read(arbUartFD, msgChars, count);
			} while (readchars == EAGAIN);

			if (readchars >= 0)
			{
				count -= readchars;
				msgChars += readchars;
			}
			else break;
		}
		while (count > 0);

		if (count < 0) DEBUGPRINT("arbotix: read error: %s\n", strerror(count));

		if (count == 0) {
			//whole payload read
			msgChars =  (unsigned char *)  &arbMsg;

			DEBUGPRINT("arbotix read: %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", msgChars[0], msgChars[1], msgChars[2], msgChars[3], msgChars[4], msgChars[5], msgChars[6], msgChars[7], msgChars[8]);

			//calculate checksum
			csum = 0;
			for (i = 0; i < sizeof(message_t); i++) {
				csum += *msgChars++;
			}
			//read checksum
			count = 0;
			do {
				count = read(arbUartFD, &c, 1);
			}  while (count == 0 || count == EAGAIN);

			if (count == 1 && c == (csum & 0xff)) {
				//valid checksum
				if (arbMsg.msgType < MSG_TYPE_COUNT)
				{
					DEBUGPRINT("Arbotix Rx: %s\n", msgTypes_L[arbMsg.msgType]);
				}
				else
				{
					ERRORPRINT("arbotix: bad msgType: %i -", arbMsg.msgType);
				}
				switch (arbMsg.msgType) {
				case MSG_TYPE_STATUS:    //Status report payload
					if (arbMsg.state < AX_STATE_COUNT)
					{
						DEBUGPRINT("arbotix: status message: %s\n", arbotixStateNames[arbMsg.state])
					}
					else
					{
						DEBUGPRINT("arbotix: bad status message: %u\n", arbMsg.state);
					}
					lastStatusMessage = arbMsg;
					statusTimeout = time(NULL) + arbTimeout;
					ReportHexapodStatus((ArbotixState_enum) arbMsg.state);

					switch(arbotixState)
					{
					case ARBOTIX_STOPPING:		 //in process of stopping
					case ARBOTIX_SITTING:        //in process of sitting down
					case ARBOTIX_STANDING:       //in process of standing
						actionTimeout = time(NULL) + arbTimeout;
						break;
					default:
						actionTimeout = -1;
						break;
					}
					break;
					case MSG_TYPE_POSE:      //leg position payload
						DEBUGPRINT("arbotix: Leg %i: X=%i Y=%i Z=%i\n", arbMsg.legNumber, arbMsg.x, arbMsg.y, arbMsg.z );
						break;
					case MSG_TYPE_SERVOS:    //Servo payload
						DEBUGPRINT("arbotix: Leg %i: Coxa=%i Femur=%i Tibia=%i\n", arbMsg.legNumber, arbMsg.coxa, arbMsg.femur, arbMsg.tibia );
						break;
					case MSG_TYPE_MSG:       //Text message
						DEBUGPRINT("arbotix: %4s\n", arbMsg.text);
						break;
					case MSG_TYPE_ODOMETRY: {
						psMessage_t msg;
						psInitPublish(msg, ODOMETRY);
						msg.odometryPayload.xMovement = arbMsg.xMovement;
						msg.odometryPayload.yMovement = arbMsg.yMovement;
						msg.odometryPayload.zRotation = arbMsg.zRotation;
						NewBrokerMessage(msg);
					}
					break;
					case MSG_TYPE_VOLTS:     //Text message
						lastVoltsMessage = arbMsg;
						DEBUGPRINT("arbotix: batt: %0.1fV\n",
								(float) (arbMsg.volts / 10));
						break;
					case MSG_TYPE_FAIL:
						ERRORPRINT("arbotix: Fail: %s = %i\n", servoNames[arbMsg.servo], arbMsg.angle);
						ReportHexapodStatus(ARBOTIX_ERROR);
						break;
					case MSG_TYPE_ERROR:
					{
						if (arbMsg.errorCode >= 0 && arbMsg.errorCode < MSG_TYPE_COUNT)
						{
							if (arbMsg.errorCode < MSG_TYPE_UNKNOWN)
							{
								DEBUGPRINT("arbotix: Debug: %s\n", msgTypes_L[arbMsg.errorCode]);
							}
							else
							{
								DEBUGPRINT("arbotix: Error: %s\n", msgTypes_L[arbMsg.errorCode]);
								switch(arbMsg.errorCode)
								{
								case LOW_VOLTAGE:
									ReportHexapodStatus(ARBOTIX_LOWVOLTAGE);
									break;
								default:
									ReportHexapodStatus(ARBOTIX_ERROR);
									break;
								}
							}
						}
						else
						{
							DEBUGPRINT("arbotix: debug: %i?\n", arbMsg.errorCode);
						}
					}
					break;
					default:
						break;
				}
			}
			else
			{
				msgChars =  (unsigned char *)  &arbMsg;
				ERRORPRINT("arbotix checksum! %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", msgChars[0], msgChars[1], msgChars[2], msgChars[3], msgChars[4], msgChars[5], msgChars[6], msgChars[7], msgChars[8]);
				ERRORPRINT("arbotix: sum=%x. Expected %02x, got %02x\n", csum, (csum & 0xff), c);
			}
		}
	}
	return NULL;
}

//thread to check for ArbotixM command timeouts
void *ArbTimeoutThread(void *a)
{
	DEBUGPRINT("arbotix Timeout thread started\n");

	while (1)
	{
		//check for timeouts and errors
		switch(arbotixState)
		{
		case ARBOTIX_STOPPING:		 //in process of stopping
		case ARBOTIX_SITTING:        //in process of sitting down
		case ARBOTIX_STANDING:       //in process of standing
			if (actionTimeout > 0 && actionTimeout < time(NULL))
			{
				ERRORPRINT("arbotix: Timeout");
				ReportHexapodStatus(ARBOTIX_TIMEOUT);
				SendGetStatus();
			}
			break;
		default:
			//error states
		case ARBOTIX_STATE_UNKNOWN:	//start up
		case ARBOTIX_TIMEOUT:		//no response to command
		case ARBOTIX_ERROR:			//error reported
			if (statusTimeout < time(NULL))
			{
				DEBUGPRINT("arbotix: timeout GetStatus");
				SendGetStatus();
			}

			break;
		}
		sleep(1);
	}
	return 0;
}

//write byte
int writeByte(unsigned char c)
{
	int written = 0;
	do
	{
		written = write(arbUartFD, &c, 1);
	} while (written == EAGAIN || written == 0);

	return written;
}

//command helpers
//turn X degrees
ActionResult_enum ActionTurn(int degrees)
{
	if (abs(degrees) < 5) return ACTION_SUCCESS;

	message_t arbMsg;
	memset(&arbMsg, 0, sizeof(message_t));
	arbMsg.msgType = MSG_TYPE_WALK;
	arbMsg.zRotateSpeed = (degrees > 0 ? arbotixTurnSpeed : -arbotixTurnSpeed);
	arbMsg.steps = abs(degrees / degreesPerStep);

	SendArbMessage(&arbMsg);
	statusTimeout = time(NULL) + 2;		//give it 2 seconds to answer

	ReportHexapodStatus(ARBOTIX_WAITING);
	return ACTION_RUNNING;
}
//turn to X heading
ActionResult_enum ActionTurnTo(int degrees)
{
	bool imuOnline = ps_test_condition(SOURCE, IMU_ONLINE);

	if (!imuOnline)
	{
		lastLuaCallReason = "imu offline";
		return ACTION_FAIL;
	}
	int heading = 0;
	if (ps_registry_get_int("pose", "heading", &heading) != PS_OK)
	{
		lastLuaCallReason = "no heading";
		return ACTION_FAIL;
	}

	return ActionTurn(degrees - heading);
}
ActionResult_enum ActionMove(int cms)
{
	if (abs(cms) < 5) return ACTION_SUCCESS;

	message_t arbMsg;
	memset(&arbMsg, 0, sizeof(message_t));

	arbMsg.msgType = MSG_TYPE_WALK;
	arbMsg.xSpeed = (cms > 0 ? arbotixWalkSpeed : -arbotixWalkSpeed);
	arbMsg.steps = abs(cms / cmPerStep);

	SendArbMessage(&arbMsg);
	statusTimeout = time(NULL) + 2;		//give it 2 seconds to answer

	ReportHexapodStatus(ARBOTIX_WAITING);
	return ACTION_RUNNING;
}
int RandomTurn()
{
	float turn = defTurn + RANDOM_TURN_SPREAD * (drand48() - 0.5);
	return (int) turn;
}

int RandomMove()
{
	float move = defMove + RANDOM_MOVE_SPREAD * (drand48() - 0.5);
	return (int) move;
}


//send message to ArbotixM
pthread_mutex_t	arbMsgMtx = PTHREAD_MUTEX_INITIALIZER;
void SendArbMessage(message_t *arbMsg) {

	int csum = 0;
	unsigned int i;
	unsigned char *msgChars = (unsigned char*) arbMsg;

	DEBUGPRINT("arbotix write: %02x %02x %02x %02x %02x %02x %02x %02x %02x", msgChars[0], msgChars[1], msgChars[2], msgChars[3], msgChars[4], msgChars[5], msgChars[6], msgChars[7], msgChars[8]);

	//critical section
	int m = pthread_mutex_lock(&arbMsgMtx);
	if (m != 0)
	{
		ERRORPRINT("arbMsgMtx lock %i", m);
	}

	writeByte(MSG_START);

	for (i=0; i<sizeof(message_t); i++)
	{
		writeByte(*msgChars);
		csum += *msgChars++;
	}

	writeByte(csum & 0xff);

	m = pthread_mutex_unlock(&arbMsgMtx);
	if (m != 0)
	{
		ERRORPRINT("arbMsgMtx unlock %i", m);
	}
	//end critical section

	DEBUGPRINT("Arb Tx: %s", msgTypes_L[arbMsg->msgType]);
}

//functions to send various messages to the ArbotixM
void SendGetStatus() {
	message_t arbMsg;
	memset(&arbMsg, 0, sizeof(message_t));
	arbMsg.msgType = MSG_TYPE_GETSTATUS;
	SendArbMessage(&arbMsg);
	statusTimeout = time(NULL) + 2;		//give it 2 seconds to answer
}
//send command only
void SendActionCommand(msgType_enum m) {
	message_t arbMsg;
	memset(&arbMsg, 0, sizeof(message_t));
	arbMsg.msgType = m;
	SendArbMessage(&arbMsg);
	actionTimeout = time(NULL) + arbTimeout;		//give it 5 seconds or so
}
//send GET_POSE command plus legNumber
void SendGetPose(int leg) {
	message_t arbMsg;
	memset(&arbMsg, 0, sizeof(message_t));
	arbMsg.legNumber = leg;
	arbMsg.msgType = MSG_TYPE_GET_POSE;
	SendArbMessage(&arbMsg);
}

//report Arbotix Status
pthread_mutex_t	arbStateMtx = PTHREAD_MUTEX_INITIALIZER;
void ReportHexapodStatus(ArbotixState_enum s) {
	if (arbotixState != s)
	{
		ps_registry_set_text("Status", "Arbotix", arbotixStateNames[s]);

		//critical section
		int m = pthread_mutex_lock(&arbStateMtx);
		if (m != 0)
		{
			ERRORPRINT("arbStateMtx lock %i", m);
		}

		arbotixState = s;

		DEBUGPRINT("State %s", arbotixStateNames[s]);

		switch (arbotixState)
		{
		case ARBOTIX_RELAXED:        //torque off										== BUG_AX_ON
		case ARBOTIX_TORQUED:        //just powered on: in sitting position: torque on	== BUG_AX_ON
		case ARBOTIX_OFFLINE:        //not powered
			ps_set_condition(SERVOS_OFFLINE);
			ps_cancel_condition(SERVOS_READY);
			ps_cancel_condition(SERVOS_ERRORS);
			ps_cancel_condition(SERVOS_WALKING);
			break;
		case ARBOTIX_READY:          //standing up: stopped and ready 					== BUG_STANDBY
			ps_set_condition(SERVOS_READY);
			ps_cancel_condition(SERVOS_OFFLINE);
			ps_cancel_condition(SERVOS_ERRORS);
			ps_cancel_condition(SERVOS_WALKING);
			break;
		case ARBOTIX_WALKING:        //walking											== BUG_ACTIVE
			ps_set_condition(SERVOS_WALKING);
			ps_cancel_condition(SERVOS_READY);
			ps_cancel_condition(SERVOS_ERRORS);
			ps_cancel_condition(SERVOS_OFFLINE);
			break;
		case ARBOTIX_LOWVOLTAGE:     //low volts shutdown
		case ARBOTIX_TIMEOUT:        //no response to command
		case ARBOTIX_ERROR:
			ps_set_condition(SERVOS_ERRORS);
			ps_cancel_condition(SERVOS_WALKING);
			ps_cancel_condition(SERVOS_READY);
			ps_cancel_condition(SERVOS_OFFLINE);
			break;
		case ARBOTIX_WAITING:
		case ARBOTIX_POSING:         //posing leg control
		case ARBOTIX_STOPPING:       //halt sent
		case ARBOTIX_SITTING:        //in process of sitting down
		case ARBOTIX_STANDING:       //in process of standing
		case ARBOTIX_STATE_UNKNOWN:  //start up
		default:
			break;
		}

		m = pthread_mutex_unlock(&arbStateMtx);
		if (m != 0)
		{
			ERRORPRINT("arbStateMtx unlock %i", m);
		}
		//end critical section
	}
}

