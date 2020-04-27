/*
 * article_operation_thread.cpp
 *
 *  Created on: 02/07/2014
 *      Author: monfee
 */

#include "article_operation_thread.h"
#include "article.h"
#include "compression.h"
#include "xml.h"
#include "thread_manager.h"
#include "wiki_db_split.h"
#include "wiki_api.h"

#include <stdlib.h>

#include <iostream>

//table_article *article_operation_thread::article_table_ptr = NULL;
wiki_db *article_operation_thread::wiki_db_ptr = NULL;

//std::deque<article *> article_operation_thread::article_queue;
//void (*article_operation_thread::run_func)() = &article_operation_thread::process;

using namespace std;

article_operation_thread::article_operation_thread() {
	id = -1;
}

article_operation_thread::~article_operation_thread() {

}

void article_operation_thread::process(int thread_id) {
	cerr << "start article parsing thread #" << thread_id/*std::this_thread::get_id()*/ << endl << flush;
	wiki_api api;
	article *art_ptr = NULL;
	std::thread::id id = std::this_thread::get_id();
	thread_manager::instance().signin(thread_id);
	while ((art_ptr = thread_manager::instance().get_aticle_to_parse()) != NULL) {
		++count;

//		/**
//		 * DEBUG
//		 */
//		for (int i = 1; i < 10000; ++i) {
//			i + 100;
//		}
//		thread_manager::instance().add_parsed_article(art_ptr);

		process_article(art_ptr);

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

	thread_manager::instance().signoff(thread_id);
}

void article_operation_thread::process_article(article* art_ptr) {
	wiki_api api;

	if (++count % article_parsing_report_number == 0)
		printf("# %d: %s\n", count, art_ptr->get_title().c_str());

	std::string article = api.get_wiki_page(art_ptr->get_title().c_str());

	if (article.length() == 0 || article.find("\"Error\":") != std::string::npos || article.find("\{\"error") != std::string::npos) {
		std::cerr << "ERROR:" << art_ptr->get_article_id() << ":" << art_ptr->get_title() << std::endl;
		art_ptr->set_result(0);
	}
	else {
		void *blob = malloc(MIN_COMPRESSION_BUFFER_SIZE(article.length()));
		int blob_len = gzip_compress(article.c_str(), (char *)blob);
		art_ptr->set_article((char *)blob);
		art_ptr->set_article_len(blob_len);

//		wiki_db::instance().add_task(art_ptr);
		thread_manager::instance().add_parsed_article(art_ptr);
	}
}

void article_operation_thread::process_redirect(article* art_ptr) {
	/*
	 * THIS WILL BE RUN ONLY ONCE TO DEAL WITH REDIRECT
	 */
	const char *start = NULL;
	if ((start = strstr(art_ptr->get_article(), "<redirect")) != NULL) {
		++count;
		if (count % 100 == 0)
			printf("# %d: %s\n", count, art_ptr->get_title().c_str());

		art_ptr->set_redirect(1);

		char *redir = between(start, "title=\"", "\"", 0);


		if (redir) {
			// if it is redirect we are not gonna compress it
			wiki_db_split::instance().get_current_db()->get_article_table()->update_content(art_ptr->get_article_id(), redir, strlen(redir), 1);

			delete [] redir;
		}
	}

	delete art_ptr;
}

