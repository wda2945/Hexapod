/*
 ============================================================================
gd Name        : Broker.c
 Author      : Martin
 Version     :
 Copyright   : (c) 2013 Martin Lane-Smith
 Description : Linux PubSub broker
 ============================================================================
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "helpers.h"
#include "pubsubdata.h"
#include <softwareProfile.h>
#include "pubsub/pubsub.h"

#include "agent/agent.h"
#include "arbotix/arbotix.h"
#include "autopilot/autopilot.h"
#include "behavior/behavior.h"
#include "dancer/dancer.h"
#include "gripper/gripper.h"
#include "i2c_task/i2c_task.h"
#include "lidar/lidar.h"
#include "navigator/navigator.h"
#include "pubsub/responder.h"
#include "syslog/syslog.h"

//pass message to appropriate modules
void RouteMessage(psMessage_t *msg)
{
	if (!initComplete) return;

	AdjustMessageLength(msg);

	switch(psDefaultTopics[msg->header.messageType])
	{
	default:
		break;
	case LOG_TOPIC:              //SysLog
		SysLog_PROCESS_MESSAGE(msg);
		Agent_PROCESS_MESSAGE(msg);
		break;
	case ANNOUNCEMENTS_TOPIC:    //Common channel
		Behavior_PROCESS_MESSAGE(msg);
		Responder_PROCESS_MESSAGE(msg);
		break;
	case TICK_1S_TOPIC:
		Arbotix_PROCESS_MESSAGE(msg);
		Autopilot_PROCESS_MESSAGE(msg);
		Behavior_PROCESS_MESSAGE(msg);
		Dancer_PROCESS_MESSAGE(msg);
		Responder_PROCESS_MESSAGE(msg);
		break;

	case RAW_NAV_TOPIC:			//inputs to navigator
//		Agent_PROCESS_MESSAGE(msg);		//copy IMU to App
		Navigator_PROCESS_MESSAGE(msg);
		break;
	case NAV_TOPIC:				//output from navigator
		Autopilot_PROCESS_MESSAGE(msg);
		Behavior_PROCESS_MESSAGE(msg);
		break;
	case PILOT_TOPIC:			//inputs to autopilot
		Autopilot_PROCESS_MESSAGE(msg);
		break;
	case DANCE_TOPIC:			//inputs to dancer
		Dancer_PROCESS_MESSAGE(msg);
		break;
	case ARBOTIX_TOPIC:			//inputs to Arbotix
		Arbotix_PROCESS_MESSAGE(msg);
		break;
	case ARB_REPORT_TOPIC:		//reports from Arbotix
		Autopilot_PROCESS_MESSAGE(msg);
		Dancer_PROCESS_MESSAGE(msg);
		break;
	case REPORT_TOPIC:			//reports to Behavior Tree
	case SYS_ACTION_TOPIC:		//commands from APP
	case ACTION_TOPIC:
		Behavior_PROCESS_MESSAGE(msg);
		break;
	case SYS_REPORT_TOPIC:       //reports to APP
	case CONFIG_TOPIC:           //To send config data to APP
	case RESPONSE_TOPIC:
	case CONDITIONS_TOPIC:
		Agent_PROCESS_MESSAGE(msg);
		Behavior_PROCESS_MESSAGE(msg);
		break;
	case EVENTS_TOPIC:
//		Agent_PROCESS_MESSAGE(msg);
		Behavior_PROCESS_MESSAGE(msg);
		break;
	}
}

////options
//#define optionmacro(name, var, min, max, def) int var;
//#include "options.h"
//#undef optionmacro
//
////Settings
//#define settingmacro(name, var, min, max, def) float var;
//#include "settings.h"
//#undef settingmacro

