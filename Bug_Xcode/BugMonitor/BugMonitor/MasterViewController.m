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
enum { SYSTEM_PAGE, CONDITIONS_PAGE1, CONDITIONS_PAGE2, CONDITIONS_PAGE3, BEHAVIOR_PAGE, RC_PAGE, LOG_PAGE, PAGE_COUNT};

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
    self.conditionsViewController1  = [[ConditionsViewController alloc] initForList: CONDITIONS_STATUS];
    self.conditionsViewController2  = [[ConditionsViewController alloc] initForList: CONDITIONS_PROXIMITY];
    self.conditionsViewController3  = [[ConditionsViewController alloc] initForList: CONDITIONS_ERRORS];
    self.logViewController          = [[LogViewController alloc] init];
    self.rcController               = [_storyBoard instantiateViewControllerWithIdentifier:@"RC"];
    self.systemViewController       = [[SystemViewController alloc] init];
    self.subsystemViewController    = [[SubsystemViewController alloc] init];
    
    [_sourceCodes setObject:_subsystemViewController forKey:[NSNumber numberWithInt: OVERMIND]];
    [_subsystems  setObject:_subsystemViewController forKey:_subsystemViewController.name];
    
    [_subsystemViewController addListener: self ];
    
    self.viewControllers = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                            _behaviorViewController, @"Behavior",
                            _conditionsViewController1, @"Conditions1",
                            _conditionsViewController2, @"Conditions2",
                            _conditionsViewController3, @"Conditions3",
                            _logViewController, @"SysLog",
                            _rcController,@"RC",
                            _systemViewController, @"System",
                            _subsystemViewController, _subsystemViewController.name,
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
                case CONDITIONS_PAGE1:
                    cell.textLabel.text = @"Conditions: Status";
                    cell.imageView.image = [UIImage imageNamed:@"conditions.png"];
                    break;
                case CONDITIONS_PAGE2:
                    cell.textLabel.text = @"Conditions: Proximity";
                    cell.imageView.image = [UIImage imageNamed:@"conditions.png"];
                    break;
                case CONDITIONS_PAGE3:
                    cell.textLabel.text = @"Conditions: Errors";
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
                cell.textLabel.text = @"Hexapod";
                cell.imageView.image = [UIImage imageNamed: @"bug-32.png"];
                
                if (_subsystemViewController.configured)
                {
                    cell.accessoryType = UITableViewCellAccessoryCheckmark;
                }
                else
                {
                    cell.accessoryType = UITableViewCellAccessoryNone;
                }
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
                case RC_PAGE:
                    if ([_collectionController presentView: @"RC"]) return indexPath;
                    break;
                case CONDITIONS_PAGE1:
                    if ([_collectionController presentView: @"Conditions1"]) return indexPath;
                    break;
                case CONDITIONS_PAGE2:
                    if ([_collectionController presentView: @"Conditions2"]) return indexPath;
                    break;
                case CONDITIONS_PAGE3:
                    if ([_collectionController presentView: @"Conditions3"]) return indexPath;
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
