/*
 * article_operation_thread.h
 *
 *  Created on: 02/07/2014
 *      Author: monfee
 */

#ifndef ARTICLE_OPERATION_THREAD_H_
#define ARTICLE_OPERATION_THREAD_H_

#include "wiki_db.h"
//#include "thread_manager.h"

#include "task_thread.h"

class article;
class wiki_api;
class thread_manager;

class article_operation_thread : public task_thread {
public:
	static wiki_db *wiki_db_ptr;

private:

//	static thread_manager *thread_manager_ptr;

public:
	article_operation_thread();
	virtual ~article_operation_thread();

	static void process(int thread_id);

//	static void add_task(article *art_ptr);
//
//	static article *get_task();

	static void process_article(article *art_ptr);

	static void process_redirect(article *art_ptr);

};

#endif /* ARTICLE_OPERATION_THREAD_H_ */
