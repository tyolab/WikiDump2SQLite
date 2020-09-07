/*
 * wiki_db.cpp
 *
 *  Created on: 04/07/2014
 *      Author: monfee
 */

#include "wiki_db.h"
#include "table_math.h"

#include <stdio.h>

#include <iostream>

using namespace std;

//std::deque<article *> wiki_db::article_queue;
int wiki_db::fix_mode = 0;

wiki_db::wiki_db() : wiki_db("wikipedia.db")/*, db(), article_t(&db), history_t(&db)*/ {
//	db_name = "wikipedia.db";

}

wiki_db::wiki_db(const char* name) {
	db = db_sqlite(name);

	article_t = new table_article(&db);
	history_t = new table_history(&db);
	title_t = new table_title(&db);
	index = -1;
	connected = 0;
	in_memory = 0;
	min = 0;
	max = 0;
	max_not_redirect = 0;
	something_changed = 0;

	how_many_left = 0;
}


wiki_db::~wiki_db() {
	delete article_t;
	delete history_t;
}

table_article *wiki_db::get_article_table() {
	return article_t;
}

void wiki_db::write_to_disk() {
//	db.end_transaction();

//		string new_name = increase_file_num(db_name);
//		cerr << "saving in-memory database to file: " << new_name << endl;
	if (in_memory && something_changed) {
		cerr << "writing database '" << get_db_name() <<  "' to disk" << endl << flush;
		db.save_in_memory_database_to_file(db.get_db_name().c_str());
	}
//	db.begin_transaction();
}

//void wiki_db::set_article_table(table_article& art_t) {
//	article_t = art_t;
//}

table_history *wiki_db::get_history_table() {
	return history_t;
}

void wiki_db::open() {
	if (in_memory) {
		db.create_in_memory_database();

		db.load_database_into_memory(db.get_db_name().c_str());
	}
	else
		db.connect(db.get_db_name().c_str());

	article_t->create();
	history_t->create();

	history_t->insert_default_row();

	if (index < 1) {

		title_t->create();

		table_math math_table(&db);
		math_table.create();
	}

	int rows_count = article_t->get_row_count();

	if (rows_count > 0)
		cerr << "current article table has " << rows_count << " records." << endl;

	connected = 1;
}

void wiki_db::close() {
	if (in_memory)
		db.save_in_memory_database_to_file(db.get_db_name().c_str());
	else
		db.close();
	connected = 0;
}

void wiki_db::begin_transaction() {
	db.begin_transaction();
}

void wiki_db::end_transaction() {
	db.end_transaction();
}

void wiki_db::update_article(article *art_ptr) {
	const void *blob = art_ptr->get_article();
	int blob_len = art_ptr->get_article_len();

	article_t->update_content(art_ptr->get_article_id(), blob, blob_len);
}

void wiki_db::insert_article(article *art_ptr) {
	article_t->insert(art_ptr);
}

void wiki_db::detect() {
	max = article_t->get_max_id();
	min = article_t->get_min_id();
	how_many_left = article_t->get_how_many_left();
	max_not_redirect = article_t->get_max_not_redirect();
}

int wiki_db::get_limit() {
	if (index == 0)
		return 160000;
	return 500000;
}

long wiki_db::get_something_changed() const {
	return something_changed;
}

void wiki_db::set_something_changed(long somethingchanged) {
	something_changed = somethingchanged;
}

void wiki_db::set_fix_mode(int mode) {
	wiki_db::fix_mode = mode;
}

int wiki_db::get_fix_mode() {
	return wiki_db::fix_mode;
}