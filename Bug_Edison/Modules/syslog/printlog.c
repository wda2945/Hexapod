/*
 ============================================================================
 Name        : PrintLogLinux.c
 Author      : Martin
 Version     :
 Copyright   : (c) 2013 Martin Lane-Smith
 Description : Print log messages to stdout/file
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "pubsubdata.h"
#include "syslog/syslog.h"

//print to an open stream - all processes
void PrintLogMessage(FILE *f, psMessage_t *msg)
{
	char *severity;

	const time_t now = time(NULL);
	struct tm *timestruct = localtime(&now);

	if (msg->header.messageType != SYSLOG_MSG)
		return;

	msg->logPayload.text[PS_MAX_LOG_TEXT-1] = '\0';

	switch (msg->logPayload.severity) {
	case SYSLOG_ROUTINE:
	default:
		severity = "R";
		break;
	case SYSLOG_INFO:
		severity = "I";
		break;
	case SYSLOG_WARNING:
		severity = "W";
		break;
	case SYSLOG_ERROR:
		severity = "E";
		break;
	case SYSLOG_FAILURE:
		severity = "F";
		break;
	}

	fprintf(f, "%02i:%02i:%02i %s@%s: %s\n",
			timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec,
			severity, msg->logPayload.file, msg->logPayload.text);
}


void DebugPrint(FILE *dbgfile, char *logtext)
{
	const time_t now = time(NULL);
	struct tm *timestruct = localtime(&now);

	fprintf(dbgfile, "%02i:%02i:%02i %s",
			timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec, logtext);

	fflush(dbgfile);
}
