/*
 * responder.h
 *
 *  Created on: Aug 7, 2015
 *      Author: martin
 */

#ifndef BROKER_RESPONDER_H_
#define BROKER_RESPONDER_H_

#include "pubsubdata.h"

//responder
int ResponderInit();

void ResponderProcessMessage(psMessage_t *msg);

#endif /* BROKER_RESPONDER_H_ */
