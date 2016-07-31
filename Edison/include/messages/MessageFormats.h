/* 
 * File:   MessageFormats.h
 * Author: martin
 *
 * Created on December 8, 2013, 10:53 AM
 */

#ifndef MESSAGEFORMATS_H
#define	MESSAGEFORMATS_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

#ifdef __XC32
#include "FreeRTOS.h"
#include "queue.h"
#else
typedef uint32_t TickType_t;
#endif

#define PS_NAME_LENGTH 30
#define PS_SHORT_NAME_LENGTH 16

//----------------------SYSTEM MANAGEMENT

//PS_TICK_PAYLOAD

//Data broadcast by 1S tick
typedef struct {
    uint8_t systemPowerState;		//PowerState_enum
} psTickPayload_t;

//---------------------- STATS -----------------------------------------------

//PS_COMMS_STATS_PAYLOAD

typedef struct {
    char destination[4];
    int messagesSent;
    int addressDiscarded;
    int congestionDiscarded;      //congestion
    int logMessagesDiscarded;
    int messagesReceived;
    int addressIgnored;        //wrong address
    int parseErrors;
} psCommsStatsPayload_t;

//----------------------NAVIGATION & SENSORS------------------------------------------
//----------------------RAW

//PS_IMU

typedef struct {
    int16_t magX, 	magY, 	magZ;
    int16_t accelX,	accelY,	accelZ;
    int16_t gyroX, 	gyroY, 	gyroZ;
    struct timeval timestamp;
} psImuPayload_t;

//PS_ODOMETRY

typedef struct {
    //movement since last message
    float xMovement;         	// cm
    float yMovement;			// cm
    float zRotation;
    //status
    uint8_t motorsRunning;
} psOdometryPayload_t;

//---------------------------------------------PROXIMITY----------------------------

//Detailed report
typedef struct {
    uint8_t direction;      //psProxDirection_enum (45 degree sectors clockwise from heading)
    uint8_t rangeReported;  //0 - 200 cm
    uint8_t confidence;     //0 - 100%
    uint8_t status;	
} psProxReportPayload_t;

//summary
typedef struct {       //bitmaps for 8 directions
	uint8_t proxStatus[8];		//ProxStatus_enum
} psProxSummaryPayload_t;


#endif	/* MESSAGEFORMATS_H */

