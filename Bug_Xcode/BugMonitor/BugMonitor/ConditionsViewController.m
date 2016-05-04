//
//  ConditionsViewController.m
//  RoboMonitor
//
//  Created by Martin Lane-Smith on 1/13/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#import "ConditionsViewController.h"

@interface ConditionsViewController () {
    UITableView *tableView;
    NSMutableArray *notifications;
    NSArray *sortedList;
    
    NotificationMask_t conditions;
    NotificationMask_t valid;
    
    ConditionsList_enum myList;
}

@end

@implementation ConditionsViewController

- (ConditionsViewController*) init
{
    if (self = [super init])
    {
        self.view = tableView = [[UITableView alloc] init];
        tableView.dataSource = self;
        tableView.delegate = self;
       
#define CONDITION(e, n) n,
        char *conditionNames[]		= {
#include "messages/NotificationConditionsList.h"
        };
#undef CONDITION
        
#define CONDITION(e, n) n,
        char *proxConditionNames[]		= {
#include "messages/NotificationConditionsListProximity.h"
            NULL
        };
#undef CONDITION
        
#define CONDITION(e, n) n,
        char *errorConditionNames[]		= {
#include "messages/NotificationConditionsListErrors.h"
            NULL
        };
#undef CONDITION
        
#define CONDITION(e, n) n,
        char *statusConditionNames[]		= {
#include "messages/NotificationConditionsListStatus.h"
            NULL
        };
#undef CONDITION
        
        notifications = [NSMutableArray array];
        
        for (int i=1; i< CONDITION_COUNT; i++)
        {
            NSNumber *conditionNumber = [NSNumber numberWithInt:i];
            NSString *conditionName = [NSString stringWithFormat:@"%s", conditionNames[i]];
            
            char **nameList;
            
            switch(myList)
            {
                case CONDITIONS_ERRORS:
                    nameList = errorConditionNames;
                    break;
                case CONDITIONS_PROXIMITY:
                    nameList = proxConditionNames;
                    break;
                case CONDITIONS_STATUS:
                    nameList = statusConditionNames;
                    break;
            }
            while (*nameList)
            {
                if (strcmp(conditionNames[i], *nameList) == 0)
                {
                    NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
                                          conditionNumber, @"number",
                                          conditionName, @"name",
                                          nil];
                    [notifications addObject:dict];
                }
                nameList++;
            }
        }

    }
    return self;
}

- (ConditionsViewController*) initForList: (ConditionsList_enum) list
{
    myList = list;
    return [self init];
}

- (void) clearConditions
{
    conditions = valid = 0;
    [(UITableView*)self.view reloadData];
}
-(void) didConnect{
    conditions = valid = 0;
    [(UITableView*)self.view reloadData];
}

-(void) didDisconnect{}
-(void) didReceiveMsg: (PubSubMsg*) message
{
    switch(message.msg.header.messageType){
        case CONDITIONS:
        {
            NotificationMask_t C = message.msg.eventMaskPayload.value;
            NotificationMask_t V = message.msg.eventMaskPayload.valid;
            
            conditions |= (V & C);
            
            if ((V & ~C))
            {
                conditions &= ~(V & ~C);
            }
            
            valid |= V;
            
            [(UITableView*)self.view reloadData];
        }
            break;
        default:
            break;
    }

}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    return NO;
}
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Return the number of sections.
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return notifications.count;
}

- (UITableViewCell *)tableView:(UITableView *)tView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"xxw"];
    if (!cell)
    {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"xxw"];
    }
    
    NSDictionary *dict = [notifications objectAtIndex:indexPath.row];
    
    cell.textLabel.text = [dict objectForKey:@"name"];
    
    if (conditions & valid & NOTIFICATION_MASK([[dict objectForKey:@"number"] intValue]))
    {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    }
    else
    {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }
    
    return cell;
}

@end
