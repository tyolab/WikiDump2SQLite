/*
 * db_sqlite.cpp
 *
 *  Created on: 27/06/2014
 *      Author: monfee
 */

#include "db_sqlite.h"

#include <string.h>
#include <stdio.h>

#include <iostream>

using namespace std;

db_sqlite::db_sqlite() : db_sqlite("wikipedia.db") {

}

db_sqlite::db_sqlite(const char* name) : db_name(name) {
	db = NULL;
	connected = 0;
}


db_sqlite::~db_sqlite() {
	if (db != NULL)
		close();
}

/*
** This function is used to load the contents of a database file on disk
** into the "main" database of open database connection pInMemory, or
** to save the current contents of the database opened by pInMemory into
** a database file on disk. pInMemory is probably an in-memory database,
** but this function will also work fine if it is not.
**
** Parameter zFilename points to a nul-terminated string containing the
** name of the database file on disk to load from or save to. If parameter
** isSave is non-zero, then the contents of the file zFilename are
** overwritten with the contents of the database opened by pInMemory. If
** parameter isSave is zero, then the contents of the database opened by
** pInMemory are replaced by data loaded from the file zFilename.
**
** If the operation is successful, SQLITE_OK is returned. Otherwise, if
** an error occurs, an SQLite error code is returned.
**
** could be save / or load
*/
int db_sqlite::operate(sqlite3 *pInMemory, const char *zFilename, int isSave, char *destDBName){
  int rc;                   /* Function return code */
  sqlite3 *pFile;           /* Database connection opened on zFilename */
  sqlite3_backup *pBackup;  /* Backup object used to copy data */
  sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
  sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

  /* Open the database file identified by zFilename. Exit early if this fails
  ** for any reason. */
  rc = sqlite3_open(zFilename, &pFile);
  if( rc==SQLITE_OK ){

    /* If this is a 'load' operation (isSave==0), then data is copied
    ** from the database file just opened to database pInMemory.
    ** Otherwise, if this is a 'save' operation (isSave==1), then data
    ** is copied from pInMemory to pFile.  Set the variables pFrom and
    ** pTo accordingly. */
    pFrom = (isSave ? pInMemory : pFile);
    pTo   = (isSave ? pFile     : pInMemory);

    /* Set up the backup procedure to copy from the "main" database of
    ** connection pFile to the main database of connection pInMemory.
    ** If something goes wrong, pBackup will be set to NULL and an error
    ** code and  message left in connection pTo.
    **
    ** If the backup object is successfully created, call backup_step()
    ** to copy data from pFile to pInMemory. Then call backup_finish()
    ** to release resources associated with the pBackup object.  If an
    ** error occurred, then  an error code and message will be left in
    ** connection pTo. If no error occurred, then the error code belonging
    ** to pTo is set to SQLITE_OK.
    */
    pBackup = sqlite3_backup_init(pTo, destDBName, pFrom, "main");
    if( pBackup ){
      (void)sqlite3_backup_step(pBackup, -1);
      (void)sqlite3_backup_finish(pBackup);
    }
    rc = sqlite3_errcode(pTo);
  }

  /* Close the database connection opened on database file zFilename
  ** and return the result of this function. */
  (void)sqlite3_close(pFile);
  return rc;
}

void db_sqlite::connect(const char *file) {
	connected = open_database(file) == SQLITE_OK;
}

int db_sqlite::execute(const char *sql) {
	int rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
	if (rc != SQLITE_OK)
		cerr << "execution failed: " << sqlite3_errmsg(db) << endl;
	return rc;
}

int db_sqlite::execute(sqlite3_stmt* stmt) {
	int rc = execute_only(stmt);
	finalize_statment(stmt);
	return rc;
}

int db_sqlite::bind_blob(sqlite3_stmt* stmt, int index, const void *blob, long size) {
	int rc = sqlite3_bind_blob(stmt, index, blob, size, SQLITE_STATIC);
	if (rc != SQLITE_OK) {
		cerr << "bind blob failed: " << sqlite3_errmsg(db) << endl;
	}
}

int db_sqlite::bind_null(sqlite3_stmt* stmt, int index) {
	int rc = sqlite3_bind_null(stmt, index);
	if (rc != SQLITE_OK) {
		cerr << "bind null failed: " << sqlite3_errmsg(db) << endl;
	}
}

sqlite3_stmt *db_sqlite::prepare_statment(const char *sql) {
//	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare(db, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		cerr << "prepare failed: " << sqlite3_errmsg(db) << endl;
	}
	return stmt;
}

int db_sqlite::bind_int(sqlite3_stmt* stmt, int index, int value) {
	int rc = sqlite3_bind_int(stmt, index, value);
	if (rc != SQLITE_OK) {
		cerr << "bind int failed: " << sqlite3_errmsg(db) << endl;
	}
	return rc;
}

int db_sqlite::bind_text(sqlite3_stmt* stmt, int index, const char *value) {
	int rc = sqlite3_bind_text(stmt, index, value, strlen(value), SQLITE_STATIC);
	if (rc != SQLITE_OK) {
		cerr << "bind text failed: " << sqlite3_errmsg(db) << endl;
	}
	return rc;
}

int db_sqlite::close() {
	int rc = sqlite3_close(db);
	db = NULL;
	if (rc != SQLITE_OK) {
		cerr << "close database failed: " << sqlite3_errmsg(db) << endl;
	}
	return rc;
}

int db_sqlite::begin_transaction() {
	sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
}

int db_sqlite::create_in_memory_database() {
	int rc = sqlite3_open(":memory:", &db);
	if (rc != SQLITE_OK)
		cerr << "create in-memory database failed: " << sqlite3_errmsg(db) << endl;
	return rc;
}

int db_sqlite::load_database_into_memory(const char* db_name) {
	connected = operate(db, db_name, 0) == SQLITE_OK;
}

int db_sqlite::open_database(const char* file) {
	db_name = file;
//	int rc = sqlite3_open_v2(db_name.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);
	int rc = sqlite3_open(db_name.c_str(), &db);
	if (rc != SQLITE_OK) {
		cerr << "db open failed: " << sqlite3_errmsg(db) << endl;
	}
	return rc;
}

int db_sqlite::save_in_memory_database_to_file(const char* db_name) {
	operate(db, db_name, 1);
}

int db_sqlite::write_to_disk() {
	this->save_in_memory_database_to_file(db_name.c_str());
}

int db_sqlite::attach_database(const char* db_name, const char *as_name) {
	char buffer[2048];
	sprintf(buffer, "attach '%s' as %s", db_name, as_name);
	execute(buffer);
}

int db_sqlite::end_transaction() {
	sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);
}

int db_sqlite::count_rows(const char* table_name) {
	sqlite3_stmt *prepStmt = prepare_statment(string("select count(*) from " + string(table_name)).c_str());

	execute_only(prepStmt);
	int rows = sqlite3_column_int(prepStmt, 0);

	this->finalize_statment(prepStmt);
	return rows;
}

int db_sqlite::execute_only(sqlite3_stmt* stmt) {
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE && rc != SQLITE_OK && rc != SQLITE_ROW)
		cerr << "execution failed: " << sqlite3_errmsg(db) << endl;
	return rc;
}

int db_sqlite::finalize_statment(sqlite3_stmt* stmt) {
	int rc = sqlite3_finalize(stmt);
	if (rc != SQLITE_OK)
		cerr << "finalize statement failed: " << sqlite3_errmsg(db) << endl;
	return rc;
}

int db_sqlite::inert_table(const char* table_name, const char *from, const char* clause) {
	char buffer[2048];
	sprintf(buffer, "insert into %s select * from %s.%s %s", table_name, from, table_name, clause);
	execute(buffer);
}
