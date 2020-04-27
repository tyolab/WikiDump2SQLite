/*
 * db_table.cpp
 *
 *  Created on: 05/07/2014
 *      Author: monfee
 */

#include "db_table.h"

#include <exception>
#include <iostream>
#include <sstream>

using namespace std;

db_table::db_table(db_sqlite *db_ptr, const char *name) : db_name(name) {
	db = db_ptr;
	row_count = 0;
}

db_table::~db_table() {
}

void db_table::create() {
    int rc = db->execute(sql_create.c_str());
    if (rc != SQLITE_OK) {
		const char* error_msg = ((std::string("failed to create table ") + table_name).c_str());
		std::cerr << error_msg << std::endl;
		throw error_msg;
    }
}

int db_table::get_row_count() {
	return row_count = db->count_rows(table_name.c_str());
}

void db_table::set_db_name(const char* name) {
	db_name = name;
}

void db_table::clear() {
	db->execute((string("delete from ") + table_name).c_str());
}

int db_table::get_number_from_first_column(const char* sql) {
	sqlite3_stmt *prepStmt = db->prepare_statment(sql);

	db->execute_only(prepStmt);

	int rows = sqlite3_column_int(prepStmt, 0);

	db->finalize_statment(prepStmt);
	return rows;
}

void db_table::set_table(const char* name) {
	table_name = name;
}

std::string db_table::create_select_statment(const char *fileds, const char *clause) {
	std::stringstream buf;
	buf << "select " << fileds << " from " << db_name << "." << table_name;
	if (clause != NULL)
		buf <<" where " << clause;
	return buf.str();
}
