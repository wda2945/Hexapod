//
//  PubSubMsg.m
//  FIDO
//
//  Created by Martin Lane-Smith on 12/5/13.
//  Copyright (c) 2013 Martin Lane-Smith. All rights reserved.
//

#import "PubSubMsg.h"
#import "PubSubData.h"
#import "AppDelegate.h"

#import "Helpers.h"

#define messagemacro(m,q,t,f,l) f,
int psMsgFormats[PS_MSG_COUNT] = {
#include "messageList.h"
};
#undef messagemacro

#define messagemacro(m,q,t,f,l) l,
char *psLongMsgNames[PS_MSG_COUNT] = {
#include "messageList.h"
};
#undef messagemacro

#define formatmacro(e,t,v,s) s,
int psMessageFormatLengths[PS_FORMAT_COUNT] = {
#include "MsgFormatList.h"
};
#undef formatmacro

@interface PubSubMsg (){
    char encodedMessage[100];
    int encodedLen;
    size_t bytesToSend;
    char *next;
}

@end

@implementation PubSubMsg

+ (bool) sendMessage:(psMessage_t *) msg {
    PubSubMsg *message = [[PubSubMsg alloc] initWithMsg: msg];
    if (message) {
        return [message sendMessage];
    }
    else{
        return NO;
    }
}

+ (bool) sendSimpleMessage: (int) msgType {
    psMessage_t message;
    NSAssert1(msgType < PS_MSG_COUNT,@"Bad Message # %i", msgType);
    NSAssert1(psMsgFormats[msgType] == PS_NO_PAYLOAD, @"%s needs payload", psLongMsgNames[msgType]);
    
    message.header.messageType = msgType;
    
    return [PubSubMsg sendMessage: &message];
}

- (PubSubMsg*) initWithMsg: (psMessage_t *) msg {
    self = [super init];
    
    memcpy(&_msg, msg, sizeof(psMessage_t));
  
    return self;
}

- (bool) sendMessage
{
    AdjustMessageLength(&_msg);
    
    AppDelegate* delegate = (AppDelegate* )[[UIApplication sharedApplication] delegate];
    
    [delegate sendMessage: self];
    
    return true;
}

- (void) copyMessage: (psMessage_t *) message
{
    memcpy(message, &_msg, sizeof(psMessage_t));
}

@end
