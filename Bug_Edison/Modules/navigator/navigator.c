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

#include "softwareProfile.h"
#include "pubsubdata.h"
#include "pubsub/pubsub.h"
#include "syslog/syslog.h"

#include "navigator/navigator.h"
#include "navigator/kalman.h"
#include "navigator/matrix.h"

FILE *navDebugFile;

#ifdef NAVIGATOR_DEBUG
#define DEBUGPRINT(...) fprintf(stdout, __VA_ARGS__);fprintf(navDebugFile, __VA_ARGS__);fflush(navDebugFile);
#else
#define DEBUGPRINT(...) fprintf(navDebugFile, __VA_ARGS__);fflush(navDebugFile);
#endif

#define ERRORPRINT(...) fprintf(stdout, __VA_ARGS__);fprintf(navDebugFile, __VA_ARGS__);fflush(navDebugFile);

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

BrokerQueue_t navigatorQueue = BROKER_Q_INITIALIZER;

int NavigatorInit()
{
	navDebugFile = fopen_logfile("navigator");
	DEBUGPRINT("Navigator Logfile opened\n");

	//create navigator thread
	pthread_t thread;
	int s = pthread_create(&thread, NULL, NavigatorThread, NULL);
	if (s != 0)
	{
		ERRORPRINT("NavigatorThread create failed. %i\n", s);
		return -1;
	}

	return 0;
}

void NavigatorProcessMessage(psMessage_t *msg)
{
	CopyMessageToQ(&navigatorQueue, msg);
}

void *NavigatorThread(void *arg)
{
	psMessage_t *msg;

	//Robot pose
	float roll = 0;
	float pitch = 0;

	//incoming data
	ps3FloatPayload_t IMU_report;
	psOdometryPayload_t ODO_report;

	time_t latestIMUReportTime = 0;
	time_t latestReportTime = 0;
	time_t latestAppReportTime = 0;


	//latest pose report
	psMessage_t poseMsg;
	psPosePayload_t lastPoseMsg;

	bool IMUGood;
	bool reportRequired;

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

//	PRINT_MATRIX(HeadingFilter.state_transition);
//	PRINT_MATRIX(HeadingFilter.process_noise_covariance);
//	PRINT_MATRIX(HeadingFilter.observation_model);
//	PRINT_MATRIX(HeadingFilter.observation);
//	PRINT_MATRIX(HeadingFilter.observation_noise_covariance);
//	PRINT_MATRIX(HeadingFilter.state_estimate);
//	PRINT_MATRIX(HeadingFilter.estimate_covariance);

	while (1) {

		msg = GetNextMessage(&navigatorQueue);

		DEBUGPRINT("Navigator RX: %s\n", psLongMsgNames[msg->header.messageType]);

		reportRequired = false;

		switch (msg->header.messageType)
		{
		case IMU_REPORT:
		{
			latestIMUReportTime = time(NULL);
			IMU_report = msg->threeFloatPayload;
			IMUGood = true;
			//update heading belief
			roll = IMU_report.roll;
			pitch = IMU_report.pitch;
			SET_HEADING_OBSERVATION(IMU_report.heading);
			SET_HEADING_OBSERVATION_NOISE(5.0)
			SET_HEADING_CHANGE(0.0);
			predict(HeadingFilter);
			estimate(HeadingFilter);

			DEBUGPRINT("Filtered heading: %i\n", GET_HEADING);
		}
		DoneWithMessage(msg);
		break;
		case ODOMETRY:
		{
			ODO_report = msg->odometryPayload;

			//update heading belief
			float veerAngle = ODO_report.zRotation;
			SET_HEADING_CHANGE(veerAngle);
			SET_HEADING_OBSERVATION(HeadingFilter.predicted_state.data[0][0]);
			SET_HEADING_OBSERVATION_NOISE(VERY_LARGE_COVARIANCE)
			predict(HeadingFilter);
			estimate(HeadingFilter);

			DEBUGPRINT("ODO heading: %i\n", GET_HEADING);

			float hRadians = DEGREESTORADIANS(GET_HEADING);

			//update location belief
			poseMsg.posePayload.position.latitude += (ODO_report.xMovement * cosf(hRadians));
			poseMsg.posePayload.position.longitude += (ODO_report.yMovement * sinf(hRadians));

			DEBUGPRINT("ODO location: %f, %f\n",
					poseMsg.posePayload.position.latitude,
					poseMsg.posePayload.position.longitude);
		}
		DoneWithMessage(msg);
		break;
		default:
			DoneWithMessage(msg);
			break;
		}

		if (time(NULL) - latestIMUReportTime > RAW_DATA_TIMEOUT)
		{
			IMUGood = false;
		}


				//whether to report
#define max(a,b) (a>b?a:b)

				if ((abs(lastPoseMsg.position.latitude - poseMsg.posePayload.position.latitude) > REPORT_LOCATION_CHANGE) ||
						(abs(lastPoseMsg.position.longitude - poseMsg.posePayload.position.longitude) > REPORT_LOCATION_CHANGE) ||
						(abs(NORMALIZE_HEADING(lastPoseMsg.orientation.heading - poseMsg.posePayload.orientation.heading)) > REPORT_BEARING_CHANGE) ||
						(latestReportTime + REPORT_MAX_INTERVAL > time(NULL)))
				{
					reportRequired = true;
				}


		if (reportRequired)
		{
			psInitPublish(poseMsg, POSE);

			poseMsg.posePayload.orientation.roll = roll;
			poseMsg.posePayload.orientation.pitch = pitch;
			poseMsg.posePayload.orientation.heading = GET_HEADING;
			poseMsg.posePayload.orientationValid = IMUGood;
			poseMsg.posePayload.positionValid = true;

			lastPoseMsg = poseMsg.posePayload;
			RouteMessage(&poseMsg);
			latestReportTime = time(NULL);

			DEBUGPRINT("NAV; Pose Msg: N=%f, E=%f, H=%i\n",
					poseMsg.posePayload.position.latitude,
					poseMsg.posePayload.position.longitude,
					lastPoseMsg.orientation.heading )

			if (latestAppReportTime + appReportInterval < time(NULL)){
				//send another to the App
				psInitPublish(poseMsg, POSEREP);
				RouteMessage(&poseMsg);
				latestAppReportTime = time(NULL);
			}
		}

	}
}
