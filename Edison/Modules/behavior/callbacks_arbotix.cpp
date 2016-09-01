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
	Stop,
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
	MoveForward30,
	MoveBackward30,
	SetFastSpeed,
	SetMediumSpeed,
	SetLowSpeed,
	SetPoseMode,
	SetPoseSlow,
	SetPoseMedium,
	SetPoseFast,
	SetPoseBeat,
	SetPoseDownbeat,
	SetPoseUpbeat,
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
		"Stop",
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
		"MoveForward30",
		"MoveBackward30",
		"SetFastSpeed",
		"SetMediumSpeed",
		"SetLowSpeed",
		"SetPoseMode",
		"SetPoseSlow",
		"SetPoseMedium",
		"SetPoseFast",
		"SetPoseBeat",
		"SetPoseDownbeat",
		"SetPoseUpbeat",
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
static int luaHexapodPose(lua_State *L);

int InitHexapodCallbacks(lua_State *L)
{
	int i, table;
	lua_pushcfunction(L, luaHexapodAction);
	lua_setglobal(L, "HexapodAction");

	lua_pushcfunction(L, luaHexapodPose);
	lua_setglobal(L, "HexapodPose");

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

static int luaHexapodPose(lua_State *L)
{
	return HexapodAssumePose(lua_tostring(L, 1));
}

static int luaHexapodAction(lua_State *L)
{
	ArbotixAction_enum actionCode 	= (ArbotixAction_enum) lua_tointeger(L, 1);

	lastLuaCall = arbotixActionList[actionCode];

	DEBUGPRINT("Hexapod Action: %s ...", lastLuaCall.c_str());

	switch (actionCode)
	{
	case Stop:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_STOP));
		break;
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
	case MoveForward30:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_MOVE_FORWARD_30));
		break;
	case MoveBackward30:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_MOVE_BACKWARD_30));
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
	case SetPoseMode:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_POSE_MODE));
		break;
	case SetPoseSlow:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_POSE_SLOW));
		break;
	case SetPoseMedium:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_POSE_MEDIUM));
		break;
	case SetPoseFast:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_POSE_FAST));
		break;
	case SetPoseBeat:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_POSE_BEAT));
		break;
	case SetPoseDownbeat:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_POSE_DOWNBEAT));
		break;
	case SetPoseUpbeat:
		return actionReply(L, HexapodExecuteAction(HEXAPOD_POSE_UPBEAT));
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
		ERRORPRINT("Arb action: %i", actionCode);
		return fail(L);
		break;
	}

}
