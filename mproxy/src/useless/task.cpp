/**
 * task.h
 * Defines actions on task queue.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#include "global.h"
#include "task.h"

TaskQueue *task_queue;

TaskQueue::TaskQueue() {
	status = STATUS_ERR;
	if (sem_init(&count, SEM_PRIVATE, 0) < 0) {
		pperror("sem_init");
		throw status;
	}
	status = STATUS_OK;
}

TaskQueue::~TaskQueue() {
	if (sem_destroy(&count)) {
		pperror("sem_destroy");
	}
}

void TaskQueue::enqueue(Task *task) {
	mut.lock();
	queue.push_back(task);
	sem_post(&count);
	mut.unlock();
}

Task *TaskQueue::dequeue() {
	Task *t = NULL;

	sem_wait(&count);

	mut.lock();
	if (queue.size() == 0) {
		mut.unlock();
		return NULL;	
	}

	t = queue.front();
	queue.pop_front();
	mut.unlock();
	return t;
}

void TaskQueue::increment_count(int c) {
	for (int i = 0; i < c; ++i) sem_post(&count);
}
