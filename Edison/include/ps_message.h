//
//  ps_message.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//
//message handling macros

#ifndef ps_message_h
#define ps_message_h

#include "messages.h"

//convenience macro to initialize the message struct
#define psInitPublish(msg, msgType) { msg.messageType = msgType;}

#define psMessageLength(type) (psMessageFormatLengths[psMsgFormats[type]] + sizeof(ps_message_id_t))

//send a message to the broker
#define NewBrokerMessage(msg) ps_publish(psDefaultTopics[msg.messageType], &msg, psMessageLength(msg.messageType))
#define RouteMessage(msg) ps_publish(psDefaultTopics[msg.messageType], &msg, psMessageLength(msg.messageType))

#endif /* ps_types_h */
