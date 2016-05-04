/*
 * PCF8591.c
 *
 *  Created on: Aug 10, 2015
 *      Author: martin
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "mraa.h"

#include "pubsub/pubsub.h"
#include "pubsubdata.h"
#include "syslog/syslog.h"

#include "i2c_task/i2c_task.h"
#include "i2c_task/i2c_common.h"
#include "i2c_task/PCF8591.h"
#include "softwareProfile.h"

//PCF8591 Control byte
#define CHANNEL_NUMBER_MASK 		0x3
#define AUTO_INCREMENT		 		0x4
#define ANALOG_INPUT_SINGLE_ENDED 	0
#define ANALOG_OUTPUT_ENABLE		0x40

uint8_t writeData[2] = {ANALOG_INPUT_SINGLE_ENDED | ANALOG_OUTPUT_ENABLE, 0};
uint8_t readData[2];

int lastChannel = -1;

//initialize ADC
int PCF8591Init()
{
	//nothing
	return 0;
}

//read selected channel
int PCF8591Read(int channel)
{
	//set ADC I2C address
	if (mraa_i2c_address(i2c_context, ADC_ADDRESS) != MRAA_SUCCESS)
	{
		ERRORPRINT("ADC: mraa_i2c_address error\n");
		SetCondition(I2C_BUS_ERROR);
		return -1;
	}
	else
	{
		if (channel != lastChannel)
		{
			//Note: adc audio channel will be read repeatedly
			writeData[0] = (channel & CHANNEL_NUMBER_MASK) | ANALOG_INPUT_SINGLE_ENDED | ANALOG_OUTPUT_ENABLE;

			//select new channel
			if (mraa_i2c_write(i2c_context, writeData, sizeof(writeData)) != MRAA_SUCCESS)
			{
				ERRORPRINT("ADC: mraa_i2c_write() - %s\n", strerror(errno));
				SetCondition(ANALOG_ERROR);
				return -1;
			}
			else
			{
				lastChannel = channel;
				DEBUGPRINT("ADC: channel: %i selected\n", channel);
			}
		}

		//read adc data
		//read results
		if (mraa_i2c_read(i2c_context, readData, sizeof(readData)) == 0)
		{
			ERRORPRINT("ADC: mraa_i2c_read() - %s\n", strerror(errno));
			SetCondition(ANALOG_ERROR);
			return -1;
		}

		int adcData = readData[1];

		DEBUGPRINT("ADC %i = 0x%02x\n", channel, adcData);

		switch (channel)
		{
		case ADC_AUDIO_CHAN:
			//process audio data
			//tbd
			break;
		case ADC_BATTERY_CHAN:
			//send battery message
		{
			float volts = (float) adcData * BATTERY_VOLTS_FACTOR;

			psMessage_t msg;
			psInitPublish(msg, BATTERY);
			msg.batteryPayload.volts = (uint16_t) (volts * 10.0);

			DEBUGPRINT("Battery = %0.1fv\n", volts);

			if (volts < BATTERY_CRITICAL_VOLTS)
			{
				msg.batteryPayload.status = BATTERY_CRITICAL_STATUS;
			}
			else if (volts < BATTERY_LOW_VOLTS)
			{
				msg.batteryPayload.status = BATTERY_LOW_STATUS;
			}
			else
			{
				msg.batteryPayload.status = BATTERY_NOMINAL_STATUS;
			}
			NewBrokerMessage(&msg);
		}
		break;
		default:
			break;
		}
	}
	return 0;
}
