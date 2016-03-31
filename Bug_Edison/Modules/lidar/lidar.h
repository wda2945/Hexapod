/*
 * lidar.h
 *
 *      Author: martin
 */

#ifndef LIDAR_HPP
#define LIDAR_HPP

#include "pubsubdata.h"
#include "behavior/behavior_enums.h"

extern int lidarFD;

int LidarInit();

void LidarProcessMessage(psMessage_t *msg);

bool proximityStatus(ProxSectorMask_enum sector, ProxStatusMask_enum proximity);

#endif
