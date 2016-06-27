/*
 * behavior_enums.h
 *
 *  Created on: Jul 11, 2014
 *      Author: martin
 */

#ifndef BEHAVIOR_ENUMS_H_
#define BEHAVIOR_ENUMS_H_

#include <string>
#include "messages.h"

typedef enum {ACTION_SUCCESS, ACTION_FAIL, ACTION_RUNNING} ActionResult_enum;

extern Position_struct heelPosition;
extern bool	heelPositionValid;

extern std::string lastLuaCall;
extern std::string lastLuaCallFail;
extern std::string lastLuaCallReason;
extern std::string behaviorName;
extern BehaviorStatus_enum behaviorStatus;

typedef uint8_t ProxSectorMask_enum;
const ProxSectorMask_enum PROX_FRONT_MASK = 0x1;
const ProxSectorMask_enum PROX_FRONT_RIGHT_MASK = 0x2;
const ProxSectorMask_enum PROX_RIGHT_MASK = 0x4;
const ProxSectorMask_enum PROX_REAR_RIGHT_MASK = 0x8;
const ProxSectorMask_enum PROX_REAR_MASK = 0x10;
const ProxSectorMask_enum PROX_REAR_LEFT_MASK = 0x20;
const ProxSectorMask_enum PROX_LEFT_MASK = 0x40;
const ProxSectorMask_enum PROX_FRONT_LEFT_MASK = 0x80;

typedef uint8_t ProxStatusMask_enum;
const ProxStatusMask_enum PROX_OFFLINE_MASK = 0x1;
const ProxStatusMask_enum 	PROX_ACTIVE_MASK = 0x2;
const ProxStatusMask_enum 	PROX_ERRORS_MASK = 0x4;
const ProxStatusMask_enum 	PROX_FAR_MASK = 0x10;
const ProxStatusMask_enum 	PROX_CLOSE_MASK = 0x20;


#endif /* BEHAVIOR_ENUMS_H_ */
