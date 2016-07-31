/*
 * callbacks_arbotix.c
 *
 *  Created on: Aug 10, 2014
 *      Author: martin
 */
// BT Leaf LUA callbacks for arbotix operation

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <dirent.h>
#include <string.h>

#include "lua.hpp"

#include "hexapod.h"

#include "behavior/behavior_enums.h"
#include "behavior/behavior.hpp"
#include "behavior/behaviorDebug.h"
#include "arbotix/arbotix.hpp"

//LUA leaf actions for hexapod

typedef enum {
	Stand,
	Sit,
	Turn,
	TurnLeft,
	TurnRight,
	TurnN,
	TurnS,
	TurnE,
	TurnW,
	TurnLeft90,
	TurnRight90,
	MoveForward,
	MoveBackward,
	MoveForward10,
	MoveBackward10,
	SetFastSpeed,
	SetMediumSpeed,
	SetLowSpeed,
	EnableFrontCloseStop,
	DisableFrontCloseStop,
	EnableRearCloseStop,
	DisableRearCloseStop,
	EnableFrontFarStop,
	DisableFrontFarStop,
	EnableSystemStop,
	DisableSystemStop,
	HEXAPOD_ACTION_COUNT
} ArbotixAction_enum;

static const char *arbotixActionList[] = {
		"Stand",
		"Sit",
		"Turn",
		"TurnLeft",
		"TurnRight",
		"TurnN",
		"TurnS",
		"TurnE",
		"TurnW",
		"TurnLeft90",
		"TurnRight90",
		"MoveForward",
		"MoveBackward",
		"MoveForward10",
		"MoveBackward10",
		"SetFastSpeed",
		"SetMediumSpeed",
		"SetLowSpeed",
		"EnableFrontCloseStop",
		"DisableFrontCloseStop",
		"EnableRearCloseStop",
		"DisableRearCloseStop",
		"EnableFrontFarStop",
		"DisableFrontFarStop",
		"EnableSystemStop",
		"DisableSystemStop",
};

//actual leaf node
static int luaHexapodAction(lua_State *L);

int InitHexapodCallbacks(lua_State *L)
{
	int i, table;
	lua_pushcfunction(L, luaHexapodAction);
	lua_setglobal(L, "HexapodAction");

	lua_createtable(L, 0, HEXAPOD_ACTION_COUNT);
	table = lua_gettop(L);
	for (i=0; i< HEXAPOD_ACTION_COUNT; i++)
	{
		lua_pushstring(L, arbotixActionList[i]);
		lua_pushinteger(L, i);
		lua_settable(L, table);
	}
	lua_setglobal(L, "hexapod");

	return 0;
}

static int luaHexapodAction(lua_State *L)
{
	ArbotixAction_enum actionCode 	= (ArbotixAction_enum) lua_tointeger(L, 1);

	lastLuaCall = arbotixActionList[actionCode];

	LogRoutine("ArbotixAction: %s ...\n", lastLuaCall.c_str());

	switch (actionCode)
	{
	case Stand:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_STAND));
		break;
	case Sit:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_SIT));
		break;
	case Turn:
		if (drand48() > 0.5)
			return actionReply(L, HexapodExecuteAction(HEXAPOD_TURN_LEFT));
		else
			return actionReply(L, HexapodExecuteAction(HEXAPOD_TURN_RIGHT));
		break;
	case TurnLeft:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_TURN_LEFT));
		break;
	case TurnRight:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_TURN_RIGHT));
		break;
	case TurnN:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_TURN_N));
		break;
	case TurnS:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_TURN_S));
		break;
	case TurnE:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_TURN_E));
		break;
	case TurnW:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_TURN_W));
		break;
	case TurnLeft90:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_TURN_LEFT_90));
		break;
	case TurnRight90:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_TURN_RIGHT_90));
		break;
	case MoveForward:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_MOVE_FORWARD));
		break;
	case MoveBackward:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_MOVE_BACKWARD));
		break;
	case MoveForward10:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_MOVE_FORWARD_10));
		break;
	case MoveBackward10:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_MOVE_BACKWARD_10));
		break;
	case SetFastSpeed:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_MOVE_BACKWARD));
		break;
	case SetMediumSpeed:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_MOVE_BACKWARD));
		break;
	case SetLowSpeed:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_MOVE_BACKWARD));
		break;
	case EnableFrontCloseStop:
		movementAbortFlags |= ENABLE_FRONT_CLOSE_ABORT;
		return success(L);
		break;
	case DisableFrontCloseStop:
		movementAbortFlags &= ~ENABLE_FRONT_CLOSE_ABORT;
		return success(L);
		break;
	case EnableRearCloseStop:
		movementAbortFlags |= ENABLE_REAR_CLOSE_ABORT;
		return success(L);
		break;
	case DisableRearCloseStop:
		movementAbortFlags &= ~ENABLE_REAR_CLOSE_ABORT;
		return success(L);
		break;
	case EnableFrontFarStop:
		movementAbortFlags |= ENABLE_FRONT_FAR_ABORT;
		return success(L);
		break;
	case DisableFrontFarStop:
		movementAbortFlags &= ~ENABLE_FRONT_FAR_ABORT;
		return success(L);
		break;
	case EnableSystemStop:
		movementAbortFlags |= ENABLE_SYSTEM_ABORT;
		return success(L);
		break;
	case DisableSystemStop:
		movementAbortFlags &= ~ENABLE_SYSTEM_ABORT;
		return success(L);
		break;

	default:
		LogError("Arb action: %i\n", actionCode);
		return fail(L);
		break;
	}

}
