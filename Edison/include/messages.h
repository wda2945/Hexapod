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
#include "ps.h"

//----------------------QOS

typedef enum {
    PS_QOS1,    //critical
    PS_QOS2,    //important
    PS_QOS3     //discardable
} psQOS_enum;

//---------------------Message Formats

#include "/Users/martin/Dropbox/RoboticsCodebase/RobotMonitor/rm_messages/rm_message_formats.h"
#include "Messages/MessageFormats.h"

//---------------------Message Topics

#define topicmacro(e, name) e,
typedef enum {
#include "/Users/martin/Dropbox/RoboticsCodebase/RobotMonitor/rm_messages/rm_topics_list.h"
#include "messages/topics_list.h"
    PS_TOPIC_COUNT
} ps_topic_id_enum;
#undef topicmacro

//---------------------Message codes enum

#define messagemacro(m,q,t,f,l) m,
typedef enum {
#include "/Users/martin/Dropbox/RoboticsCodebase/RobotMonitor/rm_messages/rm_message_list.h"
#include "messages/MessageList.h"
    PS_MSG_COUNT
} psMessageType_enum;
#undef messagemacro

//---------------------Message Formats enum

#define formatmacro(e,t,p,s) e,

typedef enum {
#include "/Users/martin/Dropbox/RoboticsCodebase/RobotMonitor/rm_messages/rm_message_format_list.h"
#include "Messages/MsgFormatList.h"
    PS_FORMAT_COUNT
} psMsgFormat_enum;
#undef formatmacro

//----------------------Complete message struct

//Generic struct for all messages

#define formatmacro(e,t,p,s) t p;
typedef struct {
#pragma pack(1)
    ps_message_id_t messageType;
    //Union option for each payload type
    union {
#include "/Users/martin/Dropbox/RoboticsCodebase/RobotMonitor/rm_messages/rm_message_format_list.h"
#include "Messages/MsgFormatList.h"
    };
} psMessage_t;
#pragma pack()
#undef formatmacro

//message lookup tables. Initialized in messages.c

extern const char *psTopicNames[PS_TOPIC_COUNT];

extern const int psMsgFormats[PS_MSG_COUNT];

extern const int psDefaultTopics[PS_MSG_COUNT];

extern const char *psLongMsgNames[PS_MSG_COUNT];

extern const psQOS_enum psQOS[PS_MSG_COUNT];

extern const int psMessageFormatLengths[PS_FORMAT_COUNT];

//options
#define optionmacro(name, var, min, max, def) extern int var;
#include "options.h"
#undef optionmacro

//Settings
#define settingmacro(name, var, min, max, def) extern float var;
#include "settings.h"
#undef settingmacro

#endif
