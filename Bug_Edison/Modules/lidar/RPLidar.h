/*
 * RoboPeak RPLIDAR Driver for Arduino
 * RoboPeak.com
 * 
 * Copyright (c) 2014, RoboPeak 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include "RPLidar.h"
#include "rptypes.h"
#include "rplidar_cmd.h"

struct RPLidarMeasurement
{
    float distance;
    float angle;
    uint8_t quality;
    bool  startBit;
};
	//send reset
	u_result sendRPLidarReset();

    // ask the RPLIDAR for its health info
    u_result getRPLidarHealth(rplidar_response_device_health_t *healthinfo);
    
    // ask the RPLIDAR for its device info like the serial number
    u_result getRPLidarDeviceInfo(rplidar_response_device_info_t *info);

    // stop the measurement operation
    u_result stopRPLidar();

    // start the measurement operation
    u_result startRPLidarScan(bool force);

    // wait for one sample point to arrive
    u_result waitRPLidarPoint();
    
    // retrieve currently received sample point
    struct RPLidarMeasurement *getRPLidarCurrentPoint();

