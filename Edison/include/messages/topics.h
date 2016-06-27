/* 
 * File:   Topics.h
 * Author: martin
 *
 * Created on June 14, 2014, 9:19 PM
 */

#ifndef TOPICS_H
#define	TOPICS_H

//--------------------- Bug Topics

//Subscribable Topics

typedef enum {
    PS_UNDEFINED_TOPIC,
    LOG_TOPIC,              //SysLog
    ANNOUNCEMENTS_TOPIC,    //Common channel
	RESPONSE_TOPIC,
    CONFIG_TOPIC,           //To send config data to APP
	TICK_1S_TOPIC,
    RAW_NAV_TOPIC,			//inputs to navigator
	NAV_TOPIC,				//output from navigator
	PILOT_TOPIC,			//inputs to autopilot
    DANCE_TOPIC,			//inputs to dancer
	GRIPPER_TOPIC,			//inputs to gripper
    ARBOTIX_TOPIC,			//inputs to Arbotix
	ARB_REPORT_TOPIC,		//reports from Arbotix
    ACTION_TOPIC,   		//commands to Behavior Tree
    REPORT_TOPIC,			//reports to Behavior Tree
	SYS_ACTION_TOPIC,		//commands from APP
    SYS_REPORT_TOPIC,       //reports to APP
	CONDITIONS_TOPIC,
	EVENTS_TOPIC,
    PS_TOPIC_COUNT
} psTopic_enum;

#define PS_TOPIC_NAMES { \
    "UNDEFINED",\
    "SYSLOG",\
    "ANNOUNCE",\
	"RESPONSE",\
    "CONFIG",\
	"TICK_1S"\
    "RAW_NAV",\
	"PROX",\
    "NAV",\
    "PILOT",\
    "DANCE",\
	"ARBOTIX"\
	"ARB_REPORT"\
    "ACTION",\
    "REPORT",\
	"SYS_ACTION",\
    "SYS_REPORT",\
	"CONDITIONS",\
	"EVENTS"\
    }

#endif	/* TOPICS_H */

