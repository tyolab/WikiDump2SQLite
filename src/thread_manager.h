/*
 * thread_manager.h
 *
 *  Created on: 04/07/2014
 *      Author: monfee
 */

#ifndef THREAD_MANAGER_H_
#define THREAD_MANAGER_H_

#include "pattern_singleton.h"
#include "task_manager.h"
#include "article_operation_thread.h"

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef TRUE
	#define TRUE (!FALSE)
#endif


class article_operation_thread;

class thread_manager : public pattern_singleton<thread_manager>, public task_manager  {
public:
	static int multi_threads_mode;

private:

	std::mutex mutex_db;

	std::condition_variable cond_db;

	std::vector<article_operation_thread *> workers;

	std::deque<article *> article_queue_parsed;

	std::set<std::thread::id> db_threads;

public:
	thread_manager();
	virtual ~thread_manager();

//	void wait_for_article();

	void notify_database_operation_thread();

//	void wait_for_processed_article();

//	virtual void start_workers();

//	virtual void stop_workers();

	void stop_database_thread();

	void add_parsed_article(article *art_ptr);

	article *get_parsed_article();

	void signoff_db_thread(std::thread::id);

	void signin_db_thread(std::thread::id);

	void check_if_db_ready();

	void notify_main_thread_db_ready();

	int has_file_updated(long id);

	void stop();

	void start();
};

#endif /* THREAD_MANAGER_H_ */
