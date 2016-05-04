/*
 * BT Callbacks.c
 *
 *  Created on: Aug 10, 2014
 *      Author: martin
 */
// BT Leaf callbacks for lua
// System Callbacks

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>

#include "lua.h"
#include "lualib.h"

#include "PubSubData.h"
#include "pubsub/pubsub.h"
#include "pubsub/notifications.h"

#include "behavior/behavior_enums.h"
#include "behavior/behavior.h"
#include "behavior/behaviorDebug.h"
#include "syslog/syslog.h"

//actual leaf node
static int SystemAction(lua_State *L);

int SaveSettingsAndOptions(lua_State *L, bool useDefault);
int LoadSettingsAndOptions(lua_State *L);

typedef enum {
	SaveSettings,
	LoadSettings,
	ResetSavedSettings,
    SystemPoweroff,
    SystemSetResting,
    SystemSetActive,
    ReloadScripts,
    isAppOnline,
	SYSTEM_ACTION_COUNT
} SystemAction_enum;

static char *systemActionList[] = {
		"SaveSettings",
		"LoadSettings",
		"ResetSavedSettings",
	    "SystemPoweroff",
	    "SystemSetResting",
	    "SystemSetActive",
	    "ReloadScripts",
	    "isAppOnline",
};

int InitSystemCallbacks(lua_State *L)
{
	int i, table;
	lua_pushcfunction(L, SystemAction);
	lua_setglobal(L, "SystemAction");

	lua_createtable(L, 0, SYSTEM_ACTION_COUNT);
	table = lua_gettop(L);
	for (i=0; i< SYSTEM_ACTION_COUNT; i++)
	{
		lua_pushstring(L, systemActionList[i]);
		lua_pushinteger(L, i);
		lua_settable(L, table);
	}
	lua_setglobal(L, "system");

	return 0;
}


static int SystemAction(lua_State *L)
{
	psMessage_t msg;

	SystemAction_enum actionCode 	= lua_tointeger(L, 1);

	lastLuaCall = systemActionList[actionCode];

	LogInfo("SystemAction: %s", systemActionList[actionCode]);

	switch (actionCode)
	{
	case SaveSettings:
		return SaveSettingsAndOptions(L, false);
		break;
	case LoadSettings:
		return LoadSettingsAndOptions(L);
		break;
	case ResetSavedSettings:
		return SaveSettingsAndOptions(L, true);
		break;
	case SystemPoweroff:
		reboot(RB_POWER_OFF);
		return success(L);
		break;
	case ReloadScripts:
		psInitPublish(msg, RELOAD);
		RouteMessage(&msg);
		return success(L);
		break;
	case isAppOnline:
		if (APPonline)
			return success(L);
		else
			return fail(L);
		break;
	default:
		LogError("Sys code: %i\n", actionCode);
		return fail(L);
		break;
	}
}

int SaveSettingsAndOptions(lua_State *L, bool useDefault)
{
	bool actionFail = false;

	FILE *fp = fopen(SAVED_SETTINGS_FILE, "w");
	if (fp)
	{
		//Settings
#define settingmacro(name, var, min, max, def) if (fprintf(fp, "'%s';%f\n", name, (useDefault ? def : var)) < 0) actionFail = true;
#include "settings.h"
#undef settingmacro

		fclose(fp);

		fp = fopen(SAVED_OPTIONS_FILE, "w");
		if (fp)
		{
			//options
#define optionmacro(name, var, min, max, def) if (fprintf(fp, "'%s';%i\n", name, (useDefault ? def : var)) < 0) actionFail = true;
#include "options.h"
#undef optionmacro

			fclose(fp);

			if (actionFail) {
				return fail(L);
			}
			else
			{
				LogRoutine("Settings and Options Saved\n");
				return success(L);
			}
		}
		else
		{
			LogError("Options.txt - %s\n", strerror(errno));
			return fail(L);
		}
	}
	else
	{
		LogError("Options.txt - %s\n", strerror(errno));
		return  fail(L);
	}
}

int LoadSettingsAndOptions(lua_State *L)
{
	char name[80];
	float setting;
	int option;
	int result;

	FILE *fp = fopen(SAVED_SETTINGS_FILE, "r");
	if (fp)
	{
		do {
			result = fscanf(fp, "'%79s';%f\n", name, &setting);
			if (result  == 2)
			{
				//Settings
#define settingmacro(n, var, min, max, def) if (strncmp(name, n, 80) == 0) var = setting;
#include "settings.h"
#undef settingmacro
			}
			else if (result < 0)
			{
				LogError("Settings read: %s\n", strerror(errno));
				return fail(L);
			}
		} while (result > 0);

		fclose(fp);

		fp = fopen(SAVED_OPTIONS_FILE, "r");
		if (fp)
		{
			do {
				result = fscanf(fp, "'%79s';%i\n", name, &option);
				if (result  == 2)
				{
					//options
#define optionmacro(n, var, min, max, def) if (strncmp(name, n, 80) == 0) var = option;
#include "options.h"
#undef optionmacro
				}
				else if (result < 0)
				{
					LogError("Options read: %s\n", strerror(errno));
					return fail(L);
				}
			} while (result > 0);

			fclose(fp);

			LogRoutine("Settings and Options Saved\n");
			return success(L);
		}
		else
		{
			LogError("Settings.txt: %s\n", strerror(errno));
			return  fail(L);
		}
	}
	else
	{
		LogError("Settigns.txt: %s\n", strerror(errno));
		return  fail(L);
	}
}
