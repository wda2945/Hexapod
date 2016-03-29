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

#include "lidar/lidar.h"
#include "syslog/syslog.h"

//actual leaf node
static int ProximityAction(lua_State *L);

typedef enum {
	isFrontLeftProximity,
	isFrontProximity,
	isFrontRightProximity,
	isRearLeftProximity,
	isRearProximity,
	isRearRightProximity,
	isLeftProximity,
	isRightProximity,
	isFrontLeftFarProximity,
	isFrontFarProximity,
	isFrontRightFarProximity,
	PROXIMITY_ACTION_COUNT
} ProximityAction_enum;

static char *proximityActionList[] = {
		"isFrontLeftProximity",
		"isFrontProximity",
		"isFrontRightProximity",
		"isRearLeftProximity",
		"isRearProximity",
		"isRearRightProximity",
		"isLeftProximity",
		"isRightProximity",
		"isFrontLeftFarProximity",
		"isFrontFarProximity",
		"isFrontRightFarProximity",
};

int InitProximityCallbacks(lua_State *L)
{
	int i, table;
	lua_pushcfunction(L, ProximityAction);
	lua_setglobal(L, "ProximityAction");

	lua_createtable(L, 0, PROXIMITY_ACTION_COUNT);
	table = lua_gettop(L);
	for (i=0; i< PROXIMITY_ACTION_COUNT; i++)
	{
		lua_pushstring(L, proximityActionList[i]);
		lua_pushinteger(L, i);
		lua_settable(L, table);
	}
	lua_setglobal(L, "proximity");

	return 0;
}


static int ProximityAction(lua_State *L)
{
	ProximityAction_enum actionCode 	= lua_tointeger(L, 1);
	
	LogRoutine("ProximityAction: %s", proximityActionList[actionCode]);

	switch (actionCode)
	{
	case isFrontLeftProximity:
		return actionReply(L, proximityStatus(PROX_FRONT_LEFT_MASK, PROX_CLOSE_MASK));
		break;
	case isFrontProximity:
		return actionReply(L, proximityStatus(PROX_FRONT_MASK | PROX_FRONT_RIGHT_MASK | PROX_FRONT_LEFT_MASK, PROX_CLOSE_MASK));
		break;
	case isFrontRightProximity:
		return actionReply(L, proximityStatus(PROX_FRONT_RIGHT_MASK, PROX_CLOSE_MASK));
		break;
	case isRearLeftProximity:
		return actionReply(L, proximityStatus(PROX_REAR_LEFT_MASK, PROX_CLOSE_MASK));
		break;
	case isRearProximity:
		return actionReply(L, proximityStatus(PROX_REAR_MASK | PROX_REAR_RIGHT_MASK | PROX_REAR_LEFT_MASK, PROX_CLOSE_MASK));
		break;
	case isRearRightProximity:
		return actionReply(L, proximityStatus(PROX_REAR_RIGHT_MASK, PROX_CLOSE_MASK));
		break;
	case isLeftProximity:
		return actionReply(L, proximityStatus(PROX_LEFT_MASK, PROX_CLOSE_MASK));
		break;
	case isRightProximity:
		return actionReply(L, proximityStatus(PROX_RIGHT_MASK, PROX_CLOSE_MASK));
		break;
	case isFrontLeftFarProximity:
		return actionReply(L, proximityStatus(PROX_FRONT_LEFT_MASK, PROX_CLOSE_MASK | PROX_FAR_MASK));
		break;
	case isFrontFarProximity:
		return actionReply(L, proximityStatus(PROX_FRONT_LEFT_MASK | PROX_FRONT_RIGHT_MASK, PROX_CLOSE_MASK | PROX_FAR_MASK));
		break;
	case isFrontRightFarProximity:
		return actionReply(L, proximityStatus(PROX_FRONT_RIGHT_MASK, PROX_CLOSE_MASK | PROX_FAR_MASK));
		break;
	default:
		LogError("Prox action: %i\n", actionCode);
		break;
	}
	return fail(L);
}
