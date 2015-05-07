/**
 * task.h
 * Declares task datatype.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#ifndef _TASK_H

#define _TASK_H

#include <semaphore.h>
#include <mutex>
#include <list>

typedef struct task {
	std::mutex ready;
	void (*action)(void *);
	void *arg;
} Task;

// a generic, unbounded, thread-safe queue
class TaskQueue {
	std::mutex mut;
	std::list<Task *> queue;
public:
	sem_t count;
	int status;
	TaskQueue();
	~TaskQueue();
	void enqueue(Task *task);
	Task *dequeue();
	void increment_count(int);
};

extern TaskQueue *task_queue;

#endif
