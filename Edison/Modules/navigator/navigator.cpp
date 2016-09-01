/*
 * navigator.cpp
 *
 *      Author: martin
 */
 
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stropts.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <errno.h>

#include "hexapod.h"

#include "brokerq/brokerq.h"

#include "navigator/navigator.hpp"
#include "navigator/kalman.h"
#include "navigator/matrix.h"

FILE *navDebugFile;

#ifdef NAVIGATOR_DEBUG
#define DEBUGPRINT(...) tprintf( __VA_ARGS__);tfprintf(navDebugFile, __VA_ARGS__);
#else
#define DEBUGPRINT(...) tfprintf(navDebugFile, __VA_ARGS__);
#endif

#define ERRORPRINT(...) tprintf( __VA_ARGS__);tfprintf(navDebugFile, __VA_ARGS__);

#define NORMALIZE_HEADING(x) (x + 360)%360

#define DEGREESTORADIANS(x) (x * M_PI / 180.0)
#define RADIANSTODEGREES(x) ((x * 180.0) / M_PI)

//reporting criteria
#define REPORT_BEARING_CHANGE 		2			//degrees
#define REPORT_CONFIDENCE_CHANGE 	0.2f	//probability
#define REPORT_MAX_INTERVAL			5			//seconds
#define REPORT_MIN_CONFIDENCE		0.5f
#define REPORT_LOCATION_CHANGE 		5

#define RAW_DATA_TIMEOUT 		5	//seconds

#define VERY_LARGE_COVARIANCE 1000000000.0

//main navigator thread
void *NavigatorThread(void *arg);

void NavigatorProcessMessage(const void *_msg, int len);
ps_registry_callback_t ProcessImuUpdate;

BrokerQueue_t navigatorQueue = BROKER_Q_INITIALIZER;

//incoming data
bool new_IMU_data {false};
bool IMUGood {false};
int IMU_raw_heading;
int IMU_raw_pitch;
int IMU_raw_roll;
time_t latestIMUReportTime = 0;

psOdometryPayload_t ODO_report;
bool new_ODO_data {false};
time_t latestOdoReportTime = 0;

//latest pose report
Position_struct position;
Orientation_struct orientation;

int NavigatorInit()
{
	navDebugFile = fopen_logfile("navigator");
	DEBUGPRINT("Navigator Logfile opened");

	ps_registry_add_new("Pose", "Latitude", PS_REGISTRY_REAL_TYPE, PS_REGISTRY_SRC_WRITE);
	ps_registry_add_new("Pose", "Longitude", PS_REGISTRY_REAL_TYPE, PS_REGISTRY_SRC_WRITE);
	ps_registry_add_new("Pose", "Heading", PS_REGISTRY_REAL_TYPE, PS_REGISTRY_SRC_WRITE);

	ps_registry_set_observer("IMU", "Heading", ProcessImuUpdate, &IMU_raw_heading);
	ps_registry_set_observer("IMU", "Pitch", ProcessImuUpdate, &IMU_raw_pitch);
	ps_registry_set_observer("IMU", "Roll", ProcessImuUpdate, &IMU_raw_roll);

	ps_subscribe(ODO_TOPIC, NavigatorProcessMessage);

	//create navigator thread
	pthread_t thread;
	int s = pthread_create(&thread, NULL, NavigatorThread, NULL);
	if (s != 0)
	{
		ERRORPRINT("NavigatorThread create failed. %i", s);
		return -1;
	}

	return 0;
}

void ProcessImuUpdate(const char *domain, const char *name, const void *arg)
{
	ps_registry_get_int(domain, name, (int*) arg);
	latestIMUReportTime = time(NULL);
	IMUGood = new_IMU_data = true;
}

void NavigatorProcessMessage(const void *_msg, int len)
{
	psMessage_t *msg = (psMessage_t *) _msg;

	switch(msg->messageType)
	{
	case ODOMETRY:
		ODO_report = msg->odometryPayload;
		new_ODO_data = true;
		break;
	default:
		break;
	}
}

void *NavigatorThread(void *arg)
{
	//set up filters
	////////////////////////////////////////////////////////////////////////
	//heading filter - 2 dimensions system (h, dh), 1 dimension measurement
	KalmanFilter HeadingFilter = alloc_filter(2, 1);
	set_identity_matrix(HeadingFilter.state_transition);
#define SET_HEADING_CHANGE(H) HeadingFilter.state_transition.data[0][1] = H;
	//then predict(f)

	/* We only observe (h) each time */
	set_matrix(HeadingFilter.observation_model,
		     1.0, 0.0);
#define SET_HEADING_OBSERVATION(H) set_matrix(HeadingFilter.observation, H);
	//then estimate(f)

	/* Noise in the world. */
	double pos = 10.0;
	set_matrix(HeadingFilter.process_noise_covariance,
		     pos, 0.0,
		     0.0, 1.0);
#define SET_HEADING_PROCESS_NOISE(N) HeadingFilter.process_noise_covariance.data[0][0] = N;

	/* Noise in our observation */
	set_matrix(HeadingFilter.observation_noise_covariance, 4.0);
#define SET_HEADING_OBSERVATION_NOISE(N) set_matrix(HeadingFilter.observation_noise_covariance, N);

	/* The start.heading is unknown, so give a high variance */
	set_matrix(HeadingFilter.state_estimate, 0.0, 0.0);
	set_identity_matrix(HeadingFilter.estimate_covariance);
	scale_matrix(HeadingFilter.estimate_covariance, 100000.0);
#define GET_HEADING NORMALIZE_HEADING((int) HeadingFilter.state_estimate.data[0][0])	//always 0 to 359

	while (1) {

		if (new_IMU_data)
		{
			new_IMU_data = false;

			SET_HEADING_OBSERVATION(IMU_raw_heading);
			SET_HEADING_OBSERVATION_NOISE(5.0)
			SET_HEADING_CHANGE(0.0);
			predict(HeadingFilter);
			estimate(HeadingFilter);

			DEBUGPRINT("Filtered heading: %i", GET_HEADING);
		}

		if (new_ODO_data)
		{
			new_ODO_data = false;

			//update heading belief
			float veerAngle = ODO_report.zRotation;
			SET_HEADING_CHANGE(veerAngle);
			SET_HEADING_OBSERVATION(HeadingFilter.predicted_state.data[0][0]);
			SET_HEADING_OBSERVATION_NOISE(VERY_LARGE_COVARIANCE)
			predict(HeadingFilter);
			estimate(HeadingFilter);

			DEBUGPRINT("ODO heading: %i", GET_HEADING);

			float hRadians = DEGREESTORADIANS(GET_HEADING);

			//update location belief
			position.latitude += (ODO_report.xMovement * cosf(hRadians));
			position.longitude += (ODO_report.yMovement * sinf(hRadians));

			DEBUGPRINT("ODO location: %f, %f",
					position.latitude,
					position.longitude);
		}

		if (time(NULL) - latestIMUReportTime > RAW_DATA_TIMEOUT)
		{
			IMUGood = false;
		}

		ps_registry_set_real("Pose", "Latitude", position.latitude);
		ps_registry_set_real("Pose", "Longitude", position.longitude);
		ps_registry_set_int("Pose", "Heading", GET_HEADING);

			DEBUGPRINT("NAV; Pose Msg: N=%f, E=%f, H=%i",
					position.latitude, position.longitude,
					orientation.heading )

		usleep(500000);
	}
}
