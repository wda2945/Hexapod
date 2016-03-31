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
#define DEBUGPRINT(...) fprintf(stdout, __VA_ARGS__);fprintf(gripperDebugFile, __VA_ARGS__);fflush(gripperDebugFile);
#else
#define DEBUGPRINT(...) fprintf(gripperDebugFile, __VA_ARGS__);fflush(gripperDebugFile);
#endif

#define ERRORPRINT(...) fprintf(stdout, __VA_ARGS__);fprintf(gripperDebugFile, __VA_ARGS__);fflush(gripperDebugFile);

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
	else DEBUGPRINT("Gripper PWM pin period = 20mS\n");

	if (mraa_pwm_pulsewidth_us(gripper_pwm, CLOSED_PULSE_LENGTH) != MRAA_SUCCESS)  {
		ERRORPRINT("Can't set pulse-width\n");
		return -1;
	}
	else DEBUGPRINT("Gripper PWM pulse width = %iuS\n", CLOSED_PULSE_LENGTH);

	// enable PWM on the selected pin
	if (mraa_pwm_enable(gripper_pwm, 1) != MRAA_SUCCESS) {
		ERRORPRINT("Can't enable Gripper PWM\n");
		return -1;
	}
	else DEBUGPRINT("Gripper PWM enabled\n");

	////////////////////////////////// DEBUG

while(1)
{
	sleep(1);

	if (mraa_pwm_pulsewidth_us(gripper_pwm, OPEN_PULSE_LENGTH) != MRAA_SUCCESS)  {
		ERRORPRINT("Can't set Gripper PWM pulse-width\n");
		return -1;
	}
	else DEBUGPRINT("Gripper PWM pulse width = %iuS\n", OPEN_PULSE_LENGTH);

	sleep(1);

	//period = 20 mS (50Hz), duty = 1.5 mS
	if (mraa_pwm_pulsewidth_us(gripper_pwm, CLOSED_PULSE_LENGTH) != MRAA_SUCCESS)  {
		ERRORPRINT("Can't set Gripper PWM pulse-width\n");
		return -1;
	}
	else DEBUGPRINT("Gripper PWM pulse width = %iuS\n", CLOSED_PULSE_LENGTH);

}
	//////////////////////////// END DEBUG

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

	int current, increment;

	current = CLOSED_PULSE_LENGTH;

	DEBUGPRINT("Gripper thread\n");

	while (1)
	{
		if (gripperState != gripperGoal)
		{
			switch (gripperSpeed)
			{
			case GRIPPER_SLOW:
				increment = (OPEN_PULSE_LENGTH - CLOSED_PULSE_LENGTH) / 20;
				break;
			case GRIPPER_FAST:
				increment = (OPEN_PULSE_LENGTH - CLOSED_PULSE_LENGTH) / 10;
				break;
			}


			switch (gripperGoal)
			{
			case GRIPPER_CLOSED:
				gripperState = GRIPPER_MOVING;

				for (; current > CLOSED_PULSE_LENGTH; current -= increment)
				{
					if (mraa_pwm_pulsewidth_us(gripper_pwm, current) != MRAA_SUCCESS)  {
						ERRORPRINT("Can't set Gripper PWM pulse-width: %i\n", current);
						break;
					}
					else DEBUGPRINT("Gripper PWM pulse width = %iuS\n", current);
					usleep(100000);
				}
				gripperState = GRIPPER_CLOSED;
				break;
			case GRIPPER_OPEN:
				gripperState = GRIPPER_MOVING;

				for (; current > OPEN_PULSE_LENGTH; current -= increment)
				{
					if (mraa_pwm_pulsewidth_us(gripper_pwm, current) != MRAA_SUCCESS)  {
						ERRORPRINT("Can't set Gripper PWM pulse-width: %i\n", current);
						break;
					}
					else DEBUGPRINT("Gripper PWM pulse width = %iuS\n", current);
					usleep(100000);
				}
				gripperState = GRIPPER_OPEN;
				break;
			default:
				break;
			}

		}
		usleep(100000);
	}
	return 0;
}

GripperSpeed_enum gripperSpeed;
