/*
 ============================================================================
 Name        : scanner.c
 Author      : Martin
 Version     :
 Copyright   : (c) 2013 Martin Lane-Smith
 Description : Receives FOCUS messags from the overmind and directs the scanner accordingly.
 	 	 	   Receives proximity detector messages and publishes a consolidated PROX-REP
 ============================================================================
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "mraa.h"

#include "hexapod.h"

#include "scanner.hpp"

FILE *scanDebugFile;

#ifdef SCANNER_DEBUG
#define DEBUGPRINT(...) tprintf( __VA_ARGS__);tfprintf(scanDebugFile, __VA_ARGS__);
#else
#define DEBUGPRINT(...) fprintf(scanDebugFile, __VA_ARGS__);fflush(scanDebugFile);
#endif

#define ERRORPRINT(...) tprintf( __VA_ARGS__);tfprintf(scanDebugFile, __VA_ARGS__);

int ScannerInit()
{
	return 0;
}


//callback to monitor messages
void ScannerProcessMessage(psMessage_t *msg)
{

}

//Proximity status call from Behavior Tree
ActionResult_enum proximityStatus(ProxSectorMask_enum _sectors,  ProxStatusMask_enum _status)
{
	return ACTION_FAIL;
}
