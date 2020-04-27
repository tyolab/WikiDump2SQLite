/*
 * task_thread.cpp
 *
 *  Created on: 5 Aug 2015
 *      Author: monfee
 */

#include "task_thread.h"
#include "article.h"
#include "compression.h"
#include "xml.h"
#include "task_manager.h"
#include "wiki_api.h"

#include <iostream>

using namespace std;

void (*task_thread::task_func)(article* art_ptr) = NULL;

int task_thread::queue_size = task_thread::QUEUE_SIZE;

//int task_thread::serving = 0;

int task_thread::count = 0;

int task_thread::article_parsing_report_number = 100;

//task_manager *task_thread::tm_ptr = NULL;

task_thread::task_thread() : runnable::runnable() {
	id = -1;
}

task_thread::~task_thread() {

}

int task_thread::get_id() const {
	return id;
}

void task_thread::set_id(int id) {
	this->id = id;
}

void task_thread::process(int thread_id, task_manager *tm_ptr) {
	cerr << "start article parsing thread #" << thread_id/*std::this_thread::get_id()*/ << endl << flush;

	article *art_ptr = NULL;
	std::thread::id id = std::this_thread::get_id();
	tm_ptr->signin(thread_id);

	if (is_task_func_defined()) {
		while ((art_ptr = tm_ptr->get_aticle_to_parse()) != NULL) {
			++count;

	//		/**
	//		 * DEBUG
	//		 */
	//		for (int i = 1; i < 10000; ++i) {
	//			i + 100;
	//		}
	//		tm_ptr->add_parsed_article(art_ptr);

			(*task_func)(art_ptr);

			// delete the article in the database thread
			// after it is inserted into table
			//delete art_ptr;

			if (count % 500 == 0 && thread_id == 0)
				fprintf(stderr, "parsed %d files.\n", count);

			/**
			 * parse it or get it form server
			 */
	//		std::string article = wiki_api::get_wiki_page(art_ptr->get_title().c_str());
	//
	//		if (article.find("\"error\":") != std::string::npos) {
	//			std::cerr << "ERROR:" << art_ptr->get_article_id() << ":" << art_ptr->get_title() << std::endl;
	//		}
	//		else {
	//			void *blob = malloc(MIN_COMPRESSION_BUFFER_SIZE(article.length()));
	//			int blob_len = gzip_compress(article.c_str(), (char *)blob);
	//
	//			article_table_ptr->update_content(art_ptr->get_article_id(), blob, blob_len);

	//			free(blob);
	//			delete art_ptr;
	//		}

	//		std::this_thread::yield();
	//		sleep(1000);
			/*
			 * INSERT INTO TABLE
			 */
	//			table.insert(&art);
	//				art.set_article("I am not a superman");
	//		art.set_abstract("this is a long abstract, longer than the article.");
		}

		fprintf(stderr, "received stop signal, article parsing thread #%d existing...\n", thread_id);
	}
	else {
		if (thread_id <= 0)
			fprintf(stderr, "task function is not defined yet.\n");
	}

	tm_ptr->signoff(thread_id);
}

void task_thread::start(int id, task_manager *tm_ptr) {
	task = new std::thread(run_func, id, tm_ptr);
}
