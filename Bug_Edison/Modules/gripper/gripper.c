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

	if (mraa_pwm_pulsewidth_us(gripper_pwm, 1500) != MRAA_SUCCESS)  {
		ERRORPRINT("Can't set pulse-width\n");
		return -1;
	}
	else DEBUGPRINT("Gripper PWM pulse width = 1500uS\n");

	// enable PWM on the selected pin
	if (mraa_pwm_enable(gripper_pwm, 1) != MRAA_SUCCESS) {
		ERRORPRINT("Can't enable Gripper PWM\n");
		return -1;
	}
	else DEBUGPRINT("Gripper PWM enabled\n");

//	sleep(1);
//
//	if (mraa_pwm_pulsewidth_us(gripper_pwm, 2000) != MRAA_SUCCESS)  {
//		ERRORPRINT("Can't set Gripper PWM pulse-width\n");
//		return -1;
//	}
//	else DEBUGPRINT("Gripper PWM pulse width = 2000uS\n");
//
//	sleep(1);

//	//period = 20 mS (50Hz), duty = 1.5 mS
//	if (mraa_pwm_pulsewidth_us(gripper_pwm, 1500) != MRAA_SUCCESS)  {
//		ERRORPRINT("Can't set Gripper PWM pulse-width\n");
//		return -1;
//	}
//	else DEBUGPRINT("Gripper PWM pulse width = 1500uS\n");

//	pthread_t thread;
//
//	//create agent Rx thread
//	int s = pthread_create(&thread, NULL, GripperThread, NULL);
//	if (s != 0) {
//		ERRORPRINT("pthread_create() failed. %s\n", strerror(errno));
//		return -1;
//	}

	return 0;
}

void GripperProcessMessage(psMessage_t *msg)
{
	CopyMessageToQ(&gripperQueue, msg);
}

#define min(x,y) ((x) > (y) ? (y) : (x))
#define max(x,y) ((x) > (y) ? (x) : (y))

#define SERVO_MIN 500
#define SERVO_MAX 2500
#define SERVO_RANGE (SERVO_MAX - SERVO_MIN)
#define SERVO_MID 1500
#define SERVO_PERIOD 20
#define SERVO_LOOP	 200

void *GripperThread(void *arg)
{
	struct timespec requested_time;
	struct timespec remaining;

	requested_time.tv_nsec = SERVO_PERIOD * 1000000;
	requested_time.tv_sec = 0;

	int current, goal, increment;
	int i;
	current = goal = SERVO_MID;

	DEBUGPRINT("Gripper thread\n");

	while (1)
	{
#define SERVO_STEPS 50
#define SERVO_INC	(SERVO_RANGE / (2 * SERVO_STEPS))
		for (i=0; i<(SERVO_STEPS * 4); i++)
		{
			int openstage = i / SERVO_STEPS;
			int step = i % SERVO_STEPS;

			switch(openstage)
			{
			case 0:
				goal = SERVO_MID + SERVO_INC * step;
				break;
			case 1:
				goal = SERVO_MAX - SERVO_INC * step;
				break;
			case 2:
				goal = SERVO_MID - SERVO_INC * step;
				break;
			default:
				goal = SERVO_MIN + SERVO_INC * step;
				break;
			}
			if (mraa_pwm_pulsewidth_us(gripper_pwm, goal) != MRAA_SUCCESS) {
				ERRORPRINT("Can't set duty\n");
				sleep(1);
			}
			nanosleep (&requested_time, &remaining);
		}
		sleep(1);
	}

	//loop
	while (1)
	{
		psMessage_t *rxMessage = GetNextMessage(&gripperQueue);

		switch(rxMessage->header.messageType)
		{
		case GRIP:
		{
			float opening = rxMessage->twoFloatPayload.opening;
			opening = max(min(opening, 1.0), 0.0);
			float speed = rxMessage->twoFloatPayload.speed;
			speed = max(min(speed, 10.0), 0.5);		//seconds from open to closed

			goal = (SERVO_MIN + (SERVO_MAX - SERVO_MIN) * opening);

			DEBUGPRINT("Gripper to: %0.1f, speed %0.1f", opening, speed);

			increment = (SERVO_LOOP / 1000) * (SERVO_MAX - SERVO_MIN) / speed;	//mS change per loop

			while (goal != current)
			{
				if (abs(goal - current) <= increment)
				{
					current = goal;
				}
				else
				{
					if (goal > current)
					{
						current += increment;
					}
					else
					{
						current -= increment;
					}
				}

				if (mraa_pwm_pulsewidth_us(gripper_pwm, current) != MRAA_SUCCESS) {
					ERRORPRINT("Can't set duty\n");
					goal = current;
				}

				nanosleep (&requested_time, &remaining);
			}
		}
			break;
		default:
			break;
		}

	}
	return 0;
}
