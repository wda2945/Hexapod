/*
 * lidar.cpp
 *
 *      Author: martin
 */
 
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

#include "mraa.h"
#include "mraa_internal_types.h"

#include "softwareProfile.h"
#include "pubsubdata.h"
#include "pubsub/pubsub.h"
#include "syslog/syslog.h"
#include "behavior/behavior_enums.h"
#include "lidar/lidar.h"
#include "RPLidar.h"

FILE *lidarDebugFile;

#ifdef LIDAR_DEBUG
#define DEBUGPRINT(...) fprintf(stdout, __VA_ARGS__);fprintf(lidarDebugFile, __VA_ARGS__);fflush(lidarDebugFile);
#else
#define DEBUGPRINT(...) fprintf(lidarDebugFile, __VA_ARGS__);fflush(lidarDebugFile);
#endif

#define ERRORPRINT(...) fprintf(stdout, __VA_ARGS__);fprintf(lidarDebugFile, __VA_ARGS__);fflush(lidarDebugFile);

//queue for messages
BrokerQueue_t lidarQueue = BROKER_Q_INITIALIZER;

//Lidar thread
void *LidarThread(void *arg);

mraa_uart_context uartContext;
int lidarFD;			//LIDAR uart file descriptor
mraa_pwm_context MOTOCTL_pwm;	//LIDAR motor pwm pin

#define ANGLE_BUCKET_DEGREES 45
#define ANGLE_BUCKET_COUNT	(360 / ANGLE_BUCKET_DEGREES)
#define RANGE_BUCKET_CM		 10
#define MAX_RANGE_BUCKET	200
#define RANGE_BUCKET_COUNT	(MAX_RANGE_BUCKET / RANGE_BUCKET_CM)
#define SCAN_COUNT			10
#define COUNT_THRESHOLD		6
#define MIN_REPORT_INTERVAL 2

int proximityCounts[ANGLE_BUCKET_COUNT][RANGE_BUCKET_COUNT];
int obstacleRanges[ANGLE_BUCKET_COUNT];
int obstacleHits[ANGLE_BUCKET_COUNT];
int reportedRanges[ANGLE_BUCKET_COUNT];
time_t reportedTimes[ANGLE_BUCKET_COUNT];

int CheckLidarHealth();

int LidarInit()
{
	lidarDebugFile = fopen_logfile("lidar");
	DEBUGPRINT("Lidar Logfile opened\n");

	//set up serial port context
	uartContext = mraa_uart_init(RPLIDAR_UART_DEVICE);
	if (uartContext == 0) {
		ERRORPRINT("Lidar mraa_uart_init(%i) fail - %s\n", RPLIDAR_UART_DEVICE, strerror(errno));
		return -1;
	}
	else DEBUGPRINT("Lidar uart context created\n");

	const char *devicePath = mraa_uart_get_dev_path(uartContext);

	lidarFD = uartContext->fd;

	if (mraa_uart_set_baudrate(uartContext, RPLIDAR_UART_BAUDRATE) != MRAA_SUCCESS)
	{
		ERRORPRINT("Lidar mraa_uart_set_baudrate() fail\n");
		return -1;
	}
	else DEBUGPRINT("Lidar uart baudrate: %i\n", RPLIDAR_UART_BAUDRATE);

	if (mraa_uart_set_mode(uartContext, 8, MRAA_UART_PARITY_NONE, 1) != MRAA_SUCCESS)
	{
		ERRORPRINT("Lidar mraa_uart_set_mode() fail\n");
		return -1;
	}
	else DEBUGPRINT("Lidar mraa_uart_set_mode() OK\n");

	if (mraa_uart_set_flowcontrol(uartContext, false, false) != MRAA_SUCCESS)
	{
		ERRORPRINT("Lidar mraa_uart_set_flowcontrol() fail\n");
		return -1;
	}
	else DEBUGPRINT("Lidar mraa_uart_set_flowcontrol() OK\n");

	DEBUGPRINT("Lidar uart %s configured\n", devicePath);

//	mraa_gpio_context gpio_context = mraa_gpio_init(RPLIDAR_PWM_PIN);
//	if (mraa_gpio_dir(gpio_context, MRAA_GPIO_OUT_HIGH) != MRAA_SUCCESS)
//	{
//		ERRORPRINT("RPLIDAR CTL Pin error\n");
//	}

//	mraa_gpio_context gpio1_context = mraa_gpio_init(J19_9);
//	if (mraa_gpio_dir(gpio1_context, MRAA_GPIO_IN) != MRAA_SUCCESS)
//	{
//		ERRORPRINT("J19_9 Pin error\n");
//	}
//
//	mraa_gpio_context gpio2_context = mraa_gpio_init(J20_9);
//	if (mraa_gpio_dir(gpio2_context, MRAA_GPIO_IN) != MRAA_SUCCESS)
//	{
//		ERRORPRINT("J20_9 Pin error\n");
//	}


	// init pwm
	MOTOCTL_pwm = mraa_pwm_init(RPLIDAR_PWM_PIN);
	if (MOTOCTL_pwm == NULL) {
		ERRORPRINT("Can't create MOTOCTL PWM object\n");
		return -1;
	}
	else DEBUGPRINT("MOTOCTL PWM pin init() OK\n");

	// disable PWM on the selected pin
	if (mraa_pwm_enable(MOTOCTL_pwm, 0) != MRAA_SUCCESS) {
		ERRORPRINT("Can't enable MOTOCTL PWM\n");
		return -1;
	}
	else DEBUGPRINT("MOTOCTL PWM pin enable() OK\n");

	//period = 50Hz
	if (mraa_pwm_period_ms(MOTOCTL_pwm, 20) != MRAA_SUCCESS)  {
		ERRORPRINT("Can't set MOTOCTL PWM period\n");
		return -1;
	}
	else DEBUGPRINT("MOTOCTL PWM pin period = 20mS\n");

	//duty = 100%
	if (mraa_pwm_pulsewidth_ms(MOTOCTL_pwm, 19) != MRAA_SUCCESS)  {
		ERRORPRINT("Can't set MOTOCTL PWM pulse-width\n");
		return -1;
	}
	else DEBUGPRINT("MOTOCTL PWM pin pulse-width = 19mS\n");

	// enable PWM on the selected pin
	if (mraa_pwm_enable(MOTOCTL_pwm, 1) != MRAA_SUCCESS) {
		ERRORPRINT("Can't enable MOTOCTL PWM\n");
		return -1;
	}
	else DEBUGPRINT("MOTOCTL PWM pin enable() OK\n");

	//create thread to receive RPLIDAR messages
	pthread_t thread;
	int s = pthread_create(&thread, NULL, LidarThread, NULL);
	if (s != 0) {
		ERRORPRINT("pthread_create %i %s\n", s, strerror(errno));
		return -1;
	}

	return 0;
}

void LidarProcessMessage(psMessage_t *msg)
{
	//none at present
}

void *LidarThread(void *arg)
{
	int scanCount = SCAN_COUNT;

	DEBUGPRINT("Lidar thread ready\n");

	//check health
	CheckLidarHealth();

	//wait for RPLIDAR to reach speed
	sleep(2);

	//start scan
	DEBUGPRINT("LIDAR startScan()\n");
    while (startRPLidarScan(false) != RESULT_OK)
    {
		ERRORPRINT("startScan() fail\n");
    	sleep(10);
    }

    struct RPLidarMeasurement *lidarMeasurement;

	while (1) {

		//wait for next message from RPLIDAR
		if (waitRPLidarPoint() != RESULT_OK)
		{
			ERRORPRINT("waitPoint() timeout\n");
			sleep(5);

			startRPLidarScan(false);
			continue;
		}

		//get the data
		lidarMeasurement = getRPLidarCurrentPoint();

		DEBUGPRINT("Angle: %f, Distance: %f", lidarMeasurement->angle, lidarMeasurement->distance);

		int angleBucket = (int) floor(lidarMeasurement->angle / ANGLE_BUCKET_DEGREES);
		int rangeBucket =  (int) floor(lidarMeasurement->distance / RANGE_BUCKET_CM);
		if (rangeBucket >= RANGE_BUCKET_COUNT) rangeBucket = RANGE_BUCKET_COUNT-1;

		proximityCounts[angleBucket][rangeBucket]++;

		if (lidarMeasurement->startBit) scanCount--;

		if (scanCount <= 0)
		{
			int i,j;

			scanCount = SCAN_COUNT;

			//look for obstacles
			for (i=0; i<ANGLE_BUCKET_COUNT; i++)
			{
				int maxCount = -1;
				int range = -1;
				//find max count
				for (j=0; j<RANGE_BUCKET_COUNT; j++)
				{
					if (proximityCounts[i][j] > maxCount)
					{
						maxCount = proximityCounts[i][j];
						range = j;
					}
				}
				if (maxCount > COUNT_THRESHOLD)
				{
					//obstacle to report
					obstacleRanges[i] = range;
					obstacleHits[i] = maxCount;
				}
				else
				{
					//no obstacle to report
					obstacleRanges[i] = -1;
					obstacleHits[i] = 0;
				}
			}
			//send proximity message
			for (i=0; i<ANGLE_BUCKET_COUNT; i++)
			{
				if (obstacleHits[i])
				{
					if ((reportedRanges[i] != obstacleRanges[i])
						&& ((reportedTimes[i] + MIN_REPORT_INTERVAL) < time(NULL)))
					{
						//send report
						reportedRanges[i] = obstacleRanges[i];
						reportedTimes[i] = time(NULL);
					}
				}
			}

			//zero out counting array
			for (i=0; i<ANGLE_BUCKET_COUNT; i++)
			{
				for (j=0; j<RANGE_BUCKET_COUNT; j++)
				{
					proximityCounts[i][j] = 0;
				}
			}
		}

	}
	return 0;
}

bool proximityStatus(unsigned int sector, unsigned int proximity)
{
	return false;
}

int CheckLidarHealth()
{
	rplidar_response_device_health_t healthinfo;
	while (1)
	{
		switch (getRPLidarHealth(&healthinfo))
		{
		case RESULT_OK:
		default:
		{
			switch(healthinfo.status)
			{
			default:
			case 0:
				DEBUGPRINT("getHealth() status OK\n");
				return 0;
				break;
			case 1:
			case 2:
				ERRORPRINT("getHealth() status %i, error %i\n", healthinfo.status, healthinfo.error_code);
				sleep(1);
				sendRPLidarReset();
				sleep(1);
				break;
			}
		}
		break;
		case RESULT_INVALID_DATA:
			ERRORPRINT("getHealth() invalid data\n");
			sleep(1);
			sendRPLidarReset();
			sleep(1);
			break;
		case RESULT_OPERATION_TIMEOUT:
			ERRORPRINT("getHealth() timeout\n");
			sleep(1);
			sendRPLidarReset();
			sleep(1);
			break;
		}
	}
	return -1;
}