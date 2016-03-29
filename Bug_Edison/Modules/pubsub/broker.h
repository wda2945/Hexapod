/*
 * broker.h
 *
 *  Created on: Aug 7, 2015
 *      Author: martin
 */

#ifndef BROKER_BROKER_H_
#define BROKER_BROKER_H_

#include <pthread.h>
#include "pubsub/brokerQ.h"

//broker input queue
extern BrokerQueue_t brokerQueue;

int BrokerInit();

#endif /* BROKER_BROKER_H_ */
