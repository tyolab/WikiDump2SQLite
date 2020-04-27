/*
 * task_thread.h
 *
 *  Created on: 5 Aug 2015
 *      Author: monfee
 */

#ifndef TASK_THREAD_H_
#define TASK_THREAD_H_

#include "runnable.h"

class task_manager;
class article;

class task_thread : public runnable {
public:
	static const int QUEUE_SIZE = 500;

	static int article_parsing_report_number;

protected:
	int id;

//	static task_manager *tm_ptr;

	static int count;

//	static std::deque<article *> article_queue;

	static int queue_size;

	static void (*task_func)(article* art_ptr);

public:
	task_thread();
	virtual ~task_thread();

	int get_id() const;

	void set_id(int id);

	static void set_task_func(void (*func)(article* art_ptr)) { task_func = func; }

	static bool is_task_func_defined() { return NULL != run_func; }

	static void process(int thread_id, task_manager *tm_ptr);

//	static void set_task_manager(task_manager *tm_ptr) { task_thread::tm_ptr = tm_ptr; }

	static int get_count() { return count; }

	void set_queue_size(int size) { this->queue_size = size; }

	void start(int id, task_manager *tm_ptr);
};

#endif /* TASK_THREAD_H_ */
