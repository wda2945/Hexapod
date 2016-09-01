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
int arbotixTurnSpeed = 25;

#define MAX_POSENAME 50
char lastPoseName[MAX_POSENAME];

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
		ERRORPRINT("arbotix: mraa_uart_init_raw(%s) fail", ARB_UART_DEVICE);
		return -1;
	}
#else
	uartContext = mraa_uart_init(ARB_UART_DEVICE);
	if (uartContext == 0) {
		ERRORPRINT("arbotix: mraa_uart_init(%i) fail", ARB_UART_DEVICE);
		return -1;
	}
#endif

	arbUartFD = uartContext->fd;

	if (tcgetattr(arbUartFD, &settings) != 0) {
		ERRORPRINT("arbotix: tcgetattr: %s", strerror(errno));
		return -1;
	}

	//no processing
	settings.c_iflag = 0;
	settings.c_oflag = 0;
	settings.c_lflag = 0;
	settings.c_cflag = CLOCAL | CREAD | CS8;        //no modem, 8-bits

	if (tcsetattr(arbUartFD, TCSANOW, &settings) != 0) {
		ERRORPRINT("arbotix: tcsetattr error - %s", strerror(errno));
		return -1;
	}

	if (mraa_uart_set_baudrate(uartContext, ARB_UART_BAUDRATE) != MRAA_SUCCESS)
	{
		ERRORPRINT("arbotix: mraa_uart_set_baudrate() fail");
		return -1;
	}
	if (mraa_uart_set_mode(uartContext, 8, MRAA_UART_PARITY_NONE, 1) != MRAA_SUCCESS)
	{
		ERRORPRINT("arbotix: mraa_uart_set_mode() fail");
		return -1;
	}
	if (mraa_uart_set_flowcontrol(uartContext, false, false) != MRAA_SUCCESS)
	{
		ERRORPRINT("arbotix: mraa_uart_set_mode() fail");
		return -1;
	}

	DEBUGPRINT("arbotix: uart %s configured", uartContext->path);

	ps_registry_add_new("Status", "Arbotix", PS_REGISTRY_TEXT_TYPE, PS_REGISTRY_SRC_WRITE);
	ps_registry_set_text("Status", "Arbotix", arbotixStateNames[arbotixState]);

	//create thread to receive Arbotix messages
	pthread_t thread;
	int s = pthread_create(&thread, NULL, ArbRxThread, NULL);
	if (s != 0) {
		ERRORPRINT("arbotix: Rx pthread_create %i %s", s, strerror(errno));
		return errno;
	}

	//create thread for timeouts
	s = pthread_create(&thread, NULL, ArbTimeoutThread, NULL);
	if (s != 0) {
		ERRORPRINT("arbotix: T/O pthread_create %i %s", s, strerror(errno));
		return errno;
	}

	//	ps_subscribe( SYS_ACTION_TOPIC, ArbotixProcessMessage);

	statusTimeout = actionTimeout = -1;
	return 0;
}

ActionResult_enum 	lastResult = ACTION_SUCCESS;
ArbAction_enum		lastAction = HEXAPOD_NULL;
ArbAction_enum		timingAction = HEXAPOD_NULL;

const char *arb_action_names[] = ARB_ACTION_NAMES;

//call from Behavior Tree leaf (see callbacks_arbotix.cpp)
ActionResult_enum HexapodExecuteAction(ArbAction_enum _action)
{
	ActionResult_enum result = ACTION_FAIL;

	if (_action == HEXAPOD_STOP)
	{
		switch(arbotixState)
		{
		case ARBOTIX_WALKING:        //walking
		case ARBOTIX_POSING:         //posing leg moving
		case ARBOTIX_SITTING:        //in process of sitting down
		case ARBOTIX_STANDING:
			SendActionCommand(MSG_TYPE_HALT);
			result = ACTION_RUNNING;
			break;
		case ARBOTIX_WAITING:
			result = ACTION_RUNNING;
			break;
		default:
			result = ACTION_SUCCESS;
			break;
		}
	}

	//check last result to see if a command is still RUNNING
	else switch (lastResult)
	{
	case ACTION_SUCCESS:
	case ACTION_FAIL:
		//expecting a new action
		switch(arbotixState)
		{
		case ARBOTIX_READY:
		case ARBOTIX_ERROR:		//worth a try
		{
//			DEBUGPRINT("hex new action: %s", arb_action_names[_action]);
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
			case HEXAPOD_MOVE_FORWARD_30:
				result = ActionMove(30);
				break;
			case HEXAPOD_MOVE_BACKWARD_30:
				result = ActionMove(-30);
				break;
			case HEXAPOD_FAST_SPEED:
			case HEXAPOD_MEDIUM_SPEED:
			case HEXAPOD_SLOW_SPEED:
				//set speed
				result = ACTION_SUCCESS;
				break;
			case HEXAPOD_POSE_MODE:
				SendActionCommand(MSG_TYPE_POSEMODE);
				result = ACTION_RUNNING;
				break;
			case HEXAPOD_POSE_SLOW:
			case HEXAPOD_POSE_MEDIUM:
			case HEXAPOD_POSE_FAST:
			case HEXAPOD_POSE_BEAT:
			case HEXAPOD_POSE_DOWNBEAT:
			case HEXAPOD_POSE_UPBEAT:
				timingAction = _action;
				break;
			default:
				break;
			}
		}
		break;
		case ARBOTIX_RELAXED:
		case ARBOTIX_TORQUED:
			//sitting, only permit stand
			if (_action != HEXAPOD_STAND){
				ERRORPRINT("hex action: fail: stand only");
				LogError("hex action: fail: stand only");
				result = ACTION_FAIL;
			}
			else
			{
//				DEBUGPRINT("hex action: %s", arb_action_names[_action]);
				SendActionCommand(MSG_TYPE_STAND);
				result = ACTION_RUNNING;
			}
			break;
		default:
			ERRORPRINT("hex action: fail: bad state");
			LogError("hex action: fail: bad arb state");
			result = ACTION_FAIL;
			break;
		}
		break;
		case ACTION_RUNNING:
			//waiting for a command to complete
//			DEBUGPRINT("hex action: %s running", arb_action_names[_action]);
			//check which, for ending condition
			switch(lastAction)
			{
			case HEXAPOD_STAND:
			{
				//done if ready
				switch(arbotixState)
				{
				case ARBOTIX_TORQUED:
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
			case HEXAPOD_POSE_MODE:
				switch(arbotixState)
				{
				case ARBOTIX_WAITING:
					result = ACTION_RUNNING;
					break;
				case ARBOTIX_POSE_READY:
					result = ACTION_SUCCESS;
					break;
				default:
					result = ACTION_FAIL;
					break;
				}
				break;
				default:
				{
					//otherwise, done when ready
					switch(arbotixState)
					{
					case ARBOTIX_WALKING:
					case ARBOTIX_STOPPING:
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
	DEBUGPRINT("hex: action: %s : %s", arb_action_names[_action], actionResultNames[result]);
	lastAction = _action;
	return (lastResult = result);
}

ActionResult_enum HexapodAssumePose(const char *poseName)
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
		case ARBOTIX_POSE_READY:
		{
			message_t arbMsg;
//			DEBUGPRINT("arb: new pose: %s", poseName);
			//ready for an action
			strncpy(lastPoseName, poseName, MAX_POSENAME);

			//open pose file and send to ArbotixM
			//make path
			char posePath[100];
			snprintf(posePath, 100, "%s/%s", POSE_FOLDER, poseName);
			FILE *poseFile = fopen(posePath, "r");

			if (poseFile)
			{
				char *lineptr = (char*) calloc(100,1);
				size_t n = 100;
				ssize_t count = 0;
				int legCount = 0;

				do
				{
					count = getline(&lineptr, &n, poseFile);

					if (count <= 0) break;

					char lineType[5] = "";

					sscanf(lineptr, "%1s", lineType);

					if (strcmp(lineType, "R") == 0)
					{
						memset(&arbMsg, 0, sizeof(message_t));
						arbMsg.msgType = MSG_TYPE_SET_POSE;
						int legNumber {0};
						float x {0.0};
						float y {0.0};
						float z {0.0};
						if (sscanf(lineptr, "%1s %i %f %f %f ", lineType, &legNumber,
								&x, &y, &z) == 5)
						{
							arbMsg.legNumber = (short int) legNumber;
							arbMsg.x = (short int) x;
							arbMsg.y = (short int) y;
							arbMsg.z = (short int) z;

							DEBUGPRINT("Pose: Leg %i: %f, %f, %f", legNumber, x, y, z);

							SendArbMessage(&arbMsg);
							legCount++;
						}
					}
				}
				while (count > 0);
				free(lineptr);
				fclose(poseFile);

				if (legCount == 6)
				{
					//all legs sent
					//calculate transition time
					memset(&arbMsg, 0, sizeof(message_t));
					arbMsg.msgType = MSG_TYPE_MOVE;
					switch(timingAction)
					{
					case HEXAPOD_POSE_SLOW:
					case HEXAPOD_POSE_MEDIUM:
					case HEXAPOD_POSE_FAST:
					case HEXAPOD_POSE_BEAT:
					case HEXAPOD_POSE_DOWNBEAT:
					case HEXAPOD_POSE_UPBEAT:
					default:
						arbMsg.time = 500;
						break;
					}
					//start move
					SendArbMessage(&arbMsg);
					statusTimeout = time(NULL) + 5;			//give it 5 seconds to answer
					ReportHexapodStatus(ARBOTIX_WAITING);
					result = ACTION_RUNNING;
				}
				else
				{
					ERRORPRINT("arb: Bad pose file: %s", posePath);
					result = ACTION_FAIL;
				}
			}
			else
			{
				ERRORPRINT("arb: Failed to open: %s", posePath);
				result = ACTION_FAIL;
			}
		}
		break;
		default:
			result = ACTION_FAIL;
			break;
		}
		break;
		case ACTION_RUNNING:
			//waiting for a command to complete
//			DEBUGPRINT("hex action: %s running", poseName);
			//check for ending condition
			switch(arbotixState)
			{
			case ARBOTIX_POSING:
			case ARBOTIX_WAITING:
				result = ACTION_RUNNING;
				break;
			case ARBOTIX_POSE_READY:
				result = ACTION_SUCCESS;
				break;
			default:
				result = ACTION_FAIL;
				break;
			}
			break;
	}
	DEBUGPRINT("hex: pose: %s : %s", poseName, actionResultNames[result]);

	return (lastResult = result);
}

//helper
void CheckAX12error(int s, int errorByte, int mask, int condition, const char *errMsg)
{
	if (errorByte & mask)
	{
		ps_set_condition(condition);
		ERRORPRINT("arb: %s : %s", servoNames[s-1], errMsg);
	}
	else
	{
		ps_cancel_condition(condition);
	}
}

//thread to listen for messages from ArbotixM
void *ArbRxThread(void *a) {
	message_t arbMsg;

	unsigned char c;
	int count;
	int csum;
	unsigned int i;

	DEBUGPRINT("arbotix: Rx thread ready");

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

		//		DEBUGPRINT("arbotix: message start");

		//read the message
		count = 8;
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

		if (count < 0)
		{
			DEBUGPRINT("arb: read error %i : %s", count, strerror(errno));
		}

		if (count == 0) {
			//whole payload read
			msgChars =  (unsigned char *)  &arbMsg;

			//read msgType
			count = 0;
			do {
				count = read(arbUartFD, &arbMsg.msgType, 1);
			}  while (count == 0 || count == EAGAIN);

			//read checksum
			count = 0;
			do {
				count = read(arbUartFD, &c, 1);
			}  while (count == 0 || count == EAGAIN);

			if (count == 1)
			{
				//calculate checksum
				csum = arbMsg.msgType;
				for (i = 0; i < 8; i++) {
					csum += msgChars[i];
				}

				if (c == (csum & 0xff)) {
					//valid checksum
					if (arbMsg.msgType >= MSG_TYPE_COUNT)
					{
						ERRORPRINT("arbotix: bad msgType: %i -", arbMsg.msgType);
						continue;
					}

					switch (arbMsg.msgType) {
					case MSG_TYPE_STATUS:    //Status report payload
						if (arbMsg.state < AX_STATE_COUNT)
						{
							DEBUGPRINT("arb: status message: %s", arbotixStateNames[arbMsg.state])
						}
						else
						{
							DEBUGPRINT("arb: bad status message: %u", arbMsg.state);
							continue;
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
							DEBUGPRINT("arb: Leg %i: X=%i Y=%i Z=%i", arbMsg.legNumber, arbMsg.x, arbMsg.y, arbMsg.z );
							break;
						case MSG_TYPE_SERVOS:    //Servo payload
							DEBUGPRINT("arb: Leg %i: Coxa=%i Femur=%i Tibia=%i", arbMsg.legNumber, arbMsg.coxa, arbMsg.femur, arbMsg.tibia );
							break;
						case MSG_TYPE_MSG:       //Text message
							DEBUGPRINT("arb: %4s", arbMsg.text);
							break;
						case MSG_TYPE_ODOMETRY: {
							psMessage_t msg;
							psInitPublish(msg, ODOMETRY);
							msg.odometryPayload.xMovement = arbMsg.xMovement;
							msg.odometryPayload.yMovement = arbMsg.yMovement;
							msg.odometryPayload.zRotation = arbMsg.zRotation;
							NewBrokerMessage(msg);
							DEBUGPRINT("arb odo: x=%i, y=%i, z=%i", arbMsg.xMovement, arbMsg.yMovement, arbMsg.zRotation);
						}
						break;
						case MSG_TYPE_VOLTS:     //Text message
							lastVoltsMessage = arbMsg;
							DEBUGPRINT("arb batt: %0.1fV",
									(float) (arbMsg.volts / 10));
							break;
						case MSG_TYPE_FAIL:
							ERRORPRINT("arb Fail: %s = %i", servoNames[arbMsg.servo], arbMsg.angle);
							ReportHexapodStatus(ARBOTIX_ERROR);
							break;
						case MSG_TYPE_ERROR:
						{
							if (arbMsg.errorCode >= 0 && arbMsg.errorCode < MSG_TYPE_COUNT)
							{
								if (arbMsg.errorCode < MSG_TYPE_UNKNOWN)
								{
									DEBUGPRINT("arbotix: Debug: %s", msgTypes_L[arbMsg.errorCode]);
								}
								else
								{
									DEBUGPRINT("arbotix: Error: %s", msgTypes_L[arbMsg.errorCode]);
									switch(arbMsg.errorCode)
									{
									case LOW_VOLTAGE:
										ReportHexapodStatus(ARBOTIX_LOWVOLTAGE);
										break;
									case SERVO_ERROR:
										CheckAX12error(arbMsg.error1, arbMsg.error2, ERR_VOLTAGE, AX12_VOLTAGE ,"AX12.Voltage");
										CheckAX12error(arbMsg.error1, arbMsg.error2, ERR_ANGLE_LIMIT, AX12_ANGLE_LIMIT ,"AX12.Angle.Limit");
										CheckAX12error(arbMsg.error1, arbMsg.error2, ERR_OVERHEATING, AX12_OVERHEATING ,"AX12.Overheating");
										CheckAX12error(arbMsg.error1, arbMsg.error2, ERR_RANGE, AX12_RANGE ,"AX12.Range");
										CheckAX12error(arbMsg.error1, arbMsg.error2, ERR_CHECKSUM, AX12_CHECKSUM  ,"AX12.Checksum");
										CheckAX12error(arbMsg.error1, arbMsg.error2, ERR_OVERLOAD, AX12_OVERLOAD ,"AX12.Overload");
										CheckAX12error(arbMsg.error1, arbMsg.error2, ERR_INSTRUCTION, AX12_INSTRUCTION ,"AX12.Instruction");
										break;
									default:
										ReportHexapodStatus(ARBOTIX_ERROR);
										break;
									}
								}
							}
							else
							{
								DEBUGPRINT("arbotix: debug: %i?", arbMsg.errorCode);
							}
						}
						break;
						default:
							DEBUGPRINT("arbotix: Rx: %s", msgTypes_L[arbMsg.msgType]);
							break;
					}
				}
				else
				{
					ERRORPRINT("arbotix: sum=%x. Expected %02x, got %02x", csum, (csum & 0xff), c);
				}
			}
		}
	}
	return NULL;
}

//thread to check for ArbotixM command timeouts
void *ArbTimeoutThread(void *a)
{
	DEBUGPRINT("arbotix Timeout thread started");

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
			else if (statusTimeout < time(NULL))
			{
				DEBUGPRINT("arbotix: timeout GetStatus");
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

	arbMsg.steps = abs(6 * degrees / arbotixTurnSpeed);	//cycle time = 2 secs, 12 steps per cycle

	if (arbMsg.steps == 0) return ACTION_SUCCESS;

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
	if (ps_registry_get_int("Pose", "Heading", &heading) != PS_OK)
	{
		lastLuaCallReason = "no heading";
		return ACTION_FAIL;
	}

	return ActionTurn(degrees - heading);
}
//move X cms
ActionResult_enum ActionMove(int mm)
{
	if (abs(mm) < 10) return ACTION_SUCCESS;

	message_t arbMsg;
	memset(&arbMsg, 0, sizeof(message_t));

	arbMsg.msgType = MSG_TYPE_WALK;
	arbMsg.xSpeed = (mm > 0 ? arbotixWalkSpeed : -arbotixWalkSpeed);
	arbMsg.steps = abs(mm / 10);

	if (arbMsg.steps == 0) return ACTION_SUCCESS;

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

	int csum = arbMsg->msgType;
	unsigned int i;
	unsigned char *msgChars = (unsigned char*) arbMsg;

	for (i=0; i<8; i++)
	{
		csum += msgChars[i];
	}

	DEBUGPRINT("arbotix Tx: %02x %02x %02x %02x %02x %02x %02x %02x %02x | %02x",
			msgChars[0], msgChars[1], msgChars[2], msgChars[3],
			msgChars[4], msgChars[5], msgChars[6], msgChars[7], arbMsg->msgType, csum);

	//critical section
	int m = pthread_mutex_lock(&arbMsgMtx);
	if (m != 0)
	{
		ERRORPRINT("arbotix: arbMsgMtx lock %i", m);
	}

	writeByte(MSG_START);

	for (i=0; i<8; i++)
	{
		writeByte(msgChars[i]);
	}
	writeByte(arbMsg->msgType);
	writeByte(csum & 0xff);

	m = pthread_mutex_unlock(&arbMsgMtx);
	if (m != 0)
	{
		ERRORPRINT("arbotix: arbMsgMtx unlock %i", m);
	}
	//end critical section

	DEBUGPRINT("arbotix: Tx: %s", msgTypes_L[arbMsg->msgType]);
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
	statusTimeout = time(NULL) + 2;		//give it 2 seconds to answer
	ReportHexapodStatus(ARBOTIX_WAITING);
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
		LogInfo("status: %s", arbotixStateNames[s]);

		//critical section
		int m = pthread_mutex_lock(&arbStateMtx);
		if (m != 0)
		{
			ERRORPRINT("arbotix: arbStateMtx lock %i", m);
		}

		arbotixState = s;

		DEBUGPRINT("arbotix: State %s", arbotixStateNames[s]);

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
			ERRORPRINT("arbotix: arbStateMtx unlock %i", m);
		}
		//end critical section
	}
}

