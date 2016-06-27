/*
 * dancer.cpp
 *
 *      Author: martin
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

#include "dancer/dancer.hpp"

int DancerInit(){return 0;}

void DancerProcessMessage(psMessage_t *msg){}
