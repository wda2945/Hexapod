/*
 * brokerQ.h
 *
 * Header for broker internal message queues
 *
 *  Created on: Jul 8, 2014
 *      Author: martin
 */

#ifndef BROKERQ_H_
#define BROKERQ_H_

#include "pubsubdata.h"
#include <stdio.h>
#include "pthread.h"


//queue item struct
//just a message and a next pointer
typedef struct {
	psMessage_t msg;
	void *next;
} BrokerQueueEntry_t;

//queue struct - allocated and kept by the owning subsystem
//list head and tail, plus mutex and condition variable for signalling
typedef struct {
	BrokerQueueEntry_t *qHead, *qTail;
	pthread_mutex_t mtx;	//access control
	pthread_cond_t cond;	//signals item in previously empty queue
} BrokerQueue_t;
#define BROKER_Q_INITIALIZER {NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER}

int BrokerQueueInit(int pre);							//one init to pre-allocate shared pool of queue entries

int CopyMessageToQ(BrokerQueue_t *q, psMessage_t *msg);				//appends to queue

void AppendQueueEntry(BrokerQueue_t *q, BrokerQueueEntry_t *e);		//appends an allocated message q entry

psMessage_t *GetNextMessage(BrokerQueue_t *q);			//waits if empty, returns pointer (call DoneWithMessage!)
bool isQueueEmpty(BrokerQueue_t *q);

void DoneWithMessage(psMessage_t *msg);					//when done with message Q entry -> freelist

BrokerQueueEntry_t *GetFreeEntry();						//new broker q entry <- freelist

#endif /* BROKER_H_ */
