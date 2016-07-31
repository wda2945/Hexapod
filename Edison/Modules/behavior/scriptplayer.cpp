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

//registry
static int GetRegistry(lua_State *L);
static int SetRegistry(lua_State *L);

//logging
static int Print(lua_State *L);				//Print("...")
static int Alert(lua_State *L);
static int Fail(lua_State *L);

string  behaviorName = "Idle";
BehaviorStatus_enum behaviorStatus = BEHAVIOR_INVALID;

string lastActivityName = 	"none";
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

	ps_registry_add_new("Behavior Status", "active", PS_REGISTRY_TEXT_TYPE, PS_REGISTRY_SRC_WRITE);
	ps_registry_add_new("Behavior Status", "status", PS_REGISTRY_TEXT_TYPE, PS_REGISTRY_SRC_WRITE);
	ps_registry_add_new("Behavior Status", "fail at", PS_REGISTRY_TEXT_TYPE, PS_REGISTRY_SRC_WRITE);
	ps_registry_add_new("Behavior Status", "fail reason", PS_REGISTRY_TEXT_TYPE, PS_REGISTRY_SRC_WRITE);

	ps_registry_set_text("Behavior Status", "active", "idle");
	ps_registry_set_text("Behavior Status", "status", "active");

	//open standard libraries
	luaL_openlibs(btLuaState);

	luaopen_math(btLuaState);
	lua_setglobal(btLuaState, "math");
	luaopen_string(btLuaState);
	lua_setglobal(btLuaState, "string");

	//registry callbacks
	lua_pushcfunction(btLuaState, GetRegistry);			//access registry
	lua_setglobal(btLuaState, "GetRegistry");
	lua_pushcfunction(btLuaState, SetRegistry);			//access registry
	lua_setglobal(btLuaState, "SetRegistry");

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
	reply += LoadAllScripts(btLuaState);				//load all LUA scripts

	lua_pop(btLuaState, lua_gettop( btLuaState));		//clean stack

	if (reply == 0) return 0;
	else return -1;
}

//messages processed by scripting system
int ScriptProcessMessage(psMessage_t *msg)
{
	if (!btLuaState) return -1;

	switch (msg->messageType)
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
					ps_registry_set_text("Behavior Status", "active", msg->namePayload.name);
					ps_registry_set_text("Behavior Status", "status", "active");
					behaviorName = string(msg->namePayload.name);
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
		break;
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
					//change in activity or status

					ps_registry_set_text("Behavior Status", "status", behaviorStatusNames[statusCode]);

					switch (statusCode)
					{
					case BEHAVIOR_SUCCESS:
						LogInfo("%s SUCCESS", behaviorName.c_str());
						ps_registry_set_text("Behavior Status", "fail at", "");
						ps_registry_set_text("Behavior Status", "fail reason", "");
						break;
					case BEHAVIOR_RUNNING:
						ps_registry_set_text("Behavior Status", "fail at", "");
						ps_registry_set_text("Behavior Status", "fail reason", "");
						break;
					case BEHAVIOR_FAIL:
						LogInfo("%s FAIL @ %s - %s", behaviorName.c_str(), lastLuaCallFail.c_str(), lastLuaCallReason.c_str());
						ps_registry_set_text("Behavior Status", "fail at", lastLuaCallFail.c_str());
						ps_registry_set_text("Behavior Status", "fail reason", lastLuaCallReason.c_str());
						break;
					case BEHAVIOR_ACTIVE:
					case BEHAVIOR_INVALID:
					default:
						LogError("%s Bad Response", behaviorName.c_str());
						ps_registry_set_text("Behavior Status", "fail at", "update");
						ps_registry_set_text("Behavior Status", "fail reason", "Bad Status");
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
void AvailableScripts()
{
	//look up activities table
	lua_getglobal(btLuaState, "ActivityList");
	int table = lua_gettop( btLuaState);

	if (lua_istable(btLuaState, table) == 0)
	{
		//not a table
		LogError("No Activities table\n" );
		lua_pop(btLuaState, lua_gettop( btLuaState));
	}

	int index = 0;

    lua_pushnil(btLuaState);  /* first key */
    while (lua_next(btLuaState, table) != 0) {
      /* uses 'key' (at index -2) and 'value' (at index -1) */

    	const char *scriptName =  lua_tostring(btLuaState, -1);

    	ps_registry_add_new("Behaviors", scriptName, PS_REGISTRY_TEXT_TYPE, PS_REGISTRY_READ_ONLY);
    	ps_registry_set_text("Behaviors", scriptName, scriptName);

      /* removes 'value'; keeps 'key' for next iteration */
      lua_pop(btLuaState, 1);
    }
	lua_pop(btLuaState, lua_gettop( btLuaState));

	LogRoutine("%i activities", index);
}

//call-backs

static int GetRegistry(lua_State *L)		//GetRegistry('domain', 'name')
{
	const char *domain = lua_tostring(L,1);
	const char *name = lua_tostring(L,2);

    ps_registry_datatype_t type = ps_registry_get_type(domain, name);

    switch(type)
    {
    case PS_REGISTRY_UNKNOWN_TYPE:
    default:
    	return 0;
    	break;
    case PS_REGISTRY_INT_TYPE:
    {
    	int value;
    	if (ps_registry_get_int(domain, name, &value) != PS_OK) return 0;
    	lua_pushinteger(L, value);
    	return 1;
    }
    break;
    case PS_REGISTRY_REAL_TYPE:
    {
    	float value;
    	if (ps_registry_get_real(domain, name, &value) != PS_OK) return 0;
    	lua_pushnumber(L, value);
    	return 1;
    }
    break;
    case PS_REGISTRY_TEXT_TYPE:
    {
    	char value[100];
    	if (ps_registry_get_text(domain, name, value, 100) != PS_OK) return 0;
    	lua_pushstring(L, value);
    	return 1;
    }
    break;
    case PS_REGISTRY_BOOL_TYPE:
    {
    	bool value;
    	if (ps_registry_get_bool(domain, name, &value) != PS_OK) return 0;
    	lua_pushboolean(L, value);
    	return 1;
    }
    break;
    }
}

static int SetRegistry(lua_State *L)		//SetRegistry('domain', 'name', value)
{
	const char *domain = lua_tostring(L,1);
	const char *name = lua_tostring(L,2);

    ps_registry_datatype_t type = ps_registry_get_type(domain, name);

    switch(type)
    {
    case PS_REGISTRY_UNKNOWN_TYPE:
    default:
    	return 0;
    	break;
    case PS_REGISTRY_INT_TYPE:
    {
    	int value = lua_tointeger(L,3);
    	ps_registry_set_int(domain, name, value);
    	return 0;
    }
    break;
    case PS_REGISTRY_REAL_TYPE:
    {
    	float value = lua_tonumber(L,3);
    	ps_registry_set_real(domain, name, value);
    	return 0;
    }
    break;
    case PS_REGISTRY_TEXT_TYPE:
    {
    	const char *value = lua_tostring(L, 3);;
    	ps_registry_set_text(domain, name, value);
    	return 0;
    }
    break;
    case PS_REGISTRY_BOOL_TYPE:
    {
    	bool value = lua_toboolean(L,3);
    	ps_registry_set_bool(domain, name, value);
    	return 0;
    }
    break;
    }
}


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

