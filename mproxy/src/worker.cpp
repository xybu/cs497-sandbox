/**
 * worker.cpp
 * Worker is used to handle tasks whose order is essential.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#include "global.h"
#include "worker.h"
#include "task.h"

Worker::Worker(int id) {
	can_run = false;
	wid = id;
	can_run = true;
}

Worker::~Worker() {
	delete th;
	dprintf("Worker%d: stopped.\n", wid);
}

void Worker::start() {
	th = new std::thread(&Worker::run, this);
}

void Worker::run() {
	dprintf("Worker%d: started.\n", wid);
	while (can_run) {
		Task *t = task_queue->dequeue();
		if (t == NULL) {
			if (!can_run) {
				dprintf("Worker%d: break loop.\n", wid);
				break;
			}
			dprintf("Umm... Worker can run but got NULL task.\n");
		} else {
			dprintf("Worker%d: fetched a task.\n", wid);
			t->ready.lock();
			dprintf("Worker%d: running a task.\n", wid);
			(*(t->action))(t->arg);
			dprintf("Worker%d: completed a task.\n", wid);
			delete t;
		}
	}
}
