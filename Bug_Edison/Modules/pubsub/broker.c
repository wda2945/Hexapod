/*
 * broker.c
 *
 *  Created on: Aug 7, 2015
 *      Author: martin
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include <helpers.h>
#include <pubsubdata.h>
#include <softwareProfile.h>
#include "pubsub/pubsub.h"
#include "pubsub/broker_debug.h"
#include "syslog/syslog.h"

FILE *psDebugFile;

//broker structures
//input queue
BrokerQueue_t brokerQueue = BROKER_Q_INITIALIZER;

void *BrokerInputThread(void *args);

int BrokerInit()
{
	pthread_t inputThread;

	psDebugFile = fopen_logfile("broker");
	DEBUGPRINT("Broker Logfile opened\n");

	//create thread to receive messages from the broker queue
	int s = pthread_create(&inputThread, NULL, BrokerInputThread, NULL);
	if (s != 0)
	{
		ERRORPRINT("input pthread_create\n");
		return s;
	}
	return 0;
}

//processes messages from the broker queue
void *BrokerInputThread(void *args)
{
	DEBUGPRINT("Broker Input thread started\n");
	while(1)
	{
		//process messages off the broker queue
		psMessage_t *msg = GetNextMessage(&brokerQueue);

		AdjustMessageLength(msg);

		if (msg->header.messageType != SYSLOG_MSG)
			DEBUGPRINT("Broker: %s\n", psLongMsgNames[msg->header.messageType]);

		RouteMessage(msg);

		DoneWithMessage(msg);
	}
}

