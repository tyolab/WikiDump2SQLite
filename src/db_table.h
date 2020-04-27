/*
 * db_table.h
 *
 *  Created on: 05/07/2014
 *      Author: monfee
 */

#ifndef DB_TABLE_H_
#define DB_TABLE_H_

#include <string>

#include "db_sqlite.h"

class db_table {

protected:
	std::string table_name;
	std::string db_name;

	std::string sql_insert;
	std::string sql_create;
	std::string sql_update;

	db_sqlite *db;

	int row_count;

public:
	db_table(db_sqlite *db_ptr, const char *db_name="main");
	virtual ~db_table();

	int get_row_count();

	int get_number_from_first_column(const char *sql);

	void set_table(const char *name);

	void set_db_name(const char *name);

	std::string create_select_statment(const char *fileds, const char *clause = NULL);

	void create();

	void clear();
};


#endif /* DB_TABLE_H_ */
