/*
 * responder.c
 *
 * Responds to ping messages from the APP
 *
 *  Created on: Jul 27, 2014
 *      Author: martin
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "helpers.h"
#include "pubsubdata.h"
#include "pubsub/pubsub.h"
#include "behavior/behavior.h"
#include "syslog/syslog.h"

#include "brokerQ.h"
#include "broker_debug.h"

BrokerQueue_t responderQueue = BROKER_Q_INITIALIZER;

void *ResponderMessageThread(void *arg);

void sendOptionConfig(char *name, int var, int minV, int maxV);
void sendSettingConfig(char *name, float var, float minV, float maxV);

int configCount;
int startFlag = 1;

int ResponderInit()
{
	pthread_t thread;
	int s = pthread_create(&thread, NULL, ResponderMessageThread, NULL);
	if (s != 0)
	{
		ERRORPRINT("ResponderMessageThread create - %i\n", s);
		return -1;
	}
	return 0;
}

void ResponderProcessMessage(psMessage_t *msg)
{
	CopyMessageToQ(&responderQueue, msg);
}

//thread to receive messages and respond
void *ResponderMessageThread(void *arg)
{
	DEBUGPRINT("Responder message thread started\n");

	while (1)
	{
		psMessage_t *msg = GetNextMessage(&responderQueue);

		//check message for response requirement
		switch (msg->header.messageType)
		{
		case CONFIG:
			{
				DEBUGPRINT("Send Config msg\n");
				configCount = 0;
#define optionmacro(name, var, minV, maxV, def) sendOptionConfig(name, var, minV, maxV);
#include <options.h>
#undef optionmacro
				sleep(1);
#define settingmacro(name, var, minV, maxV, def) sendSettingConfig(name, var, minV, maxV);
#include <settings.h>
#undef settingmacro
				sleep(1);
				configCount += ReportAvailableScripts();

				{
					psMessage_t msg;
					psInitPublish(msg, CONFIG_DONE);
					msg.configPayload.count = configCount;
					RouteMessage(&msg);
				}
				DEBUGPRINT("Config Done\n");
			}
			break;

		case PING_MSG:
		{
			DEBUGPRINT("Ping msg\n");
			psMessage_t msg2;
			psInitPublish(msg2, PING_RESPONSE);
			strcpy(msg2.responsePayload.subsystem, "OVM");
			RouteMessage(&msg2);
		}
		break;
		case NEW_SETTING:
			DEBUGPRINT("Setting: %s = %f\n", msg->settingPayload.name, msg->settingPayload.value);
#define settingmacro(n, var, minV, maxV, def) if (strncmp(n,msg->settingPayload.name,PS_NAME_LENGTH) == 0)\
		var = msg->settingPayload.value;\
		sendSettingConfig(n, var, minV, maxV);
#include "settings.h"
#undef settingmacro
			break;

		case SET_OPTION:
			DEBUGPRINT("Option: %s = %i\n", msg->optionPayload.name, msg->optionPayload.value);
#define optionmacro(n, var, minV, maxV, def) if (strncmp(n,msg->optionPayload.name,PS_NAME_LENGTH) == 0)\
		var = msg->optionPayload.value;\
		sendOptionConfig(n, var, minV, maxV);
#include "options.h"
#undef optionmacro
			break;
		default:
			//ignore anything else
			break;
		}

		DoneWithMessage(msg);
	}
	return 0;
}

void sendOptionConfig(char *name, int var, int minV, int maxV) {

	psMessage_t msg;
	psInitPublish(msg, OPTION);
	strncpy(msg.optionPayload.name, name, PS_NAME_LENGTH);
	msg.optionPayload.value = var;
	msg.optionPayload.min = minV;
	msg.optionPayload.max = maxV;
	RouteMessage(&msg);
	configCount++;

	//delay
	usleep(100000);
}

void sendSettingConfig(char *name, float var, float minV, float maxV) {

	psMessage_t msg;
	psInitPublish(msg, SETTING);
	strncpy(msg.settingPayload.name, name, PS_NAME_LENGTH);
	msg.settingPayload.value = var;
	msg.settingPayload.min = minV;
	msg.settingPayload.max = maxV;
	RouteMessage(&msg);
	configCount++;

	//delay
	usleep(100000);
}
