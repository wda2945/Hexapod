//
//  MasterViewController.h
//  Monitor
//
//  Created by Martin Lane-Smith on 4/12/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MessageDelegateProtocol.h"
#import "CollectionProtocol.h"

#import "CollectionViewController.h"
#import "BehaviorViewController.h"
#import "ConditionsViewController.h"
#import "LogViewController.h"
#import "RCViewController.h"
#import "SubsystemViewController.h"
#import "SystemViewController.h"

@interface MasterViewController : UITableViewController <MessageDelegate, CollectionController, StatusUpdate>

@property (strong, nonatomic) BehaviorViewController    *behaviorViewController;
@property (strong, nonatomic) ConditionsViewController  *conditionsViewController1;
@property (strong, nonatomic) ConditionsViewController  *conditionsViewController2;
@property (strong, nonatomic) ConditionsViewController  *conditionsViewController3;
@property (strong, nonatomic) LogViewController         *logViewController;
@property (strong, nonatomic) RCViewController          *rcController;
@property (strong, nonatomic) SubsystemViewController   *statusController;
@property (strong, nonatomic) SystemViewController      *systemViewController;
@property (strong, nonatomic) SubsystemViewController   *subsystemViewController;

@property (strong, nonatomic) NSMutableDictionary *viewControllers;

@property (strong, nonatomic) UIStoryboard *storyBoard;
@property (strong, nonatomic) UIViewController <CollectionController>  *collectionController;

@property (strong, nonatomic) NSMutableDictionary *sourceCodes;
@property (strong, nonatomic) NSMutableDictionary *subsystems;

@property (strong, nonatomic) NSString *connectionCaption;

+ (MasterViewController*) getMasterViewController;

- (void) changeSubsystemName: (SubsystemViewController*) obj oldName: (NSString*) n;

@end
