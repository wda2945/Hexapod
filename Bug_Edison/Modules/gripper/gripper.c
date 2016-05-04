/*
 * gripper.cpp
 *
 *      Author: martin
 */
 
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stropts.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <errno.h>

#include "mraa.h"

#include "softwareProfile.h"
#include "pubsubdata.h"
#include "pubsub/pubsub.h"
#include "syslog/syslog.h"

#include "gripper/gripper.h"

FILE *gripperDebugFile;

#ifdef GRIPPER_DEBUG
#define DEBUGPRINT(...) tprintf( __VA_ARGS__);tfprintf(gripperDebugFile, __VA_ARGS__);
#else
#define DEBUGPRINT(...) tfprintf(gripperDebugFile, __VA_ARGS__);
#endif

#define ERRORPRINT(...) tprintf( __VA_ARGS__);tfprintf(gripperDebugFile, __VA_ARGS__);

//queue for messages
BrokerQueue_t gripperQueue = BROKER_Q_INITIALIZER;

//Gripper thread
void *GripperThread(void *arg);

mraa_pwm_context gripper_pwm;

GripperState_enum gripperState = GRIPPER_CLOSED;
GripperState_enum gripperGoal = GRIPPER_CLOSED;

int GripperInit()
{
	gripperDebugFile = fopen_logfile("gripper");
	DEBUGPRINT("Gripper Logfile opened\n");

	// init pwm
	gripper_pwm = mraa_pwm_init(GRIPPER_SERVO_PIN);
	if (gripper_pwm == NULL) {
		ERRORPRINT("Can't create Gripper PWM object\n");
		return -1;
	}

	// disable PWM on the selected pin
	if (mraa_pwm_enable(gripper_pwm, 0) != MRAA_SUCCESS) {
		ERRORPRINT("Can't disable Gripper PWM\n");
		return -1;
	}

	//period = 50Hz
	if (mraa_pwm_period_ms(gripper_pwm, 20) != MRAA_SUCCESS)  {
		ERRORPRINT("Can't set Gripper PWM period\n");
		return -1;
	}
	else {
		DEBUGPRINT("Gripper PWM pin period = 20mS\n");
	}

	if (mraa_pwm_pulsewidth_us(gripper_pwm, CLOSED_PULSE_LENGTH) != MRAA_SUCCESS)  {
		ERRORPRINT("Can't set pulse-width\n");
		return -1;
	}
	else {
		DEBUGPRINT("Gripper PWM pulse width = %iuS\n", CLOSED_PULSE_LENGTH);
	}

	// enable PWM on the selected pin
	if (mraa_pwm_enable(gripper_pwm, 1) != MRAA_SUCCESS) {
		ERRORPRINT("Can't enable Gripper PWM\n");
		return -1;
	}
	else {
		DEBUGPRINT("Gripper PWM enabled\n");
	}

	pthread_t thread;
	//create agent Rx thread
	int s = pthread_create(&thread, NULL, GripperThread, NULL);
	if (s != 0) {
		ERRORPRINT("pthread_create() failed. %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

ActionResult_enum MoveGripper(GripperState_enum action)
{
	if (action == gripperState)
	{
		return ACTION_SUCCESS;
	}
	else
	{
		gripperGoal = action;
		return ACTION_RUNNING;
	}
}

void *GripperThread(void *arg)
{

	int currentPW, stepPW;

	currentPW = CLOSED_PULSE_LENGTH;

	DEBUGPRINT("Gripper thread\n");

	while (1)
	{
		if (gripperCycle)
		{
			if (gripperState == GRIPPER_CLOSED)
			{
				gripperGoal = GRIPPER_OPEN;
			}
			else if (gripperState == GRIPPER_OPEN)
			{
				gripperGoal = GRIPPER_CLOSED;
			}
		}

		if (gripperState != gripperGoal)
		{
			if (gripperState != GRIPPER_MOVING)
			{
				switch (gripperSpeed)
				{
				case GRIPPER_SLOW:
					stepPW = (OPEN_PULSE_LENGTH - CLOSED_PULSE_LENGTH) / 100;
					break;
				case GRIPPER_FAST:
					stepPW = (OPEN_PULSE_LENGTH - CLOSED_PULSE_LENGTH) / 50;
					break;
				}
				gripperState = GRIPPER_MOVING;
			}

			switch (gripperGoal)
			{
			case GRIPPER_CLOSED:
				if (currentPW > CLOSED_PULSE_LENGTH) currentPW -= stepPW;
				else gripperState = GRIPPER_CLOSED;
				break;
			case GRIPPER_OPEN:
				if (currentPW < OPEN_PULSE_LENGTH) currentPW += stepPW;
				else gripperState = GRIPPER_OPEN;
				break;
			default:
				break;
			}

			if (mraa_pwm_pulsewidth_us(gripper_pwm, currentPW) != MRAA_SUCCESS)  {
				ERRORPRINT("Can't set Gripper PWM pulse-width: %i\n", currentPW);
				break;
			}
			else {
				DEBUGPRINT("Gripper PWM pulse width = %iuS\n", currentPW);
			}
		}
		usleep(100000);
	}
	return 0;
}

GripperSpeed_enum gripperSpeed;
