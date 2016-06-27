//
//  ps_config.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

//this project-specific config file configures plumbing
//it must be in the include path

#ifndef ps_config_h
#define ps_config_h

//pubsub parameters
#define PS_DEFAULT_MAX_PACKET 	256
#define PS_MAX_TOPIC_LIST 		10

//system logging parameters
#define PS_SOURCE_LENGTH 		5
#define PS_MAX_LOG_TEXT 		100
#define SYSLOG_LEVEL 			LOG_ALL
#define SYSLOG_QUEUE_LENGTH		100
#define LOGFILE_FOLDER			"/home/root/logfiles"

//plumbing debug and error reporting
#define PS_DEBUG(...) {char tmp[PS_MAX_LOG_TEXT];\
					snprintf(tmp,PS_MAX_LOG_TEXT,__VA_ARGS__);\
					tmp[PS_MAX_LOG_TEXT-1] = 0;\
					print_debug_message(tmp);}

#define PS_ERROR(...)  {char tmp[PS_MAX_LOG_TEXT];\
					snprintf(tmp,PS_MAX_LOG_TEXT,__VA_ARGS__);\
					tmp[PS_MAX_LOG_TEXT-1] = 0;\
					print_debug_message(tmp);\
					print_debug_message_to_file(stderr, tmp);}
        
#endif /* ps_config_h */
