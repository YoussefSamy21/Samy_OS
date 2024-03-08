/*
 * Samy_OS_FIFO.h
 *
 *  Created on: Feb 23, 2024
 *      Author: Youssef Samy
 */

#ifndef SAMY_OS_FIFO_H_
#define SAMY_OS_FIFO_H_

#include "Scheduler.h"


#define element_type    Tasks_ref*

typedef struct
{
	uint32 length;
	uint32 count;
	element_type *base;
	element_type *head;
	element_type *tail;

}FIFO_BUFF;

typedef enum
{
	QUEUE_FULL,             // FIFO BUFFER is Full
	QUEUE_EMPTY,           // FIFO BUFFER is Empty
	QUEUE_NO_ERROR,        // No Error from calling the API
	QUEUE_NULL            // FIFO Buffer does not exist
}FIFO_STATUS;

FIFO_STATUS QUEUE_init(FIFO_BUFF *buffer , element_type *ptr_arr , uint32 length);
FIFO_STATUS Enqueue_item(FIFO_BUFF *buffer , element_type item);
FIFO_STATUS Dequeue_item(FIFO_BUFF *buffer , element_type *ptr_item);
FIFO_STATUS Is_Queue_Full(FIFO_BUFF *buffer);
FIFO_STATUS Is_Queue_Empty(FIFO_BUFF *buffer);



#endif /* SAMY_OS_FIFO_H_ */
