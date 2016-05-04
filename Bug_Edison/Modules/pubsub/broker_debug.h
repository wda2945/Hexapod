//broker_debug.h


#ifndef BROKER_DEBUG_H
#define BROKER_DEBUG_H

#include <stdio.h>
#include "softwareProfile.h"

extern FILE *psDebugFile;

#ifdef BROKER_DEBUG
#define DEBUGPRINT(...) tprintf( __VA_ARGS__);tfprintf(psDebugFile, __VA_ARGS__);
#else
#define DEBUGPRINT(...) tfprintf(psDebugFile, __VA_ARGS__);
#endif

#define ERRORPRINT(...) tprintf( __VA_ARGS__);tfprintf(psDebugFile, __VA_ARGS__);

#endif
