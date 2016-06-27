/*
 * i2c_task.cpp
 *
 *      Author: martin
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "mraa.h"

#include "hexapod.h"

#include "i2c_task.hpp"
#include "i2c_common.h"
#include "LSM303.h"
#include "PCF8591.h"

FILE *i2cDebugFile;

void *I2CThread(void *arg);

//file context for I2C bus
mraa_i2c_context i2c_context;


void LSM303StartCalibrate()
{
	while (!(i2c_context = mraa_i2c_init(I2C_BUS)))
	{
		sleep(1);
	}

	LSM303Calibrate();
}

//create I2C Thread
int I2CInit()
{
	pthread_t th;
	i2cDebugFile = fopen_logfile("i2c");
	DEBUGPRINT("I2C Logfile opened\n");

	i2c_context = mraa_i2c_init(I2C_BUS);
	if (!i2c_context)
	{
		ERRORPRINT("I2C: open error: %s\n", strerror(errno));
		SetCondition(I2C_BUS_ERROR);
		return -1;
	}

	int s = pthread_create(&th, NULL, I2CThread, NULL);
	if (s != 0)
	{
		ERRORPRINT("I2CThread create error - %i\n", s);
		return -1;
	}

	return 0;
}

void I2CProcessMessage(psMessage_t *msg)
{
	//no messages used at present
}

#define BASE_INTERVAL	999			//mS	loop delay (minimum interval needed)
#define BATT_INTERVAL	10000		//mS
#define IMU_INTERVAL	BASE_INTERVAL		//mS
#define FAIL_INTERVAL	10000		//mS	wait for retry

void *I2CThread(void *arg)
{
	struct timespec requested;
	int imuCount = 0;
	int battCount = 1;

	if (mraa_i2c_frequency(i2c_context, MRAA_I2C_STD) != MRAA_SUCCESS)
	{
		ERRORPRINT("mraa_i2c_frequency() fail\n");
	}

	LSM303Init();		//imu
	PCF8591Init();		//adc

	while(1)
	{
		//read ADC audio
		if (PCF8591Read(ADC_AUDIO_CHAN) < 0)
		{
			ERRORPRINT("PCF8591Read() fail\n");
		}

		//read IMU periodically
		if (imuCount-- <= 0)
		{
			if (LSM303Read() < 0)
			{
				ERRORPRINT("LSM303Read() fail\n");
				imuCount = FAIL_INTERVAL / BASE_INTERVAL;
			}
			else
			{
				imuCount = IMU_INTERVAL / BASE_INTERVAL;
			}
		}

		//measure battery periodically
		if (battCount-- <= 0)
		{
			if (PCF8591Read(ADC_BATTERY_CHAN) < 0)
			{
				ERRORPRINT("PCF8591Read() fail\n");
				battCount = FAIL_INTERVAL / BASE_INTERVAL;
			}
			else
			{
				battCount = BATT_INTERVAL / BASE_INTERVAL;
			}
		}

		//delay
		requested.tv_sec = 0;
		requested.tv_nsec = BASE_INTERVAL * 1000000;
		while (nanosleep(&requested, &requested) != 0);
	}

	return NULL;
}

