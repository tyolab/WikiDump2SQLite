/*
 * table_title.cpp
 *
 *  Created on: 18 Sep 2014
 *      Author: monfee
 */

#include "table_title.h"

const std::string table_title::TABLE_NAME = "titles";

const std::string table_title::COLUMN_ID = "id";

const std::string table_title::COLUMN_TITLE = "title";


const std::string table_title::SQL_CREATE = "CREATE TABLE IF NOT EXISTS titles (id integer, title text, PRIMARY KEY (id) )"; /* , PRIMARY KEY (a, b, c, d) */

const std::string table_title::SQL_INSERT = "insert into titles (id, title) values(?, ?)";

const std::string table_title::SQL_UPDATE = "";

table_title::table_title(db_sqlite *db_ptr) : db_table::db_table(db_ptr)  {
	table_name = TABLE_NAME;
	sql_create = SQL_CREATE;
}

table_title::~table_title() {
}

void table_title::insert(long id, const char* title) {
	sqlite3_stmt *prepStmt = NULL;
    int rc;

    prepStmt = db->prepare_statment(SQL_INSERT.c_str());

    rc = db->bind_int(prepStmt, 1, id);
    rc = db->bind_text(prepStmt, 2, title);

	rc = db->execute(prepStmt);
}
