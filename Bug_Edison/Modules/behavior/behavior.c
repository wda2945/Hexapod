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

#include <time.h>
#include <string.h>
#include <pthread.h>

#include "lua.h"

#include "PubSubData.h"
#include "pubsub/pubsub.h"

#include "behavior/behavior.h"
#include "behavior/behaviorDebug.h"
#include "syslog/syslog.h"

FILE *behDebugFile;

pthread_mutex_t	luaMtx = PTHREAD_MUTEX_INITIALIZER;

BrokerQueue_t behaviorQueue = BROKER_Q_INITIALIZER;

bool MCPonline 			= false;
bool MCPconfigRequested = false;
bool MCPconfigured		= false;
time_t lastMCPresponseTime  = 0;
time_t MCPconfigRequestedTime = 0;

bool MOTonline 			= false;
bool MOTconfigRequested = false;
bool MOTconfigured		= false;
time_t lastMOTresponseTime  = 0;
time_t MOTconfigRequestedTime = 0;

bool APPonline 	= false;
time_t lastAPPresponseTime = 0;

void *ScriptThread(void *arg);
void *BehaviorMessageThread(void *arg);

int BehaviorInit()
{
	pthread_t thread;
	int s;

	behDebugFile = fopen_logfile("behavior");
	DEBUGPRINT("Behavior Logfile opened\n");

	int result = InitScriptingSystem();
	if (result == 0)
	{
		LogInfo("Script system initialized\n");
	}
	else
	{
		ERRORPRINT("Script system init fail: %i\n", result);
		return -1;
	}

	s = pthread_create(&thread, NULL, ScriptThread, NULL);
	if (s != 0)
	{
		ERRORPRINT("ScriptThread create - %i\n", s);
		return -1;
	}

	s = pthread_create(&thread, NULL, BehaviorMessageThread, NULL);
	if (s != 0)
	{
		ERRORPRINT("BehaviorMessageThread create - %i\n", s);
		return -1;
	}

	return 0;
}

void BehaviorProcessMessage(psMessage_t *msg)
{
	CopyMessageToQ(&behaviorQueue, msg);
}

//thread to receive messages and update lua blackboard
void *BehaviorMessageThread(void *arg)
{
	LogInfo("Behavior message thread started\n");

	{
		psMessage_t activityMsg;
		psInitPublish(activityMsg, ACTIVITY);
		strncpy(activityMsg.behaviorStatusPayload.behavior, "Idle", PS_SHORT_NAME_LENGTH);
		activityMsg.behaviorStatusPayload.status = BEHAVIOR_ACTIVE;
		activityMsg.behaviorStatusPayload.lastLuaCallFail[0] = '\0';
		activityMsg.behaviorStatusPayload.lastFailReason[0] = '\0';
		RouteMessage(&activityMsg);
	}

	while (1)
	{
		psMessage_t *msg = GetNextMessage(&behaviorQueue);

		//		DEBUGPRINT("BT msg: %s\n", psLongMsgNames[msg->header.messageType]);

		switch(msg->header.source)
		{
		case ROBO_APP:
			if (!APPonline)
			{
				APPonline = true;
				LogInfo("APP online\n");
			}
			lastAPPresponseTime = time(NULL);
			break;
		default:
			break;
		}

		switch (msg->header.messageType)
		{
			default:
			{
				//critical section
				int s = pthread_mutex_lock(&luaMtx);
				if (s != 0)
				{
					ERRORPRINT("BT: lua mutex lock %i\n", s);
				}

				ScriptProcessMessage(msg);	//update lua globals as necessary

				s = pthread_mutex_unlock(&luaMtx);
				if (s != 0)
				{
					ERRORPRINT("BT: lua mutex unlock %i\n", s);
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
	struct timespec requested_time = {0,0};
	struct timespec remaining;

	LogInfo("Script thread started\n");

	while (1)
	{
		//critical section
		int s = pthread_mutex_lock(&luaMtx);
		if (s != 0)
		{
			ERRORPRINT("BT: lua mutex lock %i\n", s);
		}

		InvokeUpdate();

		s = pthread_mutex_unlock(&luaMtx);
		if (s != 0)
		{
			ERRORPRINT("BT: lua mutex unlock %i\n", s);
		}
		//end critical section

		//check network status
		if (APPonline && (lastAPPresponseTime + (int)networkTimeout < time(NULL)))
		{
			APPonline = false;
			LogInfo("APP offline\n");
		}

		//delay
		requested_time.tv_nsec = behLoopDelay * 1000000;
		nanosleep(&requested_time, &remaining);
	}
}

//report available scripts
int ReportAvailableScripts()
{
	//critical section
	int s = pthread_mutex_lock(&luaMtx);
	if (s != 0)
	{
		ERRORPRINT("BT: lua mutex lock %i\n", s);
	}

	int messageCount = AvailableScripts();

	s = pthread_mutex_unlock(&luaMtx);
	if (s != 0)
	{
		ERRORPRINT("BT: lua mutex unlock %i\n", s);
	}
	//end critical section
	return messageCount;
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
		DEBUGPRINT(".. success\n")
		return success(L);
		break;
	case ACTION_RUNNING:
		DEBUGPRINT(".. running\n")
		return running(L);
		break;
	default:
		DEBUGPRINT(".. fail\n")
		return fail(L);
		break;
	}
}

int ChangeOption(lua_State *L, const char *name, int value)
{
	psMessage_t msg;
	psInitPublish(msg, SET_OPTION);
	strncpy(msg.optionPayload.name, name, PS_NAME_LENGTH);
	msg.optionPayload.value = value;
	RouteMessage(&msg);
	LogInfo("lua: ChangeOption(%s, %i)\n", name, value);
	return success(L);
}

int ChangeSetting(lua_State *L, const char *name, float value)
{
	psMessage_t msg;
	psInitPublish(msg, NEW_SETTING);
	strncpy(msg.settingPayload.name, name, PS_NAME_LENGTH);
	msg.settingPayload.value = value;
	RouteMessage(&msg);
	LogInfo("lua: ChangeSetting(%s, %f)\n", name, value);
	return success(L);
}
