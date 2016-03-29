/*
 * lidar.h
 *
 *      Author: martin
 */

#ifndef LIDAR_HPP
#define LIDAR_HPP

#include "pubsubdata.h"

extern int lidarFD;

int LidarInit();

void LidarProcessMessage(psMessage_t *msg);

bool proximityStatus(unsigned int sector, unsigned int proximity);

#endif
