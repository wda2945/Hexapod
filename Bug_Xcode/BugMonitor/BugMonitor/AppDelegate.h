//
//  AppDelegate.h
//  Monitor
//
//  Created by Martin Lane-Smith on 4/12/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "PubSubMsg.h"
#import "MessageDelegateProtocol.h"
#import "MasterViewController.h"
#import <CoreFoundation/CoreFoundation.h>

#define DISCONNECTED [UIImage imageNamed:@"offline.png"]
#define CONNECTED [UIImage imageNamed:@"online.png"]

#define SERVER_NAME "hexapod.local"
#define IP_ADDRESS	209

@interface AppDelegate : UIResponder <UIApplicationDelegate, NSNetServiceDelegate, NSNetServiceBrowserDelegate>

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) UISplitViewController *splitViewController;
@property (strong, nonatomic) CollectionViewController *collectionController;

@property (strong, nonatomic) NSMutableArray *msgQueue;

@property struct timeval pingTime;

- (void) sendMessage: (PubSubMsg*) msg;
- (bool) connected;

@end
