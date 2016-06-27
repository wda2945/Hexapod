/* 
 * File:   Messages.h
 * Author: martin
 *
 * Enums and structs of Messages
 *
 */

#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include <stdint.h>
#include "ps_api/ps_types.h"

//---------------------Message Enums

#include "Messages/MessageEnums.h"
#include "Messages/NotificationEnums.h"

//---------------------Message Formats

#include "Messages/MessageFormats.h"

//---------------------Message Topics enum

#include "Messages/Topics.h"

//---------------------Message codes enum

#define messagemacro(m,q,t,f,l) m,
typedef enum {
#include "Messages/MessageList.h"
    PS_MSG_COUNT
} psMessageType_enum;
#undef messagemacro

//---------------------Message Formats enum

#define formatmacro(e,t,p,s) e,

typedef enum {
#include "Messages/MsgFormatList.h"
    PS_FORMAT_COUNT
} psMsgFormat_enum;
#undef formatmacro

//----------------------Complete message struct

//Generic struct for all messages

#define formatmacro(e,t,p,s) t p;
#pragma pack(1)
typedef struct {
    psMessageHeader_t header;
    //Union option for each payload type
    union {
        uint8_t packet[1];
#include "Messages/MsgFormatList.h"
    };
} psMessage_t;
#pragma pack()
#undef formatmacro

//message lookup tables. Initialized in messages.c

extern char *subsystemNames[SUBSYSTEM_COUNT];

extern char *psTopicNames[PS_TOPIC_COUNT];

extern int psMsgFormats[PS_MSG_COUNT];

extern int psDefaultTopics[PS_MSG_COUNT];

extern char *psLongMsgNames[PS_MSG_COUNT];

extern psQOS_enum psQOS[PS_MSG_COUNT];

extern int psMessageFormatLengths[PS_FORMAT_COUNT];

//options
#define optionmacro(name, var, min, max, def) extern int var;
#include "options.h"
#undef optionmacro

//Settings
#define settingmacro(name, var, min, max, def) extern float var;
#include "settings.h"
#undef settingmacro

//other name lookups
extern char *batteryStateNames[BATTERY_STATUS_COUNT];
extern char *eventNames[EVENT_COUNT];
extern char *conditionNames[CONDITION_COUNT];
extern char *stateCommandNames[COMMAND_COUNT];
extern char *powerStateNames[POWER_STATE_COUNT];
extern char *ovmCommandNames[OVERMIND_ACTION_COUNT];
extern char *arbStateNames[ARB_STATE_COUNT];

#endif
