/**
 * worker.h
 * Declares worker datatype.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#ifndef _WORKER_H

#define _WORKER_H

#include <thread>

#define WORKER_UNDEF_ID	(-1)

class Worker {
public:
	int wid;
	std::thread *th;
	bool can_run;
	Worker(int id=WORKER_UNDEF_ID);
	~Worker();
	void start();
	void run();
};

#endif
