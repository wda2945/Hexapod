//
//  MessageDelegateProtocol.h
//
//  Created by Martin Lane-Smith on 1/1/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//

#ifndef MessageDelegate_h
#define MessageDelegate_h

#import "PubSubMsg.h"

@protocol MessageDelegate
@required
-(void) didReceiveMsg: (PubSubMsg*) message;
@optional
-(void) connection: (bool) connected;
@end

#endif
