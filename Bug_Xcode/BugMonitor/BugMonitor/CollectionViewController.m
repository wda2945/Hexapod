//
//  CollectionViewController.m
//  Monitor
//
//  Created by Martin Lane-Smith on 4/13/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//

#import "CollectionViewController.h"
#import "MasterViewController.h"


@interface CollectionViewController () {
    
    UIViewController *currentController;
    
    CGRect startFrame;
}

@property (strong, nonatomic) UIPopoverController *masterPopoverController;
@end

@implementation CollectionViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    self.definesPresentationContext = YES;

    [self addChildViewController:[MasterViewController getMasterViewController].behaviorViewController];
    [self addChildViewController:[MasterViewController getMasterViewController].conditionsViewController1];
    [self addChildViewController:[MasterViewController getMasterViewController].conditionsViewController2];
    [self addChildViewController:[MasterViewController getMasterViewController].conditionsViewController3];
    [self addChildViewController:[MasterViewController getMasterViewController].logViewController];
    [self addChildViewController:[MasterViewController getMasterViewController].rcController];
    [self addChildViewController:[MasterViewController getMasterViewController].systemViewController];
    [self addChildViewController:[MasterViewController getMasterViewController].subsystemViewController];
    
    startFrame = self.view.bounds;
    startFrame.origin.x += startFrame.size.width;
    
    [MasterViewController getMasterViewController].systemViewController.view.frame = self.view.bounds;
    [self.view addSubview: [MasterViewController getMasterViewController].systemViewController.view];
    [[MasterViewController getMasterViewController].systemViewController didMoveToParentViewController:self];
    currentController = [MasterViewController getMasterViewController].systemViewController;
    
}
- (void) addSubsystem: (SubsystemViewController*) sub
{
    [self addChildViewController:sub];
}
- (bool) presentView: (NSString*) name
{
    UIViewController *controller = [[MasterViewController getMasterViewController].viewControllers objectForKey:name];
    if (controller && controller != currentController) {        
        
        controller.title = name;
        controller.view.frame = startFrame;
       
        [self transitionFromViewController: currentController toViewController: controller 
                                  duration: 0.25 options:0
                                animations:^{
                                    controller.view.frame = self.view.bounds;
                                }
                                completion:nil];
        currentController = controller;
        return YES;
    }
    else return NO;
}


-(void) didReceiveMsg: (PubSubMsg*) message
{
    [[MasterViewController getMasterViewController].viewControllers.allValues makeObjectsPerformSelector:@selector( didReceiveMsg:) withObject:message];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Split view

- (void)splitViewController:(UISplitViewController *)splitController willHideViewController:(UIViewController *)viewController withBarButtonItem:(UIBarButtonItem *)barButtonItem forPopoverController:(UIPopoverController *)popoverController
{
    barButtonItem.title = NSLocalizedString(@"Monitor", @"Monitor");
    [self.navigationItem setLeftBarButtonItem:barButtonItem animated:YES];
    self.masterPopoverController = popoverController;
}
- (BOOL)splitViewController:(UISplitViewController *)svc shouldHideViewController:(UIViewController *)vc inOrientation:(UIInterfaceOrientation)orientation
{
    if (vc == [MasterViewController getMasterViewController])
        return YES;
    else
        return NO;
}
- (void)splitViewController:(UISplitViewController *)splitController willShowViewController:(UIViewController *)viewController invalidatingBarButtonItem:(UIBarButtonItem *)barButtonItem
{
    // Called when the view is shown again in the split view, invalidating the button and popover controller.
    [self.navigationItem setLeftBarButtonItem:nil animated:YES];
    self.masterPopoverController = nil;
}


@end
