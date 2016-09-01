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

#include "hexapod.h"

#include "i2c_task/i2c_task.hpp"
#include "i2c_task/i2c_common.h"
#include "i2c_task/PCF8591.h"

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
	ps_registry_add_new("Power", "Volts", PS_REGISTRY_REAL_TYPE, PS_REGISTRY_SRC_WRITE);
	ps_registry_add_new("Power", "Status", PS_REGISTRY_TEXT_TYPE, PS_REGISTRY_SRC_WRITE);

	return 0;
}

//read selected channel
int PCF8591Read(int channel)
{
	//set ADC I2C address
	if (mraa_i2c_address(i2c_context, ADC_ADDRESS) != MRAA_SUCCESS)
	{
		ERRORPRINT("ADC: mraa_i2c_address error");
		ps_set_condition(I2C_BUS_ERROR);
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
				ERRORPRINT("ADC: mraa_i2c_write() - %s", strerror(errno));
				ps_set_condition(ANALOG_ERROR);
				return -1;
			}
			else
			{
				lastChannel = channel;
//				DEBUGPRINT("ADC: channel: %i selected", channel);
			}
		}

		//read adc data
		//read results
		if (mraa_i2c_read(i2c_context, readData, sizeof(readData)) == 0)
		{
			ERRORPRINT("ADC: mraa_i2c_read() - %s", strerror(errno));
			ps_set_condition(ANALOG_ERROR);
			ps_cancel_condition(ADC_ONLINE);

			return -1;
		}

		int adcData = readData[1];

		DEBUGPRINT("ADC %i = 0x%02x", channel, adcData);
		ps_set_condition(ADC_ONLINE);

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

			ps_registry_set_real("Power", "Volts", volts);

			DEBUGPRINT("Battery = %0.1fv", volts);

			if (volts < BATTERY_CRITICAL_VOLTS)
			{
				ps_set_condition(BATTERY_CRITICAL);
				ps_registry_set_text("Power", "Status", "CRITICAL");
			}
			else if (volts < BATTERY_LOW_VOLTS)
			{
				ps_cancel_condition(BATTERY_CRITICAL);
				ps_set_condition(BATTERY_LOW);
				ps_registry_set_text("Power", "Status", "LOW");
			}
			else
			{
				ps_cancel_condition(BATTERY_CRITICAL);
				ps_cancel_condition(BATTERY_LOW);
				ps_registry_set_text("Power", "Status", "normal");
			}

		}
		break;
		default:
			break;
		}
	}
	return 0;
}
