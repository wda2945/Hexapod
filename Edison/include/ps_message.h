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
#define psInitPublish(msg, msgType) {\
    msg.header.messageType = msgType;\
    msg.header.messageTopic = psDefaultTopics[msgType];}

#define psMessageLength(type) (psMessageFormatLengths[psMsgFormats[type]] + sizeof(psMessageHeader_t))

//send a message to the broker
#define NewBrokerMessage(msg) ps_publish(msg.header.messageTopic, &msg, psMessageLength(msg.header.messageType))
#define RouteMessage(msg) ps_publish(msg.header.messageTopic, &msg, psMessageLength(msg.header.messageType))

#endif /* ps_types_h */
