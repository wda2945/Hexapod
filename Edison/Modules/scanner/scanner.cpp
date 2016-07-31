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

#include "URG/Urg_driver.h"
#include <math.h>
#include <iostream>

FILE *scanDebugFile;

#ifdef SCANNER_DEBUG
#define DEBUGPRINT(...) tprintf( __VA_ARGS__);tfprintf(scanDebugFile, __VA_ARGS__);
#else
#define DEBUGPRINT(...) fprintf(scanDebugFile, __VA_ARGS__);fflush(scanDebugFile);
#endif

#define ERRORPRINT(...) tprintf( __VA_ARGS__);tfprintf(scanDebugFile, __VA_ARGS__);

int lidar_distance_scan();

int ScannerInit()
{
 	lidar_distance_scan();
	return 0;
}

//Proximity status call from Behavior Tree
ActionResult_enum proximityStatus(ProxSectorMask_enum _sectors,  ProxStatusMask_enum _status)
{
	return ACTION_FAIL;
}

using namespace qrk;
using namespace std;

namespace
{
    void print_data(const Urg_driver& urg,
                    const vector<long>& data, long time_stamp)
    {
#if 1
    // Shows only the front step
        int front_index = urg.step2index(0);
        cout << data[front_index] << " [mm], ("
             << time_stamp << " [msec])" << endl;

#else
    // Prints the X-Y coordinates for all the measurement points
        long min_distance = urg.min_distance();
        long max_distance = urg.max_distance();
        size_t data_n = data.size();
        for (size_t i = 0; i < data_n; ++i) {
            long l = data[i];
            if ((l <= min_distance) || (l >= max_distance)) {
                continue;
            }

            double radian = urg.index2rad(i);
            long x = static_cast<long>(l * cos(radian));
            long y = static_cast<long>(l * sin(radian));
            cout << "(" << x << ", " << y << ")" << endl;
        }
        cout << endl;
#endif
    }
}


int lidar_distance_scan()
{
//    Connection_information information(argc, argv);

    // Connects to the sensor
    Urg_driver urg;
    if (!urg.open("/dev/ttyACM0", 115200, Urg_driver::Serial)) 
    {
        return -1;
    }

    // Gets measurement data
#if 1
    // Case where the measurement range (start/end steps) is defined
    urg.set_scanning_parameter(urg.deg2step(-90), urg.deg2step(+90), 0);
#endif
    enum { Capture_times = 10 };
    urg.start_measurement(Urg_driver::Distance, Urg_driver::Infinity_times, 0);
    for (int i = 0; i < Capture_times; ++i) {
        vector<long> data;
        long time_stamp = 0;

        if (!urg.get_distance(data, &time_stamp)) {
            cout << "Urg_driver::get_distance(): " << urg.what() << endl;
            return 1;
        }
        print_data(urg, data, time_stamp);
    }

    return 0;
}
