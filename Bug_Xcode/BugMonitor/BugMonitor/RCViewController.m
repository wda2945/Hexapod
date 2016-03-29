//
//  RCViewController.m
//  Monitor
//
//  Created by Martin Lane-Smith on 4/13/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//

#import "RCViewController.h"
#import "PubSubMsg.h"
#import "AppDelegate.h"

#define MOVE_INTERVAL 0.5f

@interface RCViewController () {
    CGPoint touchDown;
    
    NSTimer *moveTimer;
    float xMove, yMove, zRotation;
    float xLastMove, yLastMove, zLastRotation;
}

- (void) sendCommandX: (float) x andY: (float) y andRotateZ: (float) z;
- (void) moveMsg: (NSTimer*) timer;

- (void) rotate: (UIRotationGestureRecognizer*) gr;
- (void) slide: (UIPanGestureRecognizer*) gr;

@end

@implementation RCViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
        xMove = yMove = zRotation = xLastMove = yLastMove = zLastRotation = 0.0;
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    UIRotationGestureRecognizer *rotate = [[UIRotationGestureRecognizer alloc] initWithTarget:self action:@selector(rotate:)];
    
    [self.view addGestureRecognizer:rotate];
    
    UIPanGestureRecognizer *pan = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(slide:)];
   
    pan.minimumNumberOfTouches = 1;
    pan.maximumNumberOfTouches = 2;
    
    [self.view addGestureRecognizer:pan];
    
    moveTimer = [NSTimer scheduledTimerWithTimeInterval:MOVE_INTERVAL
                                                 target:self
                                               selector:@selector(moveMsg:)
                                               userInfo:nil
                                                repeats:YES];
}

- (void) rotate: (UIRotationGestureRecognizer*) gr
{
    switch (gr.state)
    {
        case UIGestureRecognizerStateEnded:
        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateFailed:
            [self sendCommandX: 0 andY: 0 andRotateZ: 0];
            break;
        case UIGestureRecognizerStateChanged:
        {
            [self sendCommandX: 0 andY: 0 andRotateZ: gr.rotation];
        }
            break;
        default:
            break;
    }
}
- (void) slide: (UIPanGestureRecognizer*) gr
{
    switch (gr.state)
    {
        case UIGestureRecognizerStateEnded:
        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateFailed:
            [self sendCommandX: 0 andY: 0 andRotateZ: 0];
            break;
        case UIGestureRecognizerStateChanged:
        {
            CGPoint pan = [gr translationInView:self.view];
            [self sendCommandX: pan.x andY: -pan.y andRotateZ: 0];
        }
            break;
        default:
            break;
    }
}

- (void) sendCommandX: (float) x andY: (float) y andRotateZ: (float) z
{
    xMove = x / 10;
    yMove = y / 10;
    zRotation = z * 10;
}
- (void) moveMsg: (NSTimer*) timer
{
    if (xMove == xLastMove && yMove == yLastMove && zRotation == zLastRotation) return;
    
    NSLog(@"Move %f, %f. turn %f", yMove, xMove, zRotation);
    
    if ([(AppDelegate*)[[UIApplication sharedApplication] delegate] connected])
    {
        psMessage_t msg;
        msg.header.messageType = MOVE;
        msg.threeFloatPayload.xSpeed = yMove;
        msg.threeFloatPayload.ySpeed = xMove;
        msg.threeFloatPayload.zRotateSpeed = zRotation;
        [PubSubMsg sendMessage:&msg];
        
        xLastMove = xMove;
        yLastMove = yMove;
        zLastRotation = zRotation;
    }
}

-(void) didReceiveMsg: (PubSubMsg*) message
{
    
}
- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
