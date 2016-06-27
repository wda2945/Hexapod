/*
 * ScriptPlayer.c
 *
 *  Created on: Aug 10, 2014
 *      Author: martin
 */
// Controller of the LUA subsystem

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <dirent.h>
#include <string.h>
#include <string>

#include "lua.hpp"

#include "hexapod.h"

#include "behavior/behavior.hpp"
#include "behavior/behavior_enums.h"
#include "behavior/behaviorDebug.h"
#include "autopilot/autopilot.hpp"

using namespace std;

lua_State	*btLuaState = NULL;

//logging
static int Print(lua_State *L);				//Print("...")
static int Alert(lua_State *L);
static int Fail(lua_State *L);

string  behaviorName = "Idle";
BehaviorStatus_enum behaviorStatus = BEHAVIOR_INVALID;

string lastActivityName = "none";
string lastActivityStatus = "invalid";

string lastLuaCall 			= "";
string lastLuaCallReason 	= "";
string lastLuaCallFail 		= "";

//allocator for lua state
static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize)
{
	(void)ud;  (void)osize;  /* not used */
	if (nsize == 0)
	{
		free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}

int InitScriptingSystem()
{
	//create a new LUA state
	if (btLuaState)
	{
		//close if previously opened
		lua_close(btLuaState);
	}
	btLuaState = lua_newstate(l_alloc, NULL);

	if (btLuaState == NULL)
	{
		ERRORPRINT("luaState create fail\n");
	   	return -1;
	}

	//open standard libraries
	luaL_openlibs(btLuaState);

	luaopen_math(btLuaState);
	lua_setglobal(btLuaState, "math");
	luaopen_string(btLuaState);
	lua_setglobal(btLuaState, "string");

	//register basic call-backs
	lua_pushcfunction(btLuaState, Alert);				//Alert Message to App
	lua_setglobal(btLuaState, "Alert");
	lua_pushcfunction(btLuaState, Print);				//Print Message
	lua_setglobal(btLuaState, "Print");
	lua_pushcfunction(btLuaState, Fail);				//Note fail() - Fail('name')
	lua_setglobal(btLuaState, "Fail");

	int reply = 0;
	reply += InitPilotingCallbacks(btLuaState);			//set BT-related call-backs
	reply += InitProximityCallbacks(btLuaState);
	reply += InitSystemCallbacks(btLuaState);
	reply += InitLuaGlobals(btLuaState);				//load system global constants
	reply += LoadAllScripts(btLuaState);				//load all LUA scripts

	lua_pop(btLuaState, lua_gettop( btLuaState));		//clean stack

	if (reply == 0) return 0;
	else return -1;
}

//messages processed by scripting system
int ScriptProcessMessage(psMessage_t *msg)
{
	if (!btLuaState) return -1;

	switch (msg->header.messageType)
	{
	case RELOAD:
		//reload all scripts
		LogInfo("Reload scripts\n");
		if (InitScriptingSystem() < 0)
		{
			LogError("Error on Reload scripts\n");
		}
		break;

	case ACTIVATE:
		//start BT activity
		LogInfo("Activate: %s\n", msg->namePayload.name);

		lua_getglobal(btLuaState, "activate");						//reference to 'activate(...)
		if (lua_isfunction(btLuaState,lua_gettop(btLuaState)))
		{
			//activate() exists
			lua_pushstring(btLuaState, msg->namePayload.name);		//name of target BT
			lua_getglobal(btLuaState, msg->namePayload.name);		//reference to the tree itself
			if (lua_istable(btLuaState,lua_gettop(btLuaState)))
			{
				//target table exists - call activate()
				int status = lua_pcall(btLuaState, 2, 0, 0);
				if (status)
				{
					const char *errormsg = lua_tostring(btLuaState, -1);
					LogError("%s, Error: %s\n",msg->namePayload.name,errormsg);
				}
				else
				{
					behaviorName = string(msg->namePayload.name);
					behaviorStatus = BEHAVIOR_ACTIVE;
				}
			}
			else
			{
				//target table does not exist
				LogError("%s is type %s\n",msg->namePayload.name,lua_typename(btLuaState,lua_type(btLuaState,lua_gettop(btLuaState))) );
			}
		}
		else
		{
			//activate() does not exist
			LogError("activate is type %s\n",lua_typename(btLuaState,lua_type(btLuaState,lua_gettop(btLuaState))) );
		}
		lua_pop(btLuaState, lua_gettop( btLuaState));
		break;

	default:
		//process others to update globals
		return UpdateGlobalsFromMessage(btLuaState, msg);
	}
	return 0;
}

static const char *behaviorStatusNames[BEHAVIOR_STATUS_COUNT] = BEHAVIOR_STATUS_NAMES;
//periodic behavior tree update invocation
int InvokeUpdate()
{
	if (!btLuaState) return -1;

	switch (behaviorStatus)
	{
	case BEHAVIOR_ACTIVE:
	case BEHAVIOR_RUNNING:

		lua_getglobal(btLuaState, "update");

		if (lua_isfunction(btLuaState,lua_gettop(btLuaState)))
		{
			//		DEBUGPRINT("LUA: calling update\n");
			int reply = lua_pcall(btLuaState, 0, 1, 0);
			if (reply)
			{
				const char *errormsg = lua_tostring(btLuaState, -1);
				LogInfo("Script update, Error: %s\n",errormsg);
				lua_pop(btLuaState, lua_gettop( btLuaState));
				return -1;
			}
			else
			{
				int i;
				size_t len;
				const char *status = lua_tolstring(btLuaState,1, &len);

				DEBUGPRINT("LUA: returned from update (%s)\n", status);

				BehaviorStatus_enum statusCode = BEHAVIOR_INVALID;
				for ( i=2; i<BEHAVIOR_STATUS_COUNT; i++)
				{
					if (strncmp(behaviorStatusNames[i], status, 4) == 0)
					{
						statusCode = (BehaviorStatus_enum) i;
						break;
					}
				}

				if ((statusCode != behaviorStatus) || (behaviorName.compare(lastActivityName) != 0))
				{
					psMessage_t activityMsg;

					//change in activity or status

					psInitPublish(activityMsg, ACTIVITY);
					strncpy(activityMsg.behaviorStatusPayload.behavior, behaviorName.c_str(), PS_SHORT_NAME_LENGTH);
					activityMsg.behaviorStatusPayload.status = statusCode;

					switch (statusCode)
					{
					case BEHAVIOR_SUCCESS:
						LogInfo("%s SUCCESS", behaviorName.c_str());
						activityMsg.behaviorStatusPayload.lastLuaCallFail[0] = '\0';
						activityMsg.behaviorStatusPayload.lastFailReason[0] = '\0';
						RouteMessage(activityMsg);
						break;
					case BEHAVIOR_RUNNING:
						activityMsg.behaviorStatusPayload.lastLuaCallFail[0] = '\0';
						activityMsg.behaviorStatusPayload.lastFailReason[0] = '\0';
						RouteMessage(activityMsg);
						break;
					case BEHAVIOR_FAIL:
						LogInfo("%s FAIL @ %s - %s", behaviorName.c_str(), lastLuaCallFail.c_str(), lastLuaCallReason.c_str());
						strncpy(activityMsg.behaviorStatusPayload.lastLuaCallFail, lastLuaCallFail.c_str(), PS_SHORT_NAME_LENGTH);
						strncpy(activityMsg.behaviorStatusPayload.lastFailReason, lastLuaCallReason.c_str(), PS_SHORT_NAME_LENGTH);
						RouteMessage(activityMsg);
						break;
					case BEHAVIOR_ACTIVE:
					case BEHAVIOR_INVALID:
					default:
						LogError("%s Bad Response", behaviorName.c_str());
						strncpy(activityMsg.behaviorStatusPayload.lastLuaCallFail, "update", PS_SHORT_NAME_LENGTH);
						strncpy(activityMsg.behaviorStatusPayload.lastFailReason, "Bad Status", PS_SHORT_NAME_LENGTH);
						RouteMessage(activityMsg);
						break;
					}

//					DEBUGPRINT("Activity %s, status: %s\n", behaviorName.c_str(), status);
					behaviorStatus = statusCode;
				}

				lua_pop(btLuaState, lua_gettop( btLuaState));

				lastActivityName = behaviorName;
				return 0;
			}
		}
		else
		{
			LogError("Global update is of type %s\n",lua_typename(btLuaState,lua_type(btLuaState,lua_gettop(btLuaState))) );
			lua_pop(btLuaState, lua_gettop( btLuaState));
			return -1;
		}
		break;
	default:
		break;
	}
	return 0;
}

//report available activity scripts
int AvailableScripts()
{
	psMessage_t msg;
	int messageCount = 0;

	psInitPublish(msg, SCRIPT);

	//look up activities table
	lua_getglobal(btLuaState, "ActivityList");
	int table = lua_gettop( btLuaState);

	if (lua_istable(btLuaState, table) == 0)
	{
		//not a table
		LogError("No Activities table\n" );
		lua_pop(btLuaState, lua_gettop( btLuaState));
		return 0;
	}

    lua_pushnil(btLuaState);  /* first key */
    while (lua_next(btLuaState, table) != 0) {
      /* uses 'key' (at index -2) and 'value' (at index -1) */
		strncpy(msg.namePayload.name, lua_tostring(btLuaState, -1), PS_NAME_LENGTH);
		RouteMessage(msg);
		messageCount++;

		//delay
		usleep( MESSAGE_DELAY * 1000);

      /* removes 'value'; keeps 'key' for next iteration */
      lua_pop(btLuaState, 1);
    }
	lua_pop(btLuaState, lua_gettop( btLuaState));

	LogRoutine("%i activities", messageCount);

	return messageCount;
}

//call-backs

//Alert
static int Alert(lua_State *L)				//Alert("...")
{
	const char *text = lua_tostring(L,1);

	psMessage_t msg;
	psInitPublish(msg, ALERT);
	strncpy(msg.namePayload.name, text, PS_NAME_LENGTH);
	RouteMessage(msg);
	LogInfo("lua: Alert (%s)\n", text);

	return 0;
}

//print
static int Print(lua_State *L)				//Print("...")
{
	const char *text = lua_tostring(L,1);
	LogRoutine("lua: %s\n",text);
	return 0;
}
//static int DebugPrint(lua_State *L)				//Print("...")
//{
//#ifdef BEHAVIOR_TREE_DEBUG
//	const char *text = lua_tostring(L,1);
//	DEBUGPRINT("lua: %s\n",text);
//#endif
//	return 0;
//}
//static int ErrorPrint(lua_State *L)				//Print("...")
//{
//	const char *text = lua_tostring(L,1);
//	ERRORPRINT("lua: %s\n",text);
//	return 0;
//}
//Fail
char failBuffer[PS_NAME_LENGTH];
static int Fail(lua_State *L)				//Fail('name')
{
	const char *name = lua_tostring(L,1);

	if (name)
	{
		strncpy(failBuffer, name, PS_NAME_LENGTH);
		lastLuaCallFail = failBuffer;
	}

	LogRoutine("lua: fail at %s\n",name);
	return 0;
}

