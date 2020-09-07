/*
 * wiki_db.h
 *
 *  Created on: 04/07/2014
 *      Author: monfee
 */

#ifndef WIKI_DB_H_
#define WIKI_DB_H_

#include "table_history.h"
#include "table_article.h"
#include "table_title.h"

#include "pattern_singleton.h"

#include "runnable.h"

class wiki_db /*: public pattern_singleton<wiki_db>, public runnable*/ {

private:
//	const char *db_name;

	db_sqlite db;

	table_article *article_t;

	table_history *history_t;

	table_title *title_t;

	int index;  // 0 the main db has title db and history db

	int connected;

	int min;

	int max;

	int max_not_redirect;

	int how_many_left;

	int in_memory;

	long something_changed;

	static int fix_mode;

public:
	wiki_db();
	wiki_db(const char *db_name);
	virtual ~wiki_db();

	void open();

//	void create_database_in_memory();
//
//	void save_database_to_file(const char *file);
	void write_to_disk();

	void close();

	table_title *get_title_table() { return title_t; }

	table_article *get_article_table();

//	void set_article_table(const table_article& articlet);

	table_history *get_history_table();

	int is_ready() { return connected; }

//	void set_history_table(const table_history& historyt);

	std::string get_db_name() {
		return db.get_db_name();
	}

	void set_db_name(const char* name) {
		db.set_db_name(std::string(name));
	}

	void begin_transaction();

	void end_transaction();

	void update_article(article *art_ptr);
	void insert_article(article *art_ptr);

	int get_index() const {
		return index;
	}

	void set_index(int index) {
		this->index = index;
	}

	int get_in_memory() const {
		return in_memory;
	}

	void set_in_memory(int inmemory) {
		in_memory = inmemory;
	}

	int get_max() const {
		return max;
	}

	int get_max_not_redirect() const {
		return max_not_redirect;
	}

	void set_max(int max) {
		this->max = max;
	}

	int get_min() const {
		return min;
	}

	void set_min(int min) {
		this->min = min;
	}

	void detect();

	int get_limit();

	int get_how_many_left() const {
		return how_many_left;
	}

	static void set_fix_mode(int mode);
	static int get_fix_mode();

	long get_something_changed() const;
	void set_something_changed(long somethingchanged);
};

#endif /* WIKI_DB_H_ */
