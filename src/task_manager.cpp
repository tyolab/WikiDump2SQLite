/*
 * task_manager.cpp
 *
 *  Created on: 5 Aug 2015
 *      Author: monfee
 */

#include "task_manager.h"

#include <algorithm>

task_manager::task_manager() {
	this->thread_number = 5;
	tm_ptr = this;
	should_stop = 0;
}

task_manager::~task_manager() {

}

void task_manager::signin(int id) {
	std::unique_lock<std::mutex> mlock(mutex_thread_stop);
	threads.push_back(id);
}

void task_manager::signoff(int id) {
	std::unique_lock<std::mutex> mlock(mutex_thread_stop);
	threads.erase(std::remove(threads.begin(), threads.end(), id), threads.end());

//	if (threads.size() == 0)
	cond_thread_stop.notify_all();
}

void task_manager::stop() {
	if (threads.size() > 0) {
		stop_workers();

		while (threads.size() > 0) {
			std::unique_lock<std::mutex> mlock(mutex_thread_stop);
			fprintf(stderr, "waiting for %d child threads to exit.\n", threads.size());
			cond_thread_stop.wait(mlock);
		}

		for (int i = 0; i < workers.size(); ++i)
			workers[i]->join();

		fprintf(stderr, "all running threads stopped.\n");
	}
}

void task_manager::start() {
	if (!runnable::is_run_func_defined())
		runnable::set_run_func((void (*)(int, ...))task_thread::process);

//	if (!runnable::is_run_func_defined())
//		throw std::runtime_error("thread entry function is not defined yet!");

	start_workers();
}

void task_manager::start_workers() {
//	runnable::set_run_func();
//	task_thread::set_task_manager(this);

	for (int i = 0; i < thread_number; ++i) {
		task_thread *aot = new task_thread();
//		threads.insert(aot);
		workers.push_back(aot);
		workers[i]->start(i, tm_ptr);
	}
}

void task_manager::stop_workers() {
//	fprintf(stderr, "still need process %d files before we can stop, ", article_queue_to_parse.size());
//	fprintf(stderr, "%d files in database queue\n", article_queue_parsed.size());

//	if (workers.size() > 0)
	for (int i = 0; i < workers.size(); ++i) {
		add_article_for_parsing(NULL);
	}

}

void task_manager::notify_article_operation_thread() {
	cond_article_queue.notify_one();
}

void task_manager::add_article_for_parsing(article* art_ptr) {
	std::unique_lock<std::mutex> mlock(mutex_article);

	article_queue_to_parse.push_back(art_ptr);

	//	mlock.unlock();     // unlock before notificiation to minimize mutex contention
	//	mlock.lock();
	//	if (article_queue_to_parse.size() > 10)
	//		cond.notify_all();
	//	else
	notify_article_operation_thread(); //.notify_one(); // notify one waiting thread
}


article* task_manager::get_aticle_to_parse() {
	std::unique_lock<std::mutex> mlock(mutex_article);
	// mutex scope lock
	int size = article_queue_to_parse.size();
	while (article_queue_to_parse.empty()) // check condition to be safe against spurious wakes
	{
		cond_main.notify_all();
		cond_article_queue.wait(mlock); // release lock and go join the waiting thread queue
	}

	article *art_ptr = article_queue_to_parse.front();

	article_queue_to_parse.pop_front();

	if (art_ptr != NULL && !art_ptr->get_in_fix_mode() && article_queue_to_parse.size() < QUEUE_SIZE)
		cond_main.notify_all();

	return art_ptr;
}


int task_manager::is_full(int count, int max) {
	std::unique_lock<std::mutex> mlock(mutex_article);
	int size = article_queue_to_parse.size();

	if (size >= QUEUE_SIZE || (max > -1 && count > max)) {
		cond_main.wait(mlock);
	}

	return 0;
}

