/*
 * BT Callbacks.c
 *
 *  Created on: Aug 10, 2014
 *      Author: martin
 */
// BT Leaf callbacks for lua

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <dirent.h>
#include <string.h>

#include "lua.h"
#include "lualib.h"

#include "PubSubData.h"
#include "pubsub/pubsub.h"

#include "behavior/behavior_enums.h"
#include "behavior/behavior.h"
#include "behavior/behaviorDebug.h"

#include "gripper/gripper.h"
#include "syslog/syslog.h"

//actual leaf node
static int GripperAction(lua_State *L);

typedef enum {
	setGripperSpeedSlow,
	setGripperSpeedFast,
	openGripper,
	closeGripper,
	GRIPPER_ACTION_COUNT
} GripperAction_enum;

static char *gripperActionList[] = {
	"setGripperSpeedSlow",
	"setGripperSpeedFast",
	"openGripper",
	"closeGripper"
};

int InitGripperCallbacks(lua_State *L)
{
	int i, table;
	lua_pushcfunction(L, GripperAction);
	lua_setglobal(L, "GripperAction");

	lua_createtable(L, 0, GRIPPER_ACTION_COUNT);
	table = lua_gettop(L);
	for (i=0; i< GRIPPER_ACTION_COUNT; i++)
	{
		lua_pushstring(L, gripperActionList[i]);
		lua_pushinteger(L, i);
		lua_settable(L, table);
	}
	lua_setglobal(L, "gripper");

	return 0;
}


static int GripperAction(lua_State *L)
{
	GripperAction_enum actionCode 	= lua_tointeger(L, 1);
	
	lastLuaCall = gripperActionList[actionCode];

	LogRoutine("GripperAction: %s", gripperActionList[actionCode]);

	switch (actionCode)
	{
	case setGripperSpeedSlow:
		gripperSpeed = GRIPPER_SLOW;
		return success(L);
		break;
	case setGripperSpeedFast:
		gripperSpeed = GRIPPER_FAST;
		return success(L);
		break;
	case openGripper:
		return actionReply(L, MoveGripper(GRIPPER_OPEN));
		break;
	case closeGripper:
		return actionReply(L, MoveGripper(GRIPPER_CLOSED));
		break;
	default:
		LogError("Gripper action: %i\n", actionCode);
		break;
	}
	return fail(L);
}
