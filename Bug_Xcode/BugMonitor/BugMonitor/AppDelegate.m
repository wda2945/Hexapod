//
//  AppDelegate.m
//  BugMonitor
//
//  Created by Martin Lane-Smith on 4/12/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//

#import "AppDelegate.h"
#import "CollectionViewController.h"
#import "MasterViewController.h"

#import "helpers.h"
#import "pubsubparser.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT_NUMBER 50000
#define PING_INTERVAL 10.0f

@interface AppDelegate () {
    dispatch_queue_t recvQueue;         //GCD Queue for recv
    dispatch_queue_t sendQueue;         //GCD Queue for send
    
    NSTimer *pingTimer;
    
    uint8_t msgSequence;
    uint8_t checksum;
    int errorreply;
}

@end

@implementation AppDelegate

int sendSocket;                   //FD for server write
int recvSocket;                   //FD for server read
bool connected;                   //sockets valid

typedef union {
    uint8_t bytes[4];
    in_addr_t address;
} IPaddress_t;

- (bool) connected
{
    return connected;
}

void SIGPIPEhandler(int sig);

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    connected = false;
    msgSequence = 0;
    
    // Override point for customization after application launch.
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        self.splitViewController = (UISplitViewController *)self.window.rootViewController;
        self.collectionController  = [_splitViewController.viewControllers lastObject];
        _splitViewController.delegate = _collectionController;
    }
    
    //set SIG handler
    if (signal(SIGPIPE, SIGPIPEhandler) == SIG_ERR)
    {
        NSLog(@"SIGPIPE err: %s", strerror(errno));
    }
    else
    {
        NSLog(@"SIGPIPE handler set");
    }
    
    //set up async read & write via GCD
    recvQueue = dispatch_queue_create ( "RecvQueue", NULL);
    sendQueue = dispatch_queue_create ( "SendQueue", NULL);
    
    NSLog(@"Socket Ready");
    
    pingTimer = [NSTimer scheduledTimerWithTimeInterval:PING_INTERVAL
                                                 target:self
                                               selector:@selector(ping:)
                                               userInfo:nil
                                                repeats:YES];
    [self ping: nil];
    return YES;
}

//convenience function
- (void) writeToSocket: (uint8_t) c
{
    size_t reply = send(sendSocket, &c, 1, 0);
    if (reply == 1)
    {
        checksum += c;
    }
    else
    {
        errorreply = -1;
    }
}

//sends a message to the robot
//runs on sendQueue thread
- (void) sendToServer: (PubSubMsg *)message
{
    psMessage_t txMessage;
    [message copyMessage: &txMessage];
    uint8_t size;
    uint8_t *buffer;
    errorreply = 0;
    
    if (connected)
    {
        //sending binary
        //send STX
        [self writeToSocket:  STX_CHAR];
        checksum = 0; //checksum starts from here
        //send header
        [self writeToSocket:  txMessage.header.length];
        [self writeToSocket:  ~txMessage.header.length];
        [self writeToSocket:  msgSequence++];
        [self writeToSocket:  txMessage.header.source];
        [self writeToSocket:  txMessage.header.messageType];
        //send payload
        buffer = (uint8_t *) &txMessage.packet;
        size = txMessage.header.length;
        
        if (size > sizeof(psMessage_t) - SIZEOF_HEADER)
        {
            size = txMessage.header.length = sizeof(psMessage_t) - SIZEOF_HEADER;
        }
        
        while (size) {
            [self writeToSocket:  *buffer];
            buffer++;
            size--;
        }
        //write checksum
        [self writeToSocket:  (checksum & 0xff)];
        
        if (errorreply < 0)
        {
            NSLog(@"send error: %s\n", strerror(errno));
            connected = false;
        }
        else
        {
            NSLog(@"Message sent: %s\n", psLongMsgNames[txMessage.header.messageType]);
        }
    }
}

//called to send a message
//transfers to sendQueue thread
- (void) sendMessage: (PubSubMsg *)message
{
    if (connected)
    {
        //dispatch msg to sendQueue
        dispatch_async(sendQueue, ^{
            [self sendToServer: message];
        });
    }
}

//called on readQueue to service messages from the robot
//exits when the connection fails
- (void) readFromServer
{
    uint8_t readByte;
    int result;
    
    psMessage_t rxMessage;
    status_t parseStatus;
    
    parseStatus.noCRC       = 0; ///< Do not expect a CRC, if > 0
    parseStatus.noSeq       = 0; ///< Do not check seq #s, if > 0
    parseStatus.noLength2   = 0; ///< Do not check for duplicate length, if > 0
    parseStatus.noTopic     = 1; ///< Do not check for topic ID, if > 0
    ResetParseStatus(&parseStatus);
    
    while (connected && read(recvSocket, &readByte, 1) == 1)
    {
        result = ParseNextCharacter(readByte, &rxMessage, &parseStatus);
        
        if (result)
        {
            PubSubMsg *psMessage = [[PubSubMsg alloc] initWithMsg: &rxMessage];
            
            if (psMessage)
            {
                NSLog(@"Message recv: %s", psLongMsgNames[rxMessage.header.messageType]);
                
                //dispatch msg to main queue
                dispatch_async(dispatch_get_main_queue(), ^{
                    [[MasterViewController getMasterViewController] didReceiveMsg: psMessage];
                });
            }
            else
            {
                NSLog(@"Bad message received");
            }
        }
    }
    //read fail or disconnect
    NSLog(@"ReadFromServer exit: %s", strerror(errno));
}

- (void) setConnectedCaption: (NSString *) caption connected: (bool) c
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [MasterViewController getMasterViewController].connectionCaption = caption;
    });
}

typedef enum {CONNECT_ERROR, LOST_CONNECTION} ServerConnectResult_enum;

- (ServerConnectResult_enum) connectTo: (IPaddress_t) ipAddress
{
    struct sockaddr_in serverSockAddress;
    
    NSString *IP = [NSString stringWithFormat: @"%i.%i.%i.%i", ipAddress.bytes[0], ipAddress.bytes[1], ipAddress.bytes[2], ipAddress.bytes[3]];
    
    [self setConnectedCaption: [NSString stringWithFormat:@"Robot @ %@", IP] connected: NO];
    NSLog(@"Robot @ %@", IP);
    
    memset(&serverSockAddress, 0, sizeof(serverSockAddress));
    serverSockAddress.sin_len = sizeof(serverSockAddress);
    serverSockAddress.sin_family = AF_INET;
    serverSockAddress.sin_port = htons(PORT_NUMBER);
    serverSockAddress.sin_addr.s_addr = ipAddress.address;
    
    //create socket
    recvSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    if (recvSocket == -1)
    {
        NSLog(@"socket() error: %s", strerror(errno));
        [self setConnectedCaption: @"socket() error" connected: NO];
    }
    else
    {
        NSLog(@"Socket Created");
        
        //bind local socket address
        struct sockaddr_in inet_address;
        memset(&inet_address, 0, sizeof(inet_address));
        inet_address.sin_len = sizeof(inet_address);
        inet_address.sin_family = AF_INET;
        inet_address.sin_port = 0; //htons(PORT_NUMBER);
        inet_address.sin_addr.s_addr = INADDR_ANY;
        
        if (bind(recvSocket, (struct sockaddr*) &inet_address, sizeof(inet_address)) == -1)
        {
            NSLog(@"bind() error: %s", strerror(errno));
            [self setConnectedCaption: @"bind() error" connected: NO];
        }
        else
        {
            NSLog(@"Socket Bound");
            
            if (connect(recvSocket, (const struct sockaddr*) &serverSockAddress, sizeof(serverSockAddress)) < 0)
            {
                NSLog(@"Edison connect() err: %s", strerror(errno));
                [self setConnectedCaption: @"connect() error" connected: NO];
                close(recvSocket);
            }
            else
            {
                [self setConnectedCaption: [NSString stringWithFormat: @"Connected to %@", IP] connected: YES];
                //dup a socket for the send thread
                sendSocket = dup(recvSocket);
                
                connected = true;
                
                //send a ping right away
                dispatch_async(dispatch_get_main_queue(), ^{
                    [self ping:nil];
                });
                
                //read from server loops until the pipe fails
                [self readFromServer];
                
                close(recvSocket);
                
                //use GCD to process close() on send queue - between messages
                dispatch_sync(sendQueue, ^{
                    close(sendSocket);
                    connected = false;
                });
                [self setConnectedCaption: @"Connection lost" connected: NO];
                return LOST_CONNECTION;
            }
        }
        
        
    }
    return CONNECT_ERROR;
}
//called by ping timer periodically to try to connect to the robot using the readQueue thread
//if successful, it continues to receive message
//if/when unsuccessful, it exits, to be called again at the next timer fire
- (void) connectToServer
{
    struct hostent *server;
    IPaddress_t ipAddress;
    
    //First try Bonjour name
    
    [self setConnectedCaption: @"Searching..." connected: NO];
    
    server = gethostbyname2(SERVER_NAME, AF_INET);
    if (server != NULL)
    {
        memcpy(ipAddress.bytes, server->h_addr, 4);
        if ([self connectTo: ipAddress] == LOST_CONNECTION)
        {
            return;
        }
    }
    
    [self setConnectedCaption: @"Searching..." connected: NO];
    
    //then try an alternative
    ipAddress.bytes[0] = 192;
    ipAddress.bytes[1] = 168;
    ipAddress.bytes[2] = 1;
    ipAddress.bytes[3] = IP_ADDRESS;
    
    [self connectTo: ipAddress];
    
}

-(void) ping:(NSTimer*) timer{
    if (connected)
    {
        psMessage_t msg;
        msg.header.messageType = PING_MSG;
        //      msg.bytePayload.value = _systemViewController.systemStateCommand;
        [PubSubMsg sendMessage:&msg];
        gettimeofday(&_pingTime, NULL);
        NSLog(@"Ping");
    }
    else
    {
        dispatch_async(recvQueue, ^{
            [self connectToServer];
        });
    }
}

void SIGPIPEhandler(int sig)
{
    connected = false;
    close(recvSocket);
    NSLog(@"SIGPIPE");
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end
