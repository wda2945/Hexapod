/*
 * callbacks_piloting.c
 *
 *  Created on: Aug 10, 2014
 *      Author: martin
 */
// BT Leaf LUA callbacks for piloting

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
#include "autopilot/autopilot.hpp"
#include "navigator/navigator.hpp"


//actual leaf node
static int PilotAction(lua_State *L);

typedef enum {
	isPilotReady,
	ComputeHomePosition,
	ComputeRandomExplorePosition,
	ComputeRandomClosePosition,
	Orient,
	Engage,

	PILOTING_ACTION_COUNT
} PilotingAction_enum;

static const char *pilotingActionList[] = {
		"isPilotReady",
		"ComputeHomePosition",
		"ComputeRandomExplorePosition",
		"ComputeRandomClosePosition",
		"Orient",
		"Engage",
};

Position_struct heelPosition;
bool	heelPositionValid = false;

int InitPilotingCallbacks(lua_State *L)
{
	int i, table;
	lua_pushcfunction(L, PilotAction);
	lua_setglobal(L, "PilotAction");

	lua_createtable(L, 0, PILOTING_ACTION_COUNT);
	table = lua_gettop(L);
	for (i=0; i< PILOTING_ACTION_COUNT; i++)
	{
		lua_pushstring(L, pilotingActionList[i]);
		lua_pushinteger(L, i);
		lua_settable(L, table);
	}
	lua_setglobal(L, "pilot");

	return 0;
}

static int PilotAction(lua_State *L)
{
	PilotingAction_enum actionCode 	= (PilotingAction_enum) lua_tointeger(L, 1);

	lastLuaCall = pilotingActionList[actionCode];

	DEBUGPRINT("Pilot Action: %s ...", lastLuaCall.c_str());

	switch (actionCode)
	{
	case isPilotReady:
		return actionReply(L, AutopilotIsReadyToMove());
		break;
	case ComputeHomePosition:
		return fail(L); //actionReply(L, pilotSetGoalWaypoint("Home"));
		break;
	case ComputeRandomExplorePosition:
		return actionReply(L, pilotSetRandomGoal(500)); 		//1 meter
		break;
	case ComputeRandomClosePosition:
		return actionReply(L, pilotSetRandomGoal(30)); 		//1 foot
		break;
	case Orient:
		return actionReply(L, AutopilotAction(PILOT_ORIENT));
		break;
	case Engage:
		return actionReply(L, AutopilotAction(PILOT_ENGAGE));
		break;

	default:
		ERRORPRINT("Pilot action: %i", actionCode);
		return fail(L);
		break;
	}

}
