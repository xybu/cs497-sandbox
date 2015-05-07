/**
 * delayer.h
 * Delay queue and worker declarations.
 * 
 * @author Xiangyu Bu
 */

#ifndef __DELAYER_H

#define __DELAYER_H

#include <pthread.h>
#include <semaphore.h>
#include <event2/event.h>
#include <event2/util.h>
#include "stream.h"

// when mode is set CLEAR, worker on the queue should stop
// when mode is set SEND, worker continuously sleeps and sends
typedef unsigned int delay_mode_t;

#define DELAY_MODE_CLEAR 0
#define DELAY_MODE_SEND	1

// node struct of delay queue (singly linked list)
typedef struct delay_node {
	int sleep_time;
	pthread_mutex_t *write_mutex;
	struct evbuffer	*write_ev_buf;
	struct bufferevent *write_ev_event;
	Stream *message;
	delay_node_t *next_node;
} delay_node_t;

// list metadata of delay queue (singly linked list)
typedef struct delay_queue {
	delay_mode_t mode;
	delay_node_t *head;
	delay_node_t *tail;
	pthread_mutex_t mutex;
	sem_t semaphre;
} delay_queue_t;

extern delay_queue_t *glob_delay_queue;

int delay_queue_init(delay_queue_t *q);
int delay_queue_enqueue(delay_queue_t *q, delay_node_t *node);
delay_node_t *delay_queue_dequeue(delay_queue_t *q);
void delay_queue_free(delay_queue_t *q);
void delay_node_free(delay_node_t *n);

#endif
