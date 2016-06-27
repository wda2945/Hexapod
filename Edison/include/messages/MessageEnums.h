/* 
 * File:   MessageEnums.h
 * Author: martin
 *
 * Created on March 26 2015
 */

#ifndef _MESSAGE_ENUMS_H
#define	_MESSAGE_ENUMS_H
 
// Header
 
//------------------------PubSub sources and destinations

typedef enum {
    SUBSYSTEM_UNKNOWN,
    OVERMIND,
    ROBO_APP,           //iOS App
    SUBSYSTEM_COUNT
} Subsystem_enum;

#define SUBSYSTEM_NAMES {"???","OVM","APP"}

//----------------------QOS

typedef enum {
    PS_QOS1,    //critical
    PS_QOS2,    //important
    PS_QOS3     //discardable
} psQOS_enum;

typedef enum {
	RESPONSE_INIT_ERRORS = 0x01,
	RESPONSE_FIRST_TIME  = 0x02,
	RESPONSE_AGENT_ONLINE= 0x04
} pingResponseFlags_enum;

//----------------------SYSTEM_STATE

//COMMANDS - SENT BY GOD

typedef enum {
        COMMAND_UNSPECIFIED,
        COMMAND_SYSTEM_OFF,     //minimum power
        COMMAND_SLEEP,     		//only PIC + comms
        COMMAND_REST,           //OVM + minimum power
        COMMAND_STANDBY,        //ready to move
        COMMAND_ACTIVE,         //System operational (free to move)
        COMMAND_COUNT} UserCommand_enum;

#define USER_COMMAND_NAMES {\
"Unspecified",\
"Off",\
"Sleep",\
"Rest",\
"Standby",\
"Active"}

//Current system POWER state - maintained by MCP

typedef enum {
	POWER_STATE_UNKNOWN,
	POWER_SHUTDOWN,         	//eg low battery shutdown
	POWER_SLEEPING,         	//minimum power state with comms
    POWER_WAKE_ON_EVENT,      	//wake if Prox or other criteria met
	POWER_OVM_STOPPING,       	//timer waiting for Linux to stop
	//above states OVM not running
	//----------------------------
	//below states OVM may be running
	POWER_OVM_STARTING,       // -> STANDBY when BBB up
    POWER_RESTING,            // BBB plus minimum power
	POWER_STANDBY,            // == system STANDBY
	POWER_ACTIVE,             //System operational (free to move)	== system ACTIVE
	POWER_STATE_COUNT
} PowerState_enum;

#define POWER_STATE_NAMES {\
"unknown",\
"Shutdown",\
"Sleeping",\
"Event",\
"Stopping",\
"Starting",\
"Resting",\
"Standby",\
"Active"}

//commands sent by Overmind

typedef enum {
    OVERMIND_UNSPECIFIED,       //used to flag that Overmind is running
    OVERMIND_POWEROFF,          //poweroff totally
    OVERMIND_SLEEP,             //keep MCP plus comms up
    OVERMIND_WAKE_ON_EVENT,      //wake on proximity, ...
    OVERMIND_REQUEST_RESTING,
    OVERMIND_REQUEST_ACTIVE,
    OVERMIND_ACTION_COUNT
} OvermindPowerCommand_enum;

#define OVERMIND_COMMAND_NAMES {\
"unspecified",\
"Poweroff",\
"Sleep",\
"Wake-on-event",\
"Request_Rest",\
"Request_Active"\
}

typedef enum {
    BATTERY_UNKNOWN_STATUS,
    BATTERY_NOMINAL_STATUS,
    BATTERY_LOW_STATUS,
    BATTERY_CRITICAL_STATUS,
    BATTERY_SHUTDOWN_STATUS,
    BATTERY_STATUS_COUNT
} BatteryStatus_enum;

#define BATTERY_STATUS_NAMES {\
"unknown",\
"nominal",\
"low",\
"critical",\
"shutdown"\
}

typedef enum {BEHAVIOR_INVALID, BEHAVIOR_ACTIVE, BEHAVIOR_FAIL, BEHAVIOR_RUNNING, BEHAVIOR_SUCCESS, BEHAVIOR_STATUS_COUNT} BehaviorStatus_enum;
#define BEHAVIOR_STATUS_NAMES {"invalid", "active", "fail", "running", "success"}

//---------------------------------------------------------------------

//actual Arbotix state, HEXBUG only
//published in ARB_STATE messages
typedef enum {
	//reported by ArbotixM
	ARB_RELAXED,        //torque off										== BUG_AX_ON
	ARB_LOWVOLTAGE,     //low volts shutdown
	ARB_TORQUED,        //just powered on, in sitting position, torque on	== BUG_AX_ON
	ARB_READY,          //standing up, stopped and ready 					== BUG_STANDBY
	ARB_WALKING,        //walking						 					== BUG_ACTIVE
	ARB_POSING,         //posing leg control
	ARB_STOPPING,       //halt sent
	ARB_SITTING,        //in process of sitting down
	ARB_STANDING,       //in process of standing
	//controller error states
	ARB_STATE_UNKNOWN,  //start up
	ARB_OFFLINE,        //not powered
	ARB_TIMEOUT,        //no response to command
	ARB_ERROR,          //error reported
	ARB_STATE_COUNT
} ArbotixState_enum;

#define ARB_STATE_NAMES {\
"relaxed",\
"lowvolts",\
"torqued",\
"ready",\
"walking",\
"posing",\
"stopping",\
"sitting",\
"standing",\
"unknown",\
"offline",\
"timeout",\
"error"}

//----------------------Proximity Detector Report Payload
typedef enum  {
	PROX_FRONT,
	PROX_FRONT_RIGHT,
	PROX_RIGHT,
	PROX_REAR_RIGHT,
	PROX_REAR,
	PROX_REAR_LEFT,
	PROX_LEFT,
	PROX_FRONT_LEFT,
	PROX_SECTORS}
psProxDirection_enum;
#define PROX_DIRECTION_NAMES {"front","front_right","right","rear_right","rear","rear_left","left","front_left"}

typedef enum { PROX_OFFLINE, PROX_ACTIVE, PROX_ERRORS, PROX_PASSIVE, PROX_FAR, PROX_CLOSE, PROX_CONTACT, PROX_STATUS_COUNT
} ProxStatus_enum;
#define PROX_STATUS_NAMES {"offline", "active", "errors", "passive", "far", "close", "contact"}

//----------------------MOTOR command

enum MotorFlags_enum {
    ADJUSTMENT                      = 0x0001,
    ENABLE_FRONT_CONTACT_ABORT      = 0x0002,
    ENABLE_REAR_CONTACT_ABORT       = 0x0004,
    ENABLE_SYSTEM_ABORT             = 0x0008,       //abort on critical battery, etc.
    //0x10 to 0x80 reserved for Motor message
    //the following are for the autopilot
    ENABLE_FRONT_CLOSE_ABORT        = 0x0100,
    ENABLE_REAR_CLOSE_ABORT        	= 0x0200,
    ENABLE_FRONT_FAR_ABORT       	= 0x0400,
    ENABLE_WAYPOINT_STOP        	= 0x0800,
    ENABLE_LOSTFIX_ABORT			= 0x1000
};


#endif
