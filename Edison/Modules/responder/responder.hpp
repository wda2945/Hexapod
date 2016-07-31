/*
 * responder.hpp
 *
 *  Created on: Aug 7, 2015
 *      Author: martin
 */

#ifndef BROKER_RESPONDER_HPP_
#define BROKER_RESPONDER_HPP_

#include "hexapod.h"

//responder
int ResponderInit();

void ResponderProcessMessage(psMessage_t *msg);

#endif /* BROKER_RESPONDER_HPP_ */
