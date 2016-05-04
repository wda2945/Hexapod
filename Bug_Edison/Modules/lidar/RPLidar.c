

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <stropts.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <errno.h>

#include "lidar.h"
#include "lidar/lidar_debug.h"
#include "RPLidar.h"

u_result _sendCommand(_u8 cmd, const void * payload, size_t payloadsize);
u_result _waitResponseHeader(rplidar_ans_header_t * header);

struct RPLidarMeasurement _currentMeasurement;

int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y);

bool NoTimeout(struct timeval startTs);

//send reset
u_result sendRPLidarReset()
{
	u_result  ans;

	if (IS_FAIL(ans = _sendCommand(RPLIDAR_CMD_GET_DEVICE_HEALTH, NULL, 0)))
	{
		return ans;
	}

	return RESULT_OK;
}


// ask the RPLIDAR for its health info
u_result getRPLidarHealth(rplidar_response_device_health_t *healthinfo)
{
	struct timeval startTs;
	gettimeofday(&startTs, NULL);

	_u8 *infobuf = (_u8 *)healthinfo;
	_u8 recvPos = 0;

	rplidar_ans_header_t response_header;
	u_result  ans;


	if (IS_FAIL(ans = _sendCommand(RPLIDAR_CMD_GET_DEVICE_HEALTH, NULL, 0))) {
		return ans;
	}

	if (IS_FAIL(ans = _waitResponseHeader(&response_header))) {
		return ans;
	}

	// verify whether we got a correct header
	if (response_header.type != RPLIDAR_ANS_TYPE_DEVHEALTH) {
		return RESULT_INVALID_DATA;
	}

	if ((response_header.size) < sizeof(rplidar_response_device_health_t)) {
		return RESULT_INVALID_DATA;
	}

	while (NoTimeout(startTs)) {
		int currentbyte;
		int count = read(lidarFD, &currentbyte, 1);

		if (count == 0 || currentbyte < 0) continue;

		infobuf[recvPos++] = currentbyte;

		if (recvPos == sizeof(rplidar_response_device_health_t)) {
			return RESULT_OK;
		}
	}

	return RESULT_OPERATION_TIMEOUT;
}

// ask the RPLIDAR for its device info like the serial number
u_result getRPLidarDeviceInfo(rplidar_response_device_info_t *info)
{
    _u8  recvPos = 0;
    struct timeval startTs;
    gettimeofday(&startTs, NULL);
    _u8 *infobuf = (_u8*)info;
    rplidar_ans_header_t response_header;
    u_result  ans;

    if (IS_FAIL(ans = _sendCommand(RPLIDAR_CMD_GET_DEVICE_INFO,NULL,0))) {
    	return ans;
    }

    if (IS_FAIL(ans = _waitResponseHeader(&response_header))) {
    	return ans;
    }

    // verify whether we got a correct header
    if (response_header.type != RPLIDAR_ANS_TYPE_DEVINFO) {
    	return RESULT_INVALID_DATA;
    }

    if (response_header.size < sizeof(rplidar_response_device_info_t)) {
    	return RESULT_INVALID_DATA;
    }

    while (NoTimeout(startTs)) {
    	int currentbyte;
    	int count = read(lidarFD, &currentbyte, 1);

    	if (count == 0 || currentbyte < 0) continue;
    	infobuf[recvPos++] = currentbyte;

    	if (recvPos == sizeof(rplidar_response_device_info_t)) {
    		return RESULT_OK;
    	}
    }

    return RESULT_OPERATION_TIMEOUT;
}

// stop the measurement operation
u_result stopRPLidar()
{
    u_result ans = _sendCommand(RPLIDAR_CMD_STOP,NULL,0);
    return ans;
}

// start the measurement operation
u_result startRPLidarScan(bool force)
{
	DEBUGPRINT("RPLIDAR Start Scan\n");

	u_result ans;
    
    stopRPLidar(); //force the previous operation to stop

    ans = _sendCommand(force?RPLIDAR_CMD_FORCE_SCAN:RPLIDAR_CMD_SCAN, NULL, 0);
    if (IS_FAIL(ans)) return ans;

    // waiting for confirmation
    rplidar_ans_header_t response_header;
    if (IS_FAIL(ans = _waitResponseHeader(&response_header))) {
    	ERRORPRINT("RPLIDAR Start Scan Wait Response Fail\n");
    	return ans;
    }

    // verify whether we got a correct header
    if (response_header.type != RPLIDAR_ANS_TYPE_MEASUREMENT) {
    	ERRORPRINT("RPLIDAR Start Scan Invalid data\n");
    	return RESULT_INVALID_DATA;
    }

    if (response_header.size < sizeof(rplidar_response_measurement_node_t)) {
    	ERRORPRINT("RPLIDAR Start Scan Invalid Data\n");
    	return RESULT_INVALID_DATA;
    }

	DEBUGPRINT("RPLIDAR Start Scan OK\n");
    return RESULT_OK;
}

// wait for one sample point to arrive
u_result waitRPLidarPoint()
{
	DEBUGPRINT("RPLIDAR Wait Point\n");

    struct timeval startTs;
    gettimeofday(&startTs, NULL);
   rplidar_response_measurement_node_t node;
   _u8 *nodebuf = (_u8*)&node;

   _u8 recvPos = 0;

   while (NoTimeout(startTs)) {
       int currentbyte;
       int count = read(lidarFD, &currentbyte, 1);

       if (count == 0 || currentbyte < 0) continue;

        switch (recvPos) {
            case 0: // expect the sync bit and its reverse in this byte          {
                {
                    _u8 tmp = (currentbyte>>1);
                    if ( (tmp ^ currentbyte) & 0x1 ) {
                        // pass
                    } else {
                        continue;
                    }

                }
                break;
            case 1: // expect the highest bit to be 1
                {
                    if (currentbyte & RPLIDAR_RESP_MEASUREMENT_CHECKBIT) {
                        // pass
                    } else {
                        recvPos = 0;
                        continue;
                    }
                }
                break;
          }
          nodebuf[recvPos++] = currentbyte;

          if (recvPos == sizeof(rplidar_response_measurement_node_t)) {
              // store the data ...
              _currentMeasurement.distance = node.distance_q2/4.0f;
              _currentMeasurement.angle = (node.angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT)/64.0f;
              _currentMeasurement.quality = (node.sync_quality>>RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT);
              _currentMeasurement.startBit = (node.sync_quality & RPLIDAR_RESP_MEASUREMENT_SYNCBIT);

              DEBUGPRINT("RPLIDAR Point Data Received\n");

              return RESULT_OK;
          }
        
   }
   ERRORPRINT("RPLIDAR Wait Point Timeout\n");
   return RESULT_OPERATION_TIMEOUT;
}



u_result _sendCommand(_u8 cmd, const void * payload, size_t payloadsize)
{
	DEBUGPRINT("RPLIDAR Send Command: %2x + %i bytes\n", (unsigned int) cmd, payloadsize);

    rplidar_cmd_packet_t pkt_header;
    rplidar_cmd_packet_t * header = &pkt_header;
    _u8 checksum = 0;

    if (payloadsize && payload) {
        cmd |= RPLIDAR_CMDFLAG_HAS_PAYLOAD;
    }

    header->syncByte = RPLIDAR_CMD_SYNC_BYTE;
    header->cmd_flag = cmd;

    // send header first
    write(lidarFD,  (uint8_t *)header, 2);

    if (cmd & RPLIDAR_CMDFLAG_HAS_PAYLOAD) {
        checksum ^= RPLIDAR_CMD_SYNC_BYTE;
        checksum ^= cmd;
        checksum ^= (payloadsize & 0xFF);

        // calc checksum
        size_t pos;
        for (pos = 0; pos < payloadsize; ++pos) {
            checksum ^= ((_u8 *)payload)[pos];
        }

        // send size
        _u8 sizebyte = payloadsize;
        write(lidarFD,  (uint8_t *)&sizebyte, 1);

        // send payload
        write(lidarFD,  (uint8_t *)&payload, sizebyte);

        // send checksum
        write(lidarFD,  (uint8_t *)&checksum, 1);

    }

    return RESULT_OK;
}

u_result _waitResponseHeader(rplidar_ans_header_t * header)
{
	DEBUGPRINT("RPLIDAR Wait Response Header\n");

    _u8  recvPos = 0;
    struct timeval startTs;
    gettimeofday(&startTs, NULL);
    _u8 *headerbuf = (_u8*)header;
    while (NoTimeout(startTs)) {
        int currentbyte;
        int count = read(lidarFD, &currentbyte, 1);

        if (count == 0 || currentbyte < 0) continue;
        switch (recvPos) {
        case 0:
            if (currentbyte != RPLIDAR_ANS_SYNC_BYTE1) {
                continue;
            }
            break;
        case 1:
            if (currentbyte != RPLIDAR_ANS_SYNC_BYTE2) {
                recvPos = 0;
                continue;
            }
            break;
        }
        headerbuf[recvPos++] = currentbyte;

        if (recvPos == sizeof(rplidar_ans_header_t)) {
        	DEBUGPRINT("RPLIDAR Got Response Header\n");
            return RESULT_OK;
        }
    }
    ERRORPRINT("RPLIDAR Response Header Timeout\n");
    return RESULT_OPERATION_TIMEOUT;
}

struct RPLidarMeasurement *getRPLidarCurrentPoint()
{
	return &_currentMeasurement;
}

bool NoTimeout(struct timeval startTs)
{
	struct timeval currenttime;
	struct timeval diff;
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;
	gettimeofday(&currenttime, NULL);
	timeval_subtract(&diff, &currenttime, &startTs);
	return (timeval_subtract(&diff, &diff, &timeout) == 1);
}


/* Subtract the ‘struct timeval’ values X and Y, storing the result in RESULT.
Return 1 if the difference is negative, otherwise 0. */
int timeval_subtract (result, x, y)
struct timeval *result, *x, *y;
{
	/* Perform the carry for the later subtraction by updating y. */ if (x->tv_usec < y->tv_usec) {
		int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
		y->tv_usec -= 1000000 * nsec;
		y->tv_sec += nsec;
	}
	if (x->tv_usec - y->tv_usec > 1000000) {
		int nsec = (x->tv_usec - y->tv_usec) / 1000000;
		y->tv_usec += 1000000 * nsec;
		y->tv_sec -= nsec;
	}
	/* Compute the time remaining to wait. tv_usec is certainly positive. */
	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_usec = x->tv_usec - y->tv_usec;
	/* Return 1 if result is negative. */
	return x->tv_sec < y->tv_sec;
}
