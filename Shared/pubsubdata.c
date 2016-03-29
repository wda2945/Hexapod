/*
 * PubSubData.c
 *
 *	PubSub Data
 *
 *  Created on: Jan 19, 2014
 *      Author: martin
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "pubsubdata.h"
//#include "syslog/syslog.h"
//#include "broker/brokerQ.h"

//#define PSDEBUG(x) printf("%s: %s\n", processNames[process_id], x)
#define PSDEBUG(x)

char *subsystemNames[] = SUBSYSTEM_NAMES;
char *psTopicNames[] = PS_TOPIC_NAMES;

//#define messagemacro(enum, short name, qos, topic, payload format, long name)
#define messagemacro(m,q,t,f,l) f,
int psMsgFormats[PS_MSG_COUNT] = {
#include "messages/messageList.h"
};
#undef messagemacro

#define messagemacro(m,q,t,f,l) t,
int psDefaultTopics[PS_MSG_COUNT] = {
#include "messages/messageList.h"
};
#undef messagemacro

#define messagemacro(m,q,t,f,l) l,
char *psLongMsgNames[PS_MSG_COUNT] = {
#include "messages/messageList.h"
};
#undef messagemacro

#define messagemacro(m,q,t,f,l) q,
psQOS_enum psQOS[PS_MSG_COUNT] = {
#include "messages/messageList.h"
};
#undef messagemacro

#define formatmacro(e,t,v,s) s,
int psMessageFormatLengths[PS_FORMAT_COUNT] = {
#include "messages/MsgFormatList.h"
};
#undef formatmacro

//other name lookups
 char *batteryStateNames[] = BATTERY_STATUS_NAMES;

#define EVENT(e, n) n,
 char *eventNames[EVENT_COUNT] = {
#include "messages/NotificationEventsList.h"
 };
#undef EVENT

#define CONDITION(e, n) n,
 char *conditionNames[CONDITION_COUNT] = {
#include "messages/NotificationConditionsList.h"
 };
#undef CONDITION

 char *arbStateNames[ARB_STATE_COUNT] = ARB_STATE_NAMES;
