/*
 * gripper.hpp
 *
 *      Author: martin
 */

#ifndef GRIPPER_H
#define GRIPPER_H

#include "behavior/behavior_enums.h"

int GripperInit();

#define CLOSED_PULSE_LENGTH 1300		//microseconds
#define OPEN_PULSE_LENGTH	2000

typedef enum {GRIPPER_SLOW, GRIPPER_FAST} GripperSpeed_enum;

extern GripperSpeed_enum gripperSpeed;

typedef enum {GRIPPER_MOVING, GRIPPER_CLOSED, GRIPPER_OPEN} GripperState_enum;

ActionResult_enum MoveGripper(GripperState_enum action);

#endif
