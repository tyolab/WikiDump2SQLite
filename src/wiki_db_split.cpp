/*
 * wiki_db_split.cpp
 *
 *  Created on: 22 Sep 2014
 *      Author: monfee
 */

#include "wiki_db_split.h"
#include "string_utils.h"
#include "sys_file.h"
#include "wiki_db.h"
#include "thread_manager.h"
#include "article.h"

#include <iostream>
#include <string>

int wiki_db_split::milestone = 20000;
const char *wiki_db_split::BASE_NAME_DEFAULT = "wikipedia.db";

using namespace std;

wiki_db_split::wiki_db_split() {
	to_split = 1; // to split db by default
	base_name = BASE_NAME_DEFAULT;
	upto = 0;
	main = 0;
	current = 0;
}

wiki_db_split::~wiki_db_split() {
	for (int i = 0; i < dbs.size(); ++i)
		delete dbs[i];
}

void wiki_db_split::process(int thread_id) {
	cerr << "start database thread #" << thread_id/*std::this_thread::get_id()*/ << endl;

	std::thread::id id = std::this_thread::get_id();

	thread_manager::instance().signin_db_thread(id);

	article *art_ptr = NULL;
	int count = 0;
	wiki_db_split& dbs = instance();

	wiki_db *db_ptr = dbs.get_current_db(); // main db
	db_ptr->set_in_memory(1);
	db_ptr->open();
	db_ptr->begin_transaction();

	thread_manager::instance().notify_main_thread_db_ready();

//	std::thread::id id = std::this_thread::get_id();
//	thread_manager::instance().signin(id);

	int previous_id = 0;

	while ((art_ptr = thread_manager::instance().get_parsed_article()) != NULL) {
		++count;

		previous_id = art_ptr->get_id();
//		/**
//		 * DEBUG
//		 */
//		for (int i = 0; i < 1000; ++i) {
//			i + 1;
//		}
		db_ptr->update_article(art_ptr);

		if (count % 50000 == 0)
			fprintf(stderr, "saved %d files into database\n", count);


//#ifdef RELEASE
//		static int milestone = 15000;
		if (milestone > 0 && count % milestone == 0) {
			fprintf(stderr, "milestone #%d reached, write database to file.\n", count/milestone);
			db_ptr->set_something_changed(TRUE);

			db_ptr->get_history_table()->save(art_ptr->get_id(), art_ptr->get_article_id(), art_ptr->get_title().c_str());

			db_ptr->end_transaction();

			db_ptr->write_to_disk();

			db_ptr->begin_transaction();
		}
//#endif

		if (art_ptr->get_in_fix_mode())
			thread_manager::instance().notify_main_thread_db_ready();

		dbs.check_if_switching_db_needed(art_ptr);

		delete art_ptr;

		db_ptr = dbs.get_current_db();
		if (db_ptr == NULL)
			break;
	}

//	if (article_db->get_index() > 0) {
//		article_db->end_transaction();
//		article_db->write_to_disk();
//	}

	fprintf(stderr, "received stop signal, database thread #%d existing...\n", thread_id);

//	thread_manager::instance().signoff(id);
	if (db_ptr != NULL) {
		db_ptr->get_history_table()->save(previous_id, -1, "unknown");

		db_ptr->end_transaction();

		if (count > 0)
			db_ptr->write_to_disk();

		db_ptr->close();
	}

	thread_manager::instance().signoff_db_thread(id);
}

void wiki_db_split::start(int id) {
	task = new std::thread(&wiki_db_split::process, id);
}

void wiki_db_split::load_db() {
	create_main_db();

	string db_name = increase_file_num(base_name);
	while (sys_file::exist(db_name.c_str())) {
		wiki_db *db_ptr = new wiki_db(db_name.c_str());
		dbs.push_back(db_ptr);
		db_ptr->set_index(dbs.size() - 1);
		string pre_db_name = db_name;
		db_name = increase_file_num(pre_db_name.c_str());
//		db_name = increase_file_num(db_ptr->get_db_name());
	}

	for (int i = 0; i < dbs.size(); ++i) {
		dbs[i]->open();
		dbs[i]->detect();

		if (upto != -1) {
			int last = dbs[i]->get_history_table()->get_last_update();
			if (last > upto) {
				upto = last;
				current = dbs[i];
			}
		}
	}

//	main->open();
//	main->detect();
//	upto = main->get_history_table()->get_last_update();
//	where_to_stop = main->get_article_table()->get_max_article_id();
//	main->close();
//	current = main;

	for (int i = 0; i < dbs.size(); ++i) {

		int last_update = dbs[i]->get_article_table()->get_last_update();

		if (last_update == 0) {
			if (dbs[i]->get_how_many_left() > 0) {
				upto = dbs[i]->get_min();
				current = dbs[i];
				break;
			}
//			else (dbs[i]->get_max() > upto) {
//				wiki_db *db_ptr = get_next_db();
//				if (db_ptr)
//					upto = db_ptr->get_min();
//			}
			continue;
		}
		else if (last_update >= upto) {
			upto = last_update;
			current = dbs[i];
		}
		else if (upto > dbs[i]->get_max()) {
			continue;
		}
		break;
	}

	if (!current)
		current = main;

	cerr << "starting with: " << upto << ", " << current->get_db_name() << endl;

	where_to_stop = dbs[dbs.size() - 1]->get_article_table()->get_max_article_id();

	for (int i = 0; i < dbs.size(); ++i) {
		dbs[i]->close();
	}
}

int wiki_db_split::where_we_are_up_to() {
	return upto;
}

int wiki_db_split::where_we_stop() {
	return where_to_stop;
}

wiki_db* wiki_db_split::get_next_db() {
	int next = current->get_index() + 1;
	if (next < dbs.size())
		return current = dbs[next];

	return 0;
}

void wiki_db_split::create_new_db() {
	string pre_db_name = current->get_db_name();
	string new_db_name = increase_file_num(pre_db_name.c_str());
	cerr << "the new db name :" << new_db_name << endl;
	if (new_db_name == string(current->get_db_name())) {
		cerr << "can't create a new split database, exiting..." << endl;
		exit(-1);
	}
	current = new wiki_db(new_db_name.c_str());
	dbs.push_back(current);
	current->set_index(dbs.size() - 1);
}

void wiki_db_split::check_if_switching_db_needed(article* art_ptr) {
	// the max id of the articles in the database which is not a rediect entry
	// and article id has to be greater than zero
	int max_id = current->get_max_not_redirect();
	if (max_id > 0 && art_ptr->get_id() >= max_id) {
		current->get_history_table()->save(art_ptr->get_id(), art_ptr->get_article_id(), art_ptr->get_title().c_str());

		switch_db();
	}
}

void wiki_db_split::switch_db() {
	current->end_transaction();
	current->write_to_disk();
	current->close();

	wiki_db *db_ptr = get_next_db();
	current = db_ptr;
	if (!db_ptr) {
		cerr << "reach the last database." << endl << flush;
		thread_manager::instance().we_should_stop(TRUE);
//			exit(-1);
	}
	else {
		cerr << "switching database to " << db_ptr->get_db_name() << endl << flush;
		db_ptr->set_in_memory(1);
		db_ptr->open();
		db_ptr->begin_transaction();

		thread_manager::instance().notify_main_thread_db_ready();
	}
}

wiki_db* wiki_db_split::create_main_db() {
	if (!main) {
		main = new wiki_db(base_name);
		main->set_index(0);

		dbs.push_back(main);
		current = main;
	}
	return main;
}
