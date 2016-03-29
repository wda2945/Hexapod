/*
 * pubsub.h
 *
 *  message handling
 *
 *      Author: martin
 */

#ifndef PUBSUB_H
#define PUBSUB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "pubsubdata.h"
#include "pubsub/broker.h"
#include "pubsub/notifications.h"

extern bool initComplete;

//convenience macro to initialize the message struct
#define psInitPublish(msg, msgType) {\
    msg.header.messageType=msgType;\
        msg.header.length=psMessageFormatLengths[psMsgFormats[msgType]];}

//send a message to the broker
#define NewBrokerMessage(msg) CopyMessageToQ(&brokerQueue, msg)

//route message directly
void RouteMessage(psMessage_t *msg);

//options
#define optionmacro(name, var, min, max, def) extern int var;
#include "options.h"
#undef optionmacro

//Settings
#define settingmacro(name, var, min, max, def) extern float var;
#include "settings.h"
#undef settingmacro

#endif
