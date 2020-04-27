/*
 * thread_manager.cpp
 *
 *  Created on: 04/07/2014
 *      Author: monfee
 */

#include "thread_manager.h"
#include "wiki_db_split.h"

using namespace std;

//std::mutex thread_manager::mutex_article;
//
//std::mutex thread_manager::mutex_db;
//
//std::condition_variable thread_manager::cond_db;
//
//std::condition_variable thread_manager::cond_article_queue;

int thread_manager::multi_threads_mode = TRUE;
//int task_manager::should_stop = FALSE;

thread_manager::thread_manager() {
	thread_number = POOL_SIZE;
	tm_ptr = this;
}

thread_manager::~thread_manager() {
	for (int i = 0; i < workers.size(); ++i)
		delete workers[i];
}

//void thread_manager::wait_for_article() {
//	std::unique_lock<std::mutex> mlock(mutex_article);
//	cond_article_queue.wait(mlock); // release lock and go join the waiting thread queue
//}

void thread_manager::notify_database_operation_thread() {
	cond_db.notify_all();
}

//void thread_manager::wait_for_processed_article() {
//	std::unique_lock<std::mutex> mlock(mutex_db);
//	cond_db.wait(mlock); // release lock and go join the waiting thread queue
//}

//void thread_manager::start_workers() {
//
//	for (int i = 0; i < thread_number; ++i) {
//		article_operation_thread *aot = new article_operation_thread();
////		threads.insert(aot);
//		workers.push_back(aot);
//		workers[i]->start(i);
//	}
//}

//void thread_manager::stop_workers() {


//	if (workers.size() > 0)
//	for (int i = 0; i < workers.size(); ++i) {
//		add_article_for_parsing(NULL);
//	}

//		for (int i = 0; i < workers.size(); ++i) {
//			workers[i]->join();
//		}
//}

void thread_manager::stop_database_thread() {
	add_parsed_article(NULL);
//	add_parsed_article(NULL);
}

void thread_manager::add_parsed_article(article* art_ptr) {
	std::unique_lock<std::mutex> mlock(mutex_db);

	int size = article_queue_parsed.size();
	article_queue_parsed.push_back(art_ptr);

//	mlock.unlock();     // unlock before notificiation to minimize mutex contention
//	mlock.lock();

	notify_database_operation_thread(); //.notify_one(); // notify one waiting thread
}

article* thread_manager::get_parsed_article() {
	std::unique_lock<std::mutex> mlock(mutex_db);
	if (article_queue_parsed.empty()) // check condition to be safe against spurious wakes
	{
		cond_db.wait(mlock);
//		wait_for_processed_article();
	}

	article *art_ptr = article_queue_parsed.front();

	article_queue_parsed.pop_front();

	return art_ptr;
}

void thread_manager::signoff_db_thread(std::thread::id id) {
	db_threads.erase(id);
	cond_thread_stop.notify_all();
}

void thread_manager::signin_db_thread(std::thread::id id) {
	db_threads.insert(id);
}

void thread_manager::stop() {
	fprintf(stderr, "still need process %d files before we can stop, ", article_queue_to_parse.size());
	fprintf(stderr, "%d files in database queue\n", article_queue_parsed.size());

	task_manager::stop();

	if (db_threads.size() > 0) {
		std::unique_lock<std::mutex> mlock(mutex_thread_stop);
		stop_database_thread();
		cond_thread_stop.wait(mlock);

		wiki_db_split::instance().join();
	}
}

void thread_manager::start() {
	runnable::set_run_func((void (*)(int, ...))&article_operation_thread::process);

	this->start_workers();

	wiki_db_split::instance().start(0);
//	threads.insert(&wiki_db_split::instance());
}

void thread_manager::check_if_db_ready() {
	std::unique_lock<std::mutex> mlock(mutex_db);
	if (!wiki_db_split::instance().get_current_db()->is_ready())
		cond_main.wait(mlock);
}

void thread_manager::notify_main_thread_db_ready() {
	std::unique_lock<std::mutex> mlock(mutex_db);
	cond_main.notify_all();
}

int thread_manager::has_file_updated(long id) {
	std::unique_lock<std::mutex> mlock(mutex_article);
	if (wiki_db_split::instance().get_current_db()->get_article_table()->get_last_update() == id) {
		cond_main.wait(mlock);
	}
}
