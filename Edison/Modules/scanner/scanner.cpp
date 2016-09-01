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

using namespace qrk;
using namespace std;

FILE *scanDebugFile;

#ifdef SCANNER_DEBUG
#define DEBUGPRINT(...) tprintf( __VA_ARGS__);tfprintf(scanDebugFile, __VA_ARGS__);
#else
#define DEBUGPRINT(...) fprintf(scanDebugFile, __VA_ARGS__);fflush(scanDebugFile);
#endif

#define ERRORPRINT(...) tprintf( __VA_ARGS__);tfprintf(scanDebugFile, __VA_ARGS__);

// Connects to the sensor
Urg_driver urg;

typedef struct {
	ProxSectorMask_enum mask;
	ProxStatusMask_enum status;
	ps_condition_id_t closeCondition;
	ps_condition_id_t farCondition;
	int fromDegree, toDegree;
	int closeCount, farCount, totalCount;
} SectorStruct_t;

SectorStruct_t sectors[8];

void *ScannerThread(void *arg);

int lidar_distance_scan();

int ScannerInit()
{
	pthread_t thread;
	int s, i;

	sectors[0].mask = PROX_FRONT_MASK;
	sectors[0].closeCondition = FRONT_CENTER_PROXIMITY;
	sectors[0].farCondition = FRONT_FAR_PROXIMITY;
	sectors[1].mask = PROX_FRONT_RIGHT_MASK;
	sectors[1].closeCondition = FRONT_RIGHT_PROXIMITY;
	sectors[1].farCondition = FRONT_RIGHT_FAR_PROXIMITY;
	sectors[2].mask = PROX_RIGHT_MASK;
	sectors[2].closeCondition = RIGHT_PROXIMITY;
	sectors[2].farCondition = 0;
	sectors[3].mask = PROX_REAR_RIGHT_MASK;
	sectors[3].closeCondition = REAR_RIGHT_PROXIMITY;
	sectors[3].farCondition = 0;
	sectors[4].mask = PROX_REAR_MASK;
	sectors[4].closeCondition = REAR_CENTER_PROXIMITY;
	sectors[4].farCondition = 0;
	sectors[5].mask = PROX_REAR_LEFT_MASK;
	sectors[5].closeCondition = REAR_LEFT_PROXIMITY;
	sectors[5].farCondition = 0;
	sectors[6].mask = PROX_LEFT_MASK;
	sectors[6].closeCondition = LEFT_PROXIMITY;
	sectors[6].farCondition = 0;
	sectors[7].mask = PROX_FRONT_LEFT_MASK;
	sectors[7].closeCondition = FRONT_LEFT_PROXIMITY;
	sectors[7].farCondition = FRONT_LEFT_FAR_PROXIMITY;
	for (i=0; i<8; i++)
	{
		sectors[i].status = PROX_OFFLINE_MASK;
		if (i < 4)
		{
			sectors[i].toDegree = i * 45 + 23;
			sectors[i].fromDegree = sectors[i].toDegree - 45;
		}
		else if (i > 4)
		{
			sectors[i].toDegree = -((8 - i) * 45 + 23);
			sectors[i].fromDegree = sectors[i].toDegree - 45;
		}
		else
		{
			sectors[i].toDegree = (23 - 180);
			sectors[i].fromDegree = sectors[i].toDegree + 45;
		}
	}

	scanDebugFile = fopen_logfile("scanner");

	s = pthread_create(&thread, NULL, ScannerThread, NULL);
	if (s != 0)
	{
		ERRORPRINT("ScannerThread create - %i", s);
		return -1;
	}
	return 0;
}


//Proximity status call from Behavior Tree
ActionResult_enum proximityStatus(ProxSectorMask_enum _sectors,  ProxStatusMask_enum _status)
{
	int i;
	for (i=0; i<8; i++)
	{
		if (sectors[i].mask & _sectors)
		{
			if (sectors[i].status & _status) return ACTION_SUCCESS;
		}
	}
	return ACTION_FAIL;
}



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
    urg.stop_measurement();

    return 0;
}

void *ScannerThread(void *arg)
{
	DEBUGPRINT("Scanner thread started.");

	while (1)
	{
		while (!urg.open("/dev/ttyACM0", 115200, Urg_driver::Serial))
		{
			ERRORPRINT("scan: unable to open /dev/ttyACM0");
			sleep(5);
		}

		DEBUGPRINT("Product type: %s", urg.product_type());
		DEBUGPRINT("Firmware version: %s", urg.firmware_version());
		DEBUGPRINT("Serial id: %s", urg.serial_id());
		DEBUGPRINT("Status: %s", urg.status());
		DEBUGPRINT("State: %s", urg.state());

		lidar_distance_scan();

		urg.set_scanning_parameter(urg.deg2step(-90), urg.deg2step(+90), 0);
		urg.start_measurement(Urg_driver::Distance, Urg_driver::Infinity_times, 0);

		while (1)
		{
			vector<long> data;
			long time_stamp = 0;
			int i;

			for (i=0; i<8; i++)
			{
				sectors[i].closeCount = sectors[i].farCount = sectors[i].totalCount = 0;
			}

			if (!urg.get_distance(data, &time_stamp))
			{
				ERRORPRINT("Urg_driver::get_distance(): %s", urg.what());

				sleep(1);
				urg.reboot();
				urg.close();
				sleep(5);
				break;
			}
			else
			{
				long min_distance = urg.min_distance();
				long max_distance = urg.max_distance();
				size_t data_n = data.size();
				for (size_t d = 0; d < data_n; ++d) {
					long l = data[d];
					if ((l <= min_distance) || (l >= max_distance)) {
						continue;
					}

					double degree = urg.index2deg(d);

					for (i=0; i<8; i++)
					{
						if ((degree > sectors[i].fromDegree) && (degree < sectors[i].toDegree))
						{
							if (l < (long)(closeRange * 10))
							{
								sectors[i].closeCount++;
							}
							else if (l < (long)(farRange * 10))
							{
								sectors[i].farCount++;
							}
							sectors[i].totalCount++;
						}
					}
				}
				for (i=0; i<8; i++)
				{
					if (sectors[i].closeCount > (sectors[i].totalCount * proxThreshold) && (sectors[i].totalCount > 10))
					{
						sectors[i].status = PROX_CLOSE_MASK | PROX_ACTIVE_MASK;
						ps_set_condition(sectors[i].closeCondition);
						ps_cancel_condition(sectors[i].farCondition);
					}
					else if (sectors[i].closeCount > (sectors[i].totalCount * proxThreshold) && (sectors[i].totalCount > 10))
					{
						sectors[i].status = PROX_FAR_MASK | PROX_ACTIVE_MASK;
						ps_cancel_condition(sectors[i].closeCondition);
						ps_set_condition(sectors[i].farCondition);
					}
					else if (sectors[i].totalCount > 10)
					{
						sectors[i].status = PROX_ACTIVE_MASK;
						ps_cancel_condition(sectors[i].closeCondition);
						ps_cancel_condition(sectors[i].farCondition);
					}
				}
			}
			usleep(250000);
		}
	}
}

