/*
 * arbotix.h
 *
 *      Author: martin
 */

#ifndef ARBOTIX_H
#define ARBOTIX_H

//arbotix task
int ArbotixInit();

void ArbotixProcessMessage(psMessage_t *msg);

#endif
