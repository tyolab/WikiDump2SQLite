/*
 * runnable.h
 *
 *  Created on: 06/07/2014
 *      Author: monfee
 */

#ifndef RUNNABLE_H_
#define RUNNABLE_H_

#include <thread>

class runnable {

protected:
	std::thread *task;

	static void (*run_func)(int id, ...);

public:
	runnable();
	virtual ~runnable();

	void operator()() const;

	static void run(int id);

	void stop();

	void join();

	static void set_run_func(void (*func)(int id, ...));

	static bool is_run_func_defined() { return NULL != run_func; }

	std::thread *get_thread() { return task; };

	void start(int id);

};

#endif /* RUNNABLE_H_ */
