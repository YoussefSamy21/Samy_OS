/*
 * Samy_OS_FIFO.c
 *
 *  Created on: Feb 23, 2024
 *      Author: Youssef Samy
 */

#include "Samy_OS_FIFO.h"

FIFO_STATUS QUEUE_init(FIFO_BUFF *buffer , element_type *ptr_arr , uint32 length)
{
	if(!ptr_arr)
	{
		return QUEUE_NULL;
	}
	buffer->base = ptr_arr;
	buffer->head = buffer->base;
	buffer->tail = buffer->base;

	buffer->count = 0;
	buffer->length = length;

	return QUEUE_NO_ERROR;
}


FIFO_STATUS Enqueue_item(FIFO_BUFF *buffer , element_type item)
{

	// Check if Buffer Exists
	if(!buffer->base || !buffer->length)
	{
		return QUEUE_NULL;
	}

	// Check if Buffer is Full
	if((buffer->head == buffer->tail) && (buffer->count == buffer->length))
	{
		return QUEUE_FULL;
	}

	*(buffer->tail) = item;
	buffer->count++;

	// Check for Circular FIFO
	if(buffer->tail == (((uint32)buffer->base + (4*buffer->length))- 4 ) )
	{
		buffer->tail = buffer->base;
	}
	else
	{
		buffer->tail++;
	}

	return QUEUE_NO_ERROR;
}


FIFO_STATUS Dequeue_item(FIFO_BUFF *buffer , element_type *ptr_item)
{

	// Check if Buffer Exists
	if(!buffer->base || !buffer->length)
	{
		return QUEUE_NULL;
	}

	// Check if Buffer is Empty
	if(buffer->head == buffer->tail)
	{
		return QUEUE_EMPTY;
	}

	*ptr_item = *(buffer->head);
	buffer->count--;

	// Check for Circular FIFO
	if(buffer->head == (((uint32)buffer->base + (4*buffer->length )) - 4 ))
	{
		buffer->head = buffer->base;
	}
	else
	{
		buffer->head++;
	}

	return QUEUE_NO_ERROR;
}


FIFO_STATUS Is_Queue_Full(FIFO_BUFF *buffer)
{
	FIFO_STATUS status = QUEUE_NO_ERROR;

	if(buffer->count == buffer->length)
	{
		status = QUEUE_FULL;
	}
	return status;
}


FIFO_STATUS Is_Queue_Empty(FIFO_BUFF *buffer)
{
	FIFO_STATUS status = QUEUE_NO_ERROR;

	if(buffer->count == 0) // or it can also be if(buffer->head == buffer->tail)
	{
		status = QUEUE_EMPTY;
	}

	return status;
}
