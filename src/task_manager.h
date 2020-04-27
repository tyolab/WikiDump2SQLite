/*
 * task_manager.h
 *
 *  Created on: 5 Aug 2015
 *      Author: monfee
 */

#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

#include <mutex>
#include <condition_variable>
#include <vector>
#include <set>
#include <deque>

#include "pattern_singleton.h"
#include "task_thread.h"
#include "article.h"

class task_manager {

public:
	bool should_stop;

	static const int POOL_SIZE = 18;
	static const int QUEUE_SIZE = 60000;

	static const int THREAD_PASING_DONE = 1;
	static const int THREAD_DATABASE_DONE = 2;

protected:
	std::mutex mutex_article;

	std::mutex mutex_thread_stop;

	std::condition_variable cond_article_queue;

	std::condition_variable cond_thread_stop;

	std::condition_variable cond_main;

	std::vector<int> threads;

	int thread_number;

	std::vector<task_thread *> workers;

	std::deque<article *> article_queue_to_parse;

	task_manager* tm_ptr;

public:
	task_manager();
	virtual ~task_manager();

	void signin(int id);

	void signoff(int id);

	virtual void stop();

	virtual void start();

	void set_thread_number(int num) { thread_number = num; }

	virtual void start_workers();

	virtual void stop_workers();

	void notify_article_operation_thread();

	virtual void add_article_for_parsing(article *art_ptr);

	article *get_aticle_to_parse();

	int is_full(int count, int max = -1);

	int should_we_stop() { return should_stop; }
	void we_should_stop(int stop_or_not) { should_stop = stop_or_not; }

};

#endif /* TASK_MANAGER_H_ */
