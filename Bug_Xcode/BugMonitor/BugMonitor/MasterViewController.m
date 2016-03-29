//
//  MasterViewController.m
//  Monitor
//
//  Created by Martin Lane-Smith on 4/12/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//

#import "MasterViewController.h"
#import "AppDelegate.h"
#include <sys/time.h>

@interface MasterViewController () {
    bool connected;
    int currentPage;
    float rssif;
}

- (UIImage*) getStatusImage: (NSString*) subsys;

@end

//pages of App - excluding subsystems
enum { SYSTEM_PAGE, CONDITIONS_PAGE, BEHAVIOR_PAGE, /* OPTIONS_PAGE, SETTINGS_PAGE, */ DATA_PAGE, RC_PAGE, LOG_PAGE, PAGE_COUNT};

static MasterViewController *me;

@implementation MasterViewController

+ (MasterViewController*) getMasterViewController
{
    return me;
}
- (void)awakeFromNib
{
    me = self;
    
    self.subsystems = [NSMutableDictionary dictionary];
    self.sourceCodes = [NSMutableDictionary dictionary];
    
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        self.clearsSelectionOnViewWillAppear = NO;
        self.preferredContentSize = CGSizeMake(320.0, 600.0);
        self.storyBoard = [UIStoryboard storyboardWithName:@"Main_iPad" bundle:nil];
    } else {
        self.storyBoard = [UIStoryboard storyboardWithName:@"Main_iPhone" bundle:nil];
    }

    self.behaviorViewController     = [[BehaviorViewController alloc] init];
    self.conditionsViewController   = [[ConditionsViewController alloc] init];
    self.dataController         = [[DataViewController alloc] init];
    self.logViewController      = [[LogViewController alloc] init];
    self.optionsController      = [[OptionsViewController alloc] init];
    self.rcController           = [_storyBoard instantiateViewControllerWithIdentifier:@"RC"];
    self.settingsController     = [[SettingsViewController alloc] init];
    self.systemViewController   = [[SystemViewController alloc] init];
    self.subsystemViewController = nil;
    
    self.viewControllers = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                            _behaviorViewController, @"Behavior",
                            _conditionsViewController, @"Conditions",
                            _dataController, @"Data",
                            _logViewController, @"SysLog",
                            _optionsController, @"Options",
                            _rcController,@"RC",
                            _settingsController, @"Settings",
                            _systemViewController, @"System",
                   nil];
    
    currentPage = SYSTEM_PAGE;
    
    self.connectionCaption = @"Disconnected";
    
    [super awakeFromNib];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        self.collectionController = [(AppDelegate*)[[UIApplication sharedApplication] delegate] collectionController];
    }
    else
    {
        self.collectionController = self;
    }
}

- (void) setConnectionCaption:(NSString *)connectionCaption
{
    _connectionCaption = connectionCaption;
    if (self.view) [(UITableView*) self.view reloadData];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


#pragma mark - Table View

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 4;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    switch(section){
        case 0:
            return 1;
            break;
        case 1:
            return PAGE_COUNT;
            break;
        case 2:
            if (_subsystemViewController) return 1;
            else return 0;
        case 3:
            if (currentPage == LOG_PAGE) return 1;
            else return 0;
            break;
        default:
            return 0;
            break;
    }
}

- (CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section
{
    return 20.0f;
}
- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
    return 20.0f;
}
- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 35.0f;
}
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell;
    switch(indexPath.section){
        case 0:
        {
            cell = [tableView dequeueReusableCellWithIdentifier:@"MasterCell"];
            if (!cell)
            {
                cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"MasterCell"];
            }
            AppDelegate* delegate = (AppDelegate* )[[UIApplication sharedApplication] delegate];
            
            switch(indexPath.row){
                case 0:
                    if (delegate.connected){
                        cell.textLabel.text = self.connectionCaption;
                        cell.imageView.image = CONNECTED;
                    }
                    else {
                        cell.textLabel.text = self.connectionCaption;
                        cell.imageView.image = DISCONNECTED;
                    }
                    break;
                default:
                    return nil;
                    break;
            }

        }
            break;
            
      case 1:       //panels
        {
            cell = [tableView dequeueReusableCellWithIdentifier:@"DetailCell"];
            if (!cell)
            {
                cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"DetailCell"];
            }
            
            switch (indexPath.row){
                case SYSTEM_PAGE:
                    cell.textLabel.text = @"System";
                    cell.imageView.image = [UIImage imageNamed:@"house.png"];
                    break;
                case LOG_PAGE:
                    cell.textLabel.text = @"Log Messages";
                    cell.imageView.image = [UIImage imageNamed:@"log.png"];
                    break;
                case RC_PAGE:
                    cell.textLabel.text = @"Direct Control";
                    cell.imageView.image = [UIImage imageNamed:@"rc.png"];
                    break;
                case DATA_PAGE:
                    cell.textLabel.text = @"Data";
                    cell.imageView.image = [UIImage imageNamed:@"info.png"];
                    break;
//                case OPTIONS_PAGE:
//                    cell.textLabel.text = @"Options";
//                    cell.imageView.image = [UIImage imageNamed:@"option.png"];
//                    break;
//                case SETTINGS_PAGE:
//                    cell.textLabel.text = @"Settings";
//                    cell.imageView.image = [UIImage imageNamed:@"setting.png"];
//                    break;
                case CONDITIONS_PAGE:
                    cell.textLabel.text = @"Conditions";
                    cell.imageView.image = [UIImage imageNamed:@"conditions.png"];
                    break;
                case BEHAVIOR_PAGE:
                    cell.textLabel.text = @"Behavior";
                    cell.imageView.image = [UIImage imageNamed:@"control.png"];
                    break;
               default:
                    cell = nil;
                    break;
            }
        }
            break;
            
        case 2:         //subsystem
            if (_subsystemViewController)
            {
                cell = [tableView dequeueReusableCellWithIdentifier:@"SubsystemCell"];
                if (!cell)
                {
                    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"SubsystemCell"];
                }
                NSString *ssName = [_subsystems.allKeys objectAtIndex:indexPath.row];
                cell.textLabel.text = ssName;
                cell.imageView.image = [UIImage imageNamed: @"bug-32.png"];
            }
            else return nil;
            break;
        case 3:
            cell = [tableView dequeueReusableCellWithIdentifier:@"ExtraBtnCell"];
            if (!cell)
            {
                cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"ExtraBtnCell"];
            }
                cell.textLabel.text = @"Clear Log";
                cell.imageView.image = [UIImage imageNamed:@"log.png"];
            break;
        default:
            break;
    }
    return cell;
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    return NO;
}

-(void) didReceiveMsg: (PubSubMsg*) message
{
    //do we have this one?
    SubsystemViewController *ss = self.subsystemViewController;
    if (!ss) {
        //new one
        if (self.collectionController == nil)
        {
            if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
                AppDelegate *delegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
                self.collectionController = delegate.collectionController;
            }
            else
            {
                self.collectionController = self;
            }
        }
        
        self.subsystemViewController = ss = [[SubsystemViewController alloc] initWithMessage: message];
        
        [_sourceCodes setObject:ss forKey:[NSNumber numberWithInt: message.msg.header.source]];
        [_subsystems  setObject:ss forKey:ss.name];
        
        [ss addListener: self ];
        [self.viewControllers setObject:ss forKey:ss.name];
        
        [_collectionController addSubsystem: ss];
        [(UITableView*) self.view reloadData];
    }
    
    [_viewControllers.allValues makeObjectsPerformSelector:@selector(didReceiveMsg:) withObject:message];
}
-(void) connection: (bool) conn
{
    connected = conn;
    [(UITableView*) self.view reloadData];
}
- (void) changeSubsystemName: (SubsystemViewController*) ss oldName: (NSString*) n
{
    [self.subsystems removeObjectForKey:n];
    [self.subsystems  setObject:ss forKey:ss.name];
    [self.viewControllers removeObjectForKey:n];
    [self.viewControllers setObject:ss forKey:ss.name];
    
    [(UITableView*)self.view reloadData];
}

- (void) addSubsystem: (SubsystemStatus*) sub
{
    
}
- (UIImage*) getStatusImage: (NSString*) subsys
{
    SubsystemViewController *ss = self.subsystemViewController;
    
    if (ss && ss.online){
        return [UIImage imageNamed:@"online.png"];
    }
    else
    {
        return [UIImage imageNamed:@"offline.png"];
    }
}

- (NSIndexPath *)tableView:(UITableView *)tableView willSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (self.collectionController == nil)
    {
        if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
            AppDelegate *delegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
            self.collectionController = delegate.collectionController;
        }
        else
        {
            self.collectionController = self;
        }
    }
    switch(indexPath.section){
        case 0:
            break;
        case 1:
        {
            currentPage = (int) indexPath.row;
            [(UITableView*) self.view reloadData];
            switch (indexPath.row){
                case SYSTEM_PAGE:
                    if ([_collectionController presentView: @"System"]) return indexPath;
                    break;
                case LOG_PAGE:
                    if ([_collectionController presentView: @"SysLog"]) return indexPath;
                    break;
                case DATA_PAGE:
                    if ([_collectionController presentView: @"Data"]) return indexPath;
                    break;
//                case OPTIONS_PAGE:
//                    if ([_collectionController presentView: @"Options"]) return indexPath;
//                    break;
//                case SETTINGS_PAGE:
//                    if ([_collectionController presentView: @"Settings"]) return indexPath;
//                    break;
                case RC_PAGE:
                    if ([_collectionController presentView: @"RC"]) return indexPath;
                    break;
                case CONDITIONS_PAGE:
                    if ([_collectionController presentView: @"Conditions"]) return indexPath;
                    break;
                case BEHAVIOR_PAGE:
                    if ([_collectionController presentView: @"Behavior"]) return indexPath;
                    break;
                default:
                    break;
            }

        }
            break;
        case 2:
        {
            currentPage = PAGE_COUNT;
            [(UITableView*) self.view reloadData];
            if ([_collectionController presentView:  @"OVM"]) return indexPath;
        }
            break;
        case 3:
            [LogViewController ClearLog];
            break;
        default:
            break;
    }
    return nil;
}
- (bool) presentView: (NSString*) name{
    UIViewController *controller = [_viewControllers objectForKey:name];
    controller.title = name;
    if (controller) {
        [self.navigationController pushViewController:controller animated:YES];
        return YES;
    }
    return NO;
}
- (void) statusChange: (SubsystemStatus*) ss{
    [(UITableView*)self.view reloadData];
}
@end
