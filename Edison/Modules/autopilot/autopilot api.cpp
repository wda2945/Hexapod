/*
 ============================================================================
 Name        : autopilot_api.c
 Author      : Martin
 Version     :
 Copyright   : (c) 2015 Martin Lane-Smith
 Description : Receives MOVE commands
 ============================================================================
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stropts.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <errno.h>

#include "hexapod.h"

#include "navigator/navigator.hpp"
#include "autopilot.hpp"
#include "autopilot_common.h"
#include "waypoints.h"


