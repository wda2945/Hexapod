/* 
 * File:   SysLog.h
 * Author: martin
 *
 * Created on August 7, 2013, 8:19 PM
 */

#ifndef SYSLOG_H
#define	SYSLOG_H

#include <stdio.h>

#include "pubsubdata.h"
#include "pubsub/pubsub.h"

#ifndef __BASE_FILE__
#define __BASE_FILE__ __FILE__
#endif

//logfiles folder
#define LOGFILE_FOLDER 			"/home/root/logfiles"

FILE *fopen_logfile(char *name);

void _LogMessage(SysLogSeverity_enum _severity, const char *_message, const char *_file);

#define SysLog( s, ...) {char tmp[PS_MAX_LOG_TEXT+1];\
    snprintf(tmp,PS_MAX_LOG_TEXT,__VA_ARGS__);\
    tmp[PS_MAX_LOG_TEXT] = 0;\
    _LogMessage(s, tmp, __BASE_FILE__);}


#if (SYSLOG_LEVEL == SYSLOG_ROUTINE) || (LOG_TO_SERIAL == SYSLOG_ROUTINE)
#define LogRoutine( ...) {SysLog( SYSLOG_ROUTINE, __VA_ARGS__);}
#else
    #define LogRoutine( m, ...)
#endif

    #if (SYSLOG_LEVEL <= SYSLOG_INFO) || (LOG_TO_SERIAL <= SYSLOG_INFO)
#define LogInfo( ...) {SysLog( SYSLOG_INFO, __VA_ARGS__);}
#else
    #define LogInfo( m, ...)
#endif

#if ((SYSLOG_LEVEL <= SYSLOG_WARNING) || (LOG_TO_SERIAL <= SYSLOG_WARNING))
#define LogWarning( ...) {SysLog( SYSLOG_WARNING, __VA_ARGS__);}
#else
    #define LogWarning( m, ...)
#endif

#define LogError(...) {SysLog(SYSLOG_ERROR, __VA_ARGS__);}

//syslog
extern FILE *logFile;

int SysLogInit();

void SysLogProcessMessage(psMessage_t *msg);				//process for logging

void PrintLogMessage(FILE *f, psMessage_t *msg);

#define tprintf(...) {char tmp[100];\
    snprintf(tmp, 100, __VA_ARGS__);\
    tmp[100-1] = 0;\
    DebugPrint(stdout, tmp);}

#define tfprintf(dbgfile, ...) {char tmp[100];\
    snprintf(tmp,100,__VA_ARGS__);\
    tmp[100-1] = 0;\
    DebugPrint(dbgfile, tmp);}

void DebugPrint(FILE *dbgfile, char *logtext);

#endif	/* SYSLOG_H */

