//
//  behavior.c
//
//  Created by Martin Lane-Smith on 6/14/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//
// Root threads of the scripted behavior subsystem

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#include "hexapod.h"

#include "lua.hpp"

#include "behavior/behavior.hpp"
#include "behavior/behaviorDebug.h"
#include "brokerq/brokerq.h"

FILE *behDebugFile;

pthread_mutex_t	luaMtx = PTHREAD_MUTEX_INITIALIZER;
BrokerQueue_t behaviorQueue = BROKER_Q_INITIALIZER;

void *ScriptThread(void *arg);
void *BehaviorMessageThread(void *arg);
void BehaviorProcessMessage(const void *_msg, int len);

int BehaviorInit()
{
	pthread_t thread;
	int s;

	behDebugFile = fopen_logfile("behavior");

	int result = InitScriptingSystem();
	if (result == 0)
	{
		LogInfo("Script system initialized");
	}
	else
	{
		ERRORPRINT("Script system init fail: %i", result);
		return -1;
	}

	RegisterAvailableScripts();

	s = pthread_create(&thread, NULL, ScriptThread, NULL);
	if (s != 0)
	{
		ERRORPRINT("ScriptThread create - %i", s);
		return -1;
	}

	s = pthread_create(&thread, NULL, BehaviorMessageThread, NULL);
	if (s != 0)
	{
		ERRORPRINT("BehaviorMessageThread create - %i", s);
		return -1;
	}

	ps_subscribe(SYS_ACTION_TOPIC, BehaviorProcessMessage);

	return 0;
}

void BehaviorProcessMessage(const void *_msg, int len)
{
	psMessage_t *msg = (psMessage_t *) _msg;
	CopyMessageToQ(&behaviorQueue, msg);
}

//thread to receive messages and update lua blackboard
void *BehaviorMessageThread(void *arg)
{
	LogInfo("Behavior message thread started");

	while (1)
	{
		psMessage_t *msg = GetNextMessage(&behaviorQueue);

		switch (msg->messageType)
		{
			default:
			{
				//critical section
				int s = pthread_mutex_lock(&luaMtx);
				if (s != 0)
				{
					ERRORPRINT("BT: lua mutex lock %i", s);
				}

				ScriptProcessMessage(msg);	//update lua globals as necessary

				s = pthread_mutex_unlock(&luaMtx);
				if (s != 0)
				{
					ERRORPRINT("BT: lua mutex unlock %i", s);
				}
				//end critical section
			}
			break;
		}
		DoneWithMessage(msg);
	}
	return 0;
}

//thread to run scripts periodically
void *ScriptThread(void *arg)
{

	LogInfo("Script update thread started");

	while (1)
	{
		//critical section
		int s = pthread_mutex_lock(&luaMtx);
		if (s != 0)
		{
			ERRORPRINT("BT: lua mutex lock %i", s);
		}

		InvokeUpdate();

		s = pthread_mutex_unlock(&luaMtx);
		if (s != 0)
		{
			ERRORPRINT("BT: lua mutex unlock %i", s);
		}
		//end critical section

		//delay
		usleep(behLoopDelay * 1000);
	}
}

//report available scripts
void RegisterAvailableScripts()
{
	//critical section
	int s = pthread_mutex_lock(&luaMtx);
	if (s != 0)
	{
		ERRORPRINT("BT: lua mutex lock %i", s);
	}

	AvailableScripts();

	s = pthread_mutex_unlock(&luaMtx);
	if (s != 0)
	{
		ERRORPRINT("BT: lua mutex unlock %i", s);
	}
	//end critical section
}

//BT call-back result codes
int success(lua_State *L)
{
	lua_pushstring(L, "success");
	return 1;
}
int running(lua_State *L)
{
	lua_pushstring(L, "running");
	return 1;
}
int fail(lua_State *L)
{
	lua_pushstring(L, "fail");
	lastLuaCallFail = lastLuaCall;
	return 1;
}

//enum to BT response
int actionReply(lua_State *L, ActionResult_enum result)
{
	switch (result)
	{
	case ACTION_SUCCESS:
		DEBUGPRINT(".. success")
		return success(L);
		break;
	case ACTION_RUNNING:
		DEBUGPRINT(".. running")
		return running(L);
		break;
	default:
		DEBUGPRINT(".. fail")
		return fail(L);
		break;
	}
}

int ChangeOption(lua_State *L, const char *name, int value)
{
	ps_registry_set_bool("Options", name, value);
	return success(L);
}

int ChangeSetting(lua_State *L, const char *name, float value)
{
	ps_registry_set_real("Settings", name, value);
	return success(L);
}
