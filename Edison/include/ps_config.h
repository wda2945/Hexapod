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


#include "ps_api/ps_types.h"
#define SOURCE SRC_HEXAPOD
#define SOURCE_NAME "Hexapod"

//pubsub parameters
#include "messages.h"
#include "pubsub/pubsub_header.h"
#include "transport/transport_header.h"
#include "packet/packet_header.h"

//pubsub parameters
#define  PS_MAX_MESSAGE  (sizeof(psMessage_t))

#define  PS_MAX_PACKET  (sizeof(psMessage_t) + sizeof(ps_pubsub_header_t)\
                                + sizeof(ps_transport_header_t) + sizeof(ps_packet_header_t))
#define PS_MAX_TOPIC_LIST 10

//registry
#define REGISTRY_DOMAIN_LENGTH        20
#define REGISTRY_NAME_LENGTH          20
#define REGISTRY_TEXT_LENGTH          40

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
        
#ifdef __cplusplus

//mutex primitives
#include <mutex>

#define DEFINE_MUTEX(M) std::mutex M;
#define LOCK_MUTEX(M)	M.lock();
#define UNLOCK_MUTEX(M)	M.unlock();

#endif	// __cplusplus

#endif /* ps_config_h */
