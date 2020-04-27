/*
 * wiki_db_split.h
 *
 *  Created on: 22 Sep 2014
 *      Author: monfee
 */

#ifndef WIKI_DB_SPLIT_H_
#define WIKI_DB_SPLIT_H_

#include <vector>

#include "pattern_singleton.h"
#include "runnable.h"

class wiki_db;

class article;

class wiki_db_split : public pattern_singleton<wiki_db_split>, public runnable  {
public:
	static int milestone;
	static const char *BASE_NAME_DEFAULT;

private:
	std::vector<wiki_db *> dbs;

	const char *base_name;

	int to_split; // 0 false, 1 true

	int upto;
	int where_to_stop;

	wiki_db *main;
	wiki_db *current;

public:
	wiki_db_split();
	virtual ~wiki_db_split();

	void set_up_to(int to) { upto = to; }

	void set_to_split(int split_it) { to_split = split_it; }

	void create_new_db();

	void load_db();

	void start(int id);

	void set_base_name(const char *name) { base_name = name; }

	static void process(int thread_id);

	int where_we_are_up_to();

	void check_if_switching_db_needed(article *atr_ptr);

	void switch_db();

	int where_we_stop();

	wiki_db *get_current_db() { return current; }
	wiki_db *get_next_db();

	wiki_db *create_main_db();
};

#endif /* WIKI_DB_SPLIT_H_ */
