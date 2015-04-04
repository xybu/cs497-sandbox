/**
 * worker.cpp
 * Worker is used to handle tasks whose order is essential.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#include <cstdio>
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
	debug("Worker%d: stopped.\n", wid);
}

void Worker::start() {
	th = new std::thread(&Worker::run, this);
}

void Worker::run() {
	debug("Worker%d: started.\n", wid);
	while (can_run) {
		Task *t = task_queue->dequeue();
		if (t == NULL) {
			if (!can_run) {
				debug("Worker%d: break loop.\n", wid);
				break;
			}
			debug("Umm... Worker can run but got NULL task.\n");
		} else {
			debug("Worker%d: fetched a task.\n", wid);
			t->ready.lock();
			debug("Worker%d: running a task.\n", wid);
			(*(t->action))(t->arg);
			debug("Worker%d: completed a task.\n", wid);
			delete t;
		}
	}
}
