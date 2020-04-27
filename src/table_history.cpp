/*
 * table_history.cpp
 *
 *  Created on: 04/07/2014
 *      Author: monfee
 */

#include "table_history.h"

using namespace std;

const std::string table_history::TABLE_NAME = "history";
const std::string table_history::COLUMN_ID = "id";
const std::string table_history::COLUMN_ARTICLE_ID = "article_id";
//	public static final String COLUMN_TYPE = "type";
const std::string table_history::COLUMN_TITLE = "title";
const std::string table_history::COLUMN_LAST_VISIT = "last_visit";

const std::string table_history::SQL_CREATE = "CREATE TABLE IF NOT EXISTS history ("
			"id integer primary key, article_id integer, title text, "
			"last_visit text)";

const std::string table_history::SQL_INSERT = "insert into history ("
			"id, article_id, title, last_visit) values (0, 0, '', '')";

const std::string table_history::SQL_UPDATE = "update history set "
			"id=?, article_id=?, title=? , last_visit=datetime('now')";

table_history::table_history(db_sqlite *db_ptr) : db_table::db_table(db_ptr) {
	table_name = TABLE_NAME;

	sql_create = SQL_CREATE;
	sql_insert = SQL_INSERT;
}

table_history::~table_history() {

}

void table_history::save(int id, int art_id, const char* title) {
	sqlite3_stmt *prepStmt = NULL;

    int rc;

    prepStmt = db->prepare_statment(SQL_UPDATE.c_str());

    rc = db->bind_int(prepStmt, 1, id);

    rc = db->bind_int(prepStmt, 2, art_id);

    rc = db->bind_text(prepStmt, 3, title);

    db->execute(prepStmt);
}

void table_history::insert_default_row() {
	if (this->get_row_count() == 0)
		db->execute(sql_insert.c_str());
}

int table_history::get_last_update() {
	return get_number_from_first_column((string("select ") + COLUMN_ID + " from " + string(table_name)).c_str());
}
