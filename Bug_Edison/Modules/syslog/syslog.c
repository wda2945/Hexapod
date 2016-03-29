/*
 * SysLog.c
 *
 *  Created on: Sep 10, 2014
 *      Author: martin
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "pubsubdata.h"
#include "pubsub/pubsub.h"
#include "syslog/syslog.h"
#include "behavior/behavior.h"

//Sys Log queue
BrokerQueue_t logQueue = BROKER_Q_INITIALIZER;

void *LoggingThread(void *arg);

//print to an open stream
void PrintLogMessage(FILE *f, psMessage_t *msg);

FILE *logFile;

//open logging file
int SysLogInit()
{
	char logfilepath[200];
	const time_t now = time(NULL);
	struct tm *timestruct = localtime(&now);

	snprintf(logfilepath, 200, "%s/SYS_%4i_%02i_%02i_%02i_%02i_%02i.log", LOGFILE_FOLDER,
			timestruct->tm_year + 1900, timestruct->tm_mon, timestruct->tm_mday,
			timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);


	logFile = fopen(logfilepath, "w");
	if (logFile == NULL)
	{
		logFile = stderr;
		fprintf(stderr, "syslog: fopen(%s) fail (%s)\n", logfilepath, strerror(errno));
	}
	else {
		fprintf(stderr, "syslog: Logfile opened on %s\n", logfilepath);
	}

    //start log print threads
    pthread_t thread;

    int s = pthread_create(&thread, NULL, LoggingThread, NULL);
    if (s != 0)
    {
    	fprintf(stderr, "syslog: pthread_create fail (%s)\n", strerror(errno));
    	fprintf(logFile, "syslog: pthread_create fail (%s)\n", strerror(errno));
    	fflush(stdout);
    	fflush(logFile);
        return s;
    }

	fprintf(stdout, "syslog: pthread_create OK\n");
	fprintf(logFile, "syslog: pthread_create OK\n");
	fflush(stdout);
	fflush(logFile);

	return 0;
}
//writes log messages to the logfile
void *LoggingThread(void *arg)
{
	fprintf(stdout, "syslog: thread started\n");
    {
    	psMessage_t msg;
    	psInitPublish(msg, SYSLOG_MSG);
    	msg.logPayload.severity = SYSLOG_ROUTINE;
    	strcpy(msg.logPayload.file, "log");
    	strcpy(msg.logPayload.text, "Logfile started");

    	PrintLogMessage(logFile, &msg);
    }
	while (1)
	{
		psMessage_t *msg = GetNextMessage(&logQueue);
		uint8_t severity;

		severity = msg->logPayload.severity;

		//print to logfile
		if (SYSLOG_LEVEL <= severity) {
			PrintLogMessage(logFile, msg);
			fflush(logFile);
		}
        //print a copy to stdout
        if (LOG_TO_SERIAL <= severity) {
        	PrintLogMessage(stdout, msg);
        	fflush(stdout);
        }
        DoneWithMessage(msg);
	}
	return 0;
}
//accept a message from the broker for logging
void SysLogProcessMessage(psMessage_t *msg)
{
	switch (msg->header.messageType)
	{
	case SYSLOG_MSG:
		CopyMessageToQ(&logQueue, msg);
		break;
	default:
		break;
	}
}

//create new log message. called by macros in syslog.h
void _LogMessage(SysLogSeverity_enum _severity, const char *_message, const char *_file)
{
	if (SYSLOG_LEVEL <= _severity ) {
		const char *filecomponent = (strrchr(_file, '/') + 1);
		if (!filecomponent) filecomponent = _file;
		if (!filecomponent) filecomponent = "****";

		psMessage_t msg;
		psInitPublish(msg, SYSLOG_MSG);
		msg.logPayload.severity = _severity;
		strncpy(msg.logPayload.file, filecomponent, FILE_NAME_LENGTH);
		msg.logPayload.file[FILE_NAME_LENGTH] = 0;
		strncpy(msg.logPayload.text, _message, PS_MAX_LOG_TEXT);

		//publish a copy
		RouteMessage(&msg);
	}
}

FILE *fopen_logfile(char *name)
{
	char path[100];
	sprintf(path, "%s/%s.log", LOGFILE_FOLDER, name);
	return fopen(path, "w");
}
