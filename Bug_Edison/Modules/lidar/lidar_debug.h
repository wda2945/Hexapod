/*
 * lidar_debug.h
 *
 *      Author: martin
 */

#ifndef LIDAR_DEBUG_HPP
#define LIDAR_DEBUG_HPP

#include "softwareProfile.h"
#include "syslog/syslog.h"

extern FILE *lidarDebugFile;

#ifdef LIDAR_DEBUG
#define DEBUGPRINT(...) tprintf( __VA_ARGS__);tfprintf(lidarDebugFile, __VA_ARGS__);
#else
#define DEBUGPRINT(...) tfprintf(lidarDebugFile, __VA_ARGS__);
#endif

#define ERRORPRINT(...) tprintf( __VA_ARGS__);tfprintf(lidarDebugFile, __VA_ARGS__);

#endif
