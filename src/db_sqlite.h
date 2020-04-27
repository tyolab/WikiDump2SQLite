/*
 * db_sqlite.h
 *
 *  Created on: 27/06/2014
 *      Author: monfee
 */

#ifndef DB_SQLITE_H_
#define DB_SQLITE_H_

#include <sqlite3.h>
#include <string>

class db_sqlite {
private:
	sqlite3 *db;
	std::string db_name;

	int connected;

public:
	db_sqlite();
	db_sqlite(const char *name);
	virtual ~db_sqlite();

	static int operate(sqlite3 *pInMemory, const char *zFilename, int isSave, char *destDBName="main");

	int create_in_memory_database();

	int load_database_into_memory(const char *db_name);

	int attach_database(const char *db_name, const char *as_name);

	int open_database(const char *db_name);

	int save_in_memory_database_to_file(const char *db_name);

	int write_to_disk();

	void connect(const char *);
	int close();

	int execute(const char *sql);
	int execute(sqlite3_stmt *stmt);

	int execute_only(sqlite3_stmt* stmt);  // not to finalize the statment yet
	int finalize_statment(sqlite3_stmt *stmt);

	sqlite3_stmt *prepare_statment(const char *sql);

	int bind_blob(sqlite3_stmt *stmt, int index, const void *blob, long size);
	int bind_int(sqlite3_stmt *stmt, int index, int value);
	int bind_text(sqlite3_stmt *stmt, int index, const char *value);
	int bind_null(sqlite3_stmt* stmt, int index);

	int count_rows(const char *table_name);

	int is_connected() { return connected; }

	int begin_transaction();
	int end_transaction();

	int inert_table(const char *table_name, const char *from, const char *clause = "");

	std::string get_db_name() {
		return db_name;
	}

	void set_db_name(const std::string& name) {
		db_name = name;
	}
};

#endif /* DB_SQLITE_H_ */
