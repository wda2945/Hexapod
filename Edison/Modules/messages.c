/* 
 * File:   Messages.c
 * Author: martin
 *
 * Enums and structs of Messages
 *
 */

#include <string.h>
#include "messages.h"

ps_message_id_t get_message_id(const char *message_name)
{
#define messagemacro(m,q,t,f,l) if (strcmp(l, message_name)==0) return m;
#include "Messages/MessageList.h"
#undef messagemacro

	return 0;
}
const char *get_message_name(ps_message_id_t message_id)
{
#define messagemacro(m,q,t,f,l) if (message_id == m) return l;
#include "Messages/MessageList.h"
#undef messagemacro

	return "";
}

ps_topic_id_t	get_topic_id(const char *topic_name)
{
	return 0;
}

ps_topic_id_t 	get_message_topic(ps_message_id_t message_id)
{
#define messagemacro(m,q,t,f,l) if (message_id == m) return t;
#include "Messages/MessageList.h"
#undef messagemacro

	return 0;
}
ps_message_qos_t 	get_message_QoS(ps_message_id_t message_id)
{
#define messagemacro(m,q,t,f,l) if (message_id == m) return q;
#include "Messages/MessageList.h"
#undef messagemacro

	return 0;
}
int	 			get_message_payload_type(ps_message_id_t message_id)
{
#define messagemacro(m,q,t,f,l) if (message_id == m) return f;
#include "Messages/MessageList.h"
#undef messagemacro

	return 0;
}

char *subsystemNames[SUBSYSTEM_COUNT] = SUBSYSTEM_NAMES;
char *psTopicNames[PS_TOPIC_COUNT] = PS_TOPIC_NAMES;

//#define messagemacro(enum, short name, qos, topic, payload, long name)
#define messagemacro(m,q,t,f,l) f,
int psMsgFormats[PS_MSG_COUNT] = {
#include "messages/messagelist.h"
};
#undef messagemacro

#define messagemacro(m,q,t,f,l) t,
int psDefaultTopics[PS_MSG_COUNT] = {
#include "messages/messagelist.h"
};
#undef messagemacro

#define messagemacro(m,q,t,f,l) l,
char *psLongMsgNames[PS_MSG_COUNT] = {
#include "messages/messagelist.h"
};
#undef messagemacro

#define messagemacro(m,q,t,f,l) q,
psQOS_enum psQOS[PS_MSG_COUNT] = {
#include "messages/messagelist.h"
};
#undef messagemacro

#define formatmacro(e,t,v,s) s,
int psMessageFormatLengths[PS_FORMAT_COUNT] = {
#include "messages/msgformatlist.h"
};
#undef formatmacro

//options
#define optionmacro(name, var, min, max, def) int var = def;
#include "options.h"
#undef optionmacro

//Settings
#define settingmacro(name, var, min, max, def) float var = def;
#include "settings.h"
#undef settingmacro

#define EVENT(e, n) n,
char *eventNames[] 	= {
#include "messages/notificationeventslist.h"
};
#undef EVENT

#define CONDITION(e, n) n,
char *conditionNames[]		= {
#include "messages/notificationconditionslist.h"
};
#undef CONDITION

char *batteryStateNames[] 	= BATTERY_STATUS_NAMES;
char *stateCommandNames[] 	= USER_COMMAND_NAMES;
char *powerStateNames[] 	= POWER_STATE_NAMES;
char *ovmCommandNames[] 	= OVERMIND_COMMAND_NAMES;
char *arbStateNames[]		= ARB_STATE_NAMES;
