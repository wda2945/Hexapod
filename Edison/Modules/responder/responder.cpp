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
#include <pthread.h>

#include "responder.hpp"
#include "main_debug.h"

void ResponderProcessMessage(const void *, int);

void register_option(const char *name, int value);
void register_setting(const char *name, float minV, float maxV, float value);
void register_condition(int c, const char *name);

int ResponderInit()
{
	//add settings to registry
#define optionmacro(name, val, minV, maxV, def) register_option(name, val);
#include <options.h>
#undef optionmacro
	sleep(1);
#define settingmacro(name, val, minV, maxV, def) register_setting(name, minV, maxV, val);
#include <settings.h>
#undef settingmacro

	sleep(1);
	//add conditions to registry
#define CONDITION_MACRO(e, n) register_condition(e, n);
#include <NotificationConditionsList.h>
#undef CONDITION_MACRO

	ps_subscribe(ANNOUNCEMENTS_TOPIC, ResponderProcessMessage);

	return 0;
}

void ResponderProcessMessage(const void *_msg, int len)
{
	psMessage_t *msg = (psMessage_t *) _msg;
	switch (msg->messageType)
	{

	case PING_MSG:
	{
		DEBUGPRINT("Ping msg\n");
		psMessage_t msg2;
		psInitPublish(msg2, PING_RESPONSE);
		RouteMessage(msg2);

		send_registry_sync();
	}
	break;

	default:
		//ignore anything else
		break;
	}

}

void register_option(const char *name, int value) {
	ps_registry_struct_t registry_value;

	registry_value.datatype = PS_REGISTRY_BOOL_TYPE;
	registry_value.bool_value = value;
	registry_value.flags = (ps_registry_flags_t) PS_REGISTRY_SRC_WRITE | PS_REGISTRY_ANY_WRITE;

	if (ps_registry_set_new("Options", name, registry_value) == PS_OK)
	{
		DEBUGPRINT("Registered option %s", name);
	}
	else
	{
		ERRORPRINT("Registering option %s failed", name);
	}
}

void register_setting(const char *name, float minV, float maxV, float value) {
	ps_registry_struct_t registry_value;

	registry_value.datatype = PS_REGISTRY_REAL_TYPE;
	registry_value.real_min = minV;
	registry_value.real_max = maxV;
	registry_value.real_value = value;
	registry_value.flags = (ps_registry_flags_t) PS_REGISTRY_SRC_WRITE | PS_REGISTRY_ANY_WRITE;

	if (ps_registry_set_new("Settings", name, registry_value) == PS_OK)
	{
		DEBUGPRINT("Registered setting %s", name);
	}
	else
	{
		ERRORPRINT("Registering setting %s failed", name);
	}
}


void register_condition(int c, const char *name) {

	if (c == 0) return;	//null

	ps_registry_struct_t registry_value;

	registry_value.datatype = PS_REGISTRY_INT_TYPE;
	registry_value.int_value = c;
	registry_value.flags = (ps_registry_flags_t) 0;

	if (ps_registry_set_new("Conditions", name, registry_value) == PS_OK)
	{
		DEBUGPRINT("Registered condition %i = %s", c, name);
	}
	else
	{
		ERRORPRINT("Registering condition %i = %s failed", c, name);
	}
}
