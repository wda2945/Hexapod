/*
 * arbotix.h
 *
 *      Author: martin
 */

#ifndef ARBOTIX_H
#define ARBOTIX_H

#include "behavior/behavior_enums.h"

//init arbotix task
int ArbotixInit();

//lua leaf callbacks codes
//Commands to the Hexapod control module
typedef enum {
	HEXAPOD_NULL,
	HEXAPOD_STAND,
	HEXAPOD_SIT,
	HEXAPOD_TURN_LEFT,
	HEXAPOD_TURN_RIGHT,
	HEXAPOD_TURN_LEFT_90,
	HEXAPOD_TURN_RIGHT_90,
	HEXAPOD_TURN_N,
	HEXAPOD_TURN_S,
	HEXAPOD_TURN_E,
	HEXAPOD_TURN_W,
	HEXAPOD_MOVE_FORWARD,
	HEXAPOD_MOVE_BACKWARD,
	HEXAPOD_MOVE_FORWARD_10,
	HEXAPOD_MOVE_BACKWARD_10,
	HEXAPOD_FAST_SPEED,
	HEXAPOD_MEDIUM_SPEED,
	HEXAPOD_SLOW_SPEED
} ArbAction_enum;

extern uint16_t movementAbortFlags;
enum AbortFlags_enum {
    ENABLE_FRONT_CLOSE_ABORT        = 0x01,
    ENABLE_REAR_CLOSE_ABORT        	= 0x02,
    ENABLE_FRONT_FAR_ABORT       	= 0x04,
    ENABLE_SYSTEM_ABORT             = 0x08       //abort on critical battery, etc.
};

//called from callbacks_arbotix.cpp
ActionResult_enum HexapodExecuteAction(ArbAction_enum _action);

//Hexapod state, mainly received from ArbotixM
typedef enum {
	//reported by ArbotixM
	ARBOTIX_RELAXED,        //torque off
	ARBOTIX_LOWVOLTAGE,     //low volts shutdown
	ARBOTIX_TORQUED,        //just powered on, in sitting position, torque on
	ARBOTIX_READY,          //standing up, stopped and ready
	ARBOTIX_WALKING,        //walking
	ARBOTIX_POSING,         //posing leg control
	ARBOTIX_STOPPING,       //halt sent
	ARBOTIX_SITTING,        //in process of sitting down
	ARBOTIX_STANDING,       //in process of standing
	//controller state
	ARBOTIX_WAITING,		//command sent, waiting for acknowledgment
	//controller error states
	ARBOTIX_STATE_UNKNOWN,  //start up
	ARBOTIX_OFFLINE,        //not powered
	ARBOTIX_TIMEOUT,        //no response to command
	ARBOTIX_ERROR,          //error reported
	ARBOTIX_STATE_COUNT
} ArbotixState_enum;

#define ARBOTIX_STATE_NAMES {\
"relaxed",\
"lowvolts",\
"torqued",\
"ready",\
"walking",\
"posing",\
"stopping",\
"sitting",\
"standing",\
"waiting",\
"unknown",\
"offline",\
"timeout",\
"error"}

//current state
extern ArbotixState_enum arbotixState;
//names
extern const char *arbotixStateNames[];

#define RANDOM_MOVE_SPREAD 20	//cm
#define RANDOM_TURN_SPREAD 30	//degrees

#endif
