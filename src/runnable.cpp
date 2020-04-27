/*
 * runnable.cpp
 *
 *  Created on: 06/07/2014
 *      Author: monfee
 */

#include "runnable.h"

void (*runnable::run_func)(int id, ...) = NULL;

runnable::runnable() {
	task = NULL;
}

runnable::~runnable() {
	if (task) {
//		task->detach();
		delete task;
	}
}

void runnable::operator()() const {
	runnable::run(-1);
}

void runnable::run(int id) {
	(*run_func)(id);
}

void runnable::stop() {

}

//void runnable::start(int id) {
//	task = new std::thread(&runnable::run, id);
//}

void runnable::join() {
	if (task->joinable())
		task->join();
}

void runnable::set_run_func(void (*func)(int id, ...)) {
	run_func = func;
}

void runnable::start(int id) {
	task = new std::thread(run_func, id);
}
