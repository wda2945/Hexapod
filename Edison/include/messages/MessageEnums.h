/* 
 * File:   MessageEnums.h
 * Author: martin
 *
 * Created on March 26 2015
 */

#ifndef _MESSAGE_ENUMS_H
#define	_MESSAGE_ENUMS_H


typedef enum {
	RESPONSE_INIT_ERRORS = 0x01,
	RESPONSE_FIRST_TIME  = 0x02,
	RESPONSE_AGENT_ONLINE= 0x04
} pingResponseFlags_enum;

//----------------------SYSTEM_STATE

typedef enum {BEHAVIOR_INVALID, BEHAVIOR_ACTIVE, BEHAVIOR_FAIL, BEHAVIOR_RUNNING, BEHAVIOR_SUCCESS, BEHAVIOR_STATUS_COUNT} BehaviorStatus_enum;
#define BEHAVIOR_STATUS_NAMES {"invalid", "active", "fail", "running", "success"}

//---------------------------------------------------------------------



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

#endif
