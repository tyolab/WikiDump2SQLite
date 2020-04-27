/*
 * table_math.cpp
 *
 *  Created on: 05/08/2014
 *      Author: monfee
 */

#include "table_math.h"
#include "sys_file.h"

#include <string.h>

#include <iostream>

const std::string table_math::TABLE_NAME = "math";

const std::string table_math::COLUMN_ID = "id";

const std::string table_math::COLUMN_A = "a";
const std::string table_math::COLUMN_B = "b";
const std::string table_math::COLUMN_C = "c";
const std::string table_math::COLUMN_D = "d";

const std::string table_math::COLUMN_CONTENT = "content";


const std::string table_math::SQL_CREATE = "CREATE TABLE IF NOT EXISTS math (id integer, a integer, b integer, c integer, d text, content blob , type integer, ext text, PRIMARY KEY (a, b, c, d, type, ext) )"; /* , PRIMARY KEY (a, b, c, d) */

const std::string table_math::SQL_INSERT = "insert into math (a, b, c, d, content, type, ext, id) values(?, ?, ?, ?, ?, ?, ?, ?)";

const std::string table_math::SQL_UPDATE = "";

table_math::table_math(db_sqlite *db_ptr) : db_table::db_table(db_ptr)  {
	table_name = TABLE_NAME;
	sql_create = SQL_CREATE;
}

table_math::~table_math() {

}

void table_math::insert(const char *path, int type) {

	char *temp = strdup(path);

	sqlite3_stmt *prepStmt = NULL;

	std::string ext;

    int rc;

    ++row_count;

    prepStmt = db->prepare_statment(SQL_INSERT.c_str());

//    rc = db->bind_int(prepStmt, 1, row_count);

    sys_file file(path);
    file.read_entire_file();

	  char * pch;
//	  pch = strtok (temp,"/.");
//	  int count = 0;
	  int c;
//	  while (pch != NULL)
//	  {
//		  switch (count) {
//		  case 1:
//		  case 2:
//		  case 3:
//			  c = (int)*pch;
//			  rc = db->bind_int(prepStmt, count + 1, c);
//			  break;
//		  case 4:
//			  rc = db->bind_text(prepStmt, 5, pch);
//			  break;
//		  case 5:
//			  rc = db->bind_blob(prepStmt, 6, file.content(), file.length());
//			  break;
//		  }
//
//		  ++count;
//		  pch = strtok (NULL, "/.");
//	  }
//
//	  if (count >= 4)
    pch = strrchr(temp, '/');
    char *pos;
	for (int i = 1; i < 8; ++i)
	{
	  switch (i) {
	  case 1:
	  case 2:
	  case 3:
		  c = (int)*++pch;
		  rc = db->bind_int(prepStmt, i, c);
		  break;
	  case 4:
		  pos = strchr(++pch, '.');
		  ext = pos + 1;
		  *pos = '\0';
		  rc = db->bind_text(prepStmt, 4, pch);
		  break;
	  case 5:
		  rc = db->bind_blob(prepStmt, 5, file.content(), file.length());
		  break;
	  case 6:
		  rc = db->bind_int(prepStmt, i, type);
		  break;
	  case 7:
		  rc = db->bind_text(prepStmt, i, ext.c_str());
		  break;
	  }
	}

	rc = db->bind_int(prepStmt, 8, row_count);

	rc = db->execute(prepStmt);

    free(temp);
//	if (rc != SQLITE_OK) {
//		const char* error_msg = (std::string("failed to insert row(") + path + ") to table " + table_name).c_str();
//		std::cerr << error_msg << std::endl;
//	    	throw error_msg;
//	}

}


