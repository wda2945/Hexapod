/*!
  \file
  \brief Gets computer timestamp
  \author Satofumi KAMIMURA

  $Id: ticks.cpp,v c5747add6615 2015/05/07 03:18:34 alexandr $
*/

#include "ticks.h"
#include <time.h>

void gettime(struct timespec *ts)
{
  clock_gettime(CLOCK_REALTIME, ts);
}


long qrk::ticks(void)
{
    static bool is_initialized = false;
    static struct timespec first_spec;
    struct timespec current_spec;
    long msec_time;

    if (!is_initialized) {
        gettime(&first_spec);
        is_initialized = true;
    }
    gettime(&current_spec);
    msec_time =
        (current_spec.tv_sec - first_spec.tv_sec) * 1000
        + (current_spec.tv_nsec - first_spec.tv_nsec) / 1000000;

    return msec_time;
}
