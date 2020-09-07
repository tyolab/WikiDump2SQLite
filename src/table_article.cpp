/*
 * article_table.cpp
 *
 *  Created on: 27/06/2014
 *      Author: monfee
 */

#include "table_article.h"
#include "article.h"
#include "compression.h"
#include "str.h"

#include <iostream>

using namespace std;

const std::string table_article::TABLE_NAME = "articles";
const std::string table_article::COLUMN_ID = "id";
const std::string table_article::COLUMN_ARTICLE_ID = "article_id";
//	public static final String COLUMN_TYPE = "type";
const std::string table_article::COLUMN_TITLE = "title";
const std::string table_article::COLUMN_ABSTRACT = "abstract";
const std::string table_article::COLUMN_ARTICLE = "article";
const std::string table_article::COLUMN_LAST_UPDATE = "last_update";
const std::string table_article::COLUMN_LAST_VISIT = "last_visit";
const std::string table_article::COLUMN_CATEGORY = "category";
const std::string table_article::COLUMN_REDIRECT = "redirect";

const std::string table_article::SQL_INSERT = std::string("INSERT OR REPLACE INTO ") +
				TABLE_NAME  + " (" + /*COLUMN_ID + ", "
											+ */COLUMN_ARTICLE_ID + ", "
											+ COLUMN_TITLE  + ", "
											+ COLUMN_ABSTRACT  + ", "
											+ COLUMN_ARTICLE + ", "
											+ COLUMN_CATEGORY + ") VALUES (?, ?, ?, ?, ?)";

const std::string table_article::SQL_INSERT_V2 = std::string("INSERT OR REPLACE INTO ") +
				TABLE_NAME  + " (" + /*COLUMN_ID + ", "
											+ */COLUMN_ID + ", "
											+ COLUMN_ARTICLE_ID  + ", "
											+ COLUMN_ARTICLE + ", "
											+ COLUMN_REDIRECT + ") VALUES (?, ?, ?, ?)";

const std::string table_article::SQL_INSERT_ARTICLE_TITLE = "INSERT INTO " +
			TABLE_NAME  + " (" + /*COLUMN_ID + ", "
										+ */COLUMN_ARTICLE_ID + ", "
										+ COLUMN_TITLE + ") VALUES (?, ?)";

const std::string table_article::SQL_UPDATE_ARTICLE = "UPDATE " +
			TABLE_NAME  + " set " + /*COLUMN_ID + ", "
										+ */COLUMN_ABSTRACT + "=?"
										+ " , "
										+ COLUMN_ARTICLE + "=?"
										+ " where " + COLUMN_ARTICLE_ID + "=?";

const std::string table_article::SQL_UPDATE_ARTICLE_ABSTRACT = "UPDATE " +
			TABLE_NAME  + " set " + COLUMN_ABSTRACT + "=?"
										+ " where " + COLUMN_ARTICLE_ID + "=?";

const std::string table_article::SQL_UPDATE_ARTICLE_ARTICLE = "UPDATE " +
										TABLE_NAME  + " set " + COLUMN_ARTICLE + "=? "
										+ " , "
										+ COLUMN_REDIRECT + "=?"
										+ " where " + COLUMN_ARTICLE_ID + "=?";

const std::string table_article::SQL_CREATE_ARTICLES_TABLE = "CREATE TABLE IF NOT EXISTS articles ("
			"id integer primary key, article_id integer, title text, "
			"abstract blob, article blob, last_update date, last_visit date, category text, "
			"category_ids integer, redirect integer)";

const std::string table_article::SQL_CREATE_ARTICLES_TABLE_V2 = "CREATE TABLE IF NOT EXISTS articles ("
			"id integer primary key, article_id integer, title text, "
			"article blob, redirect integer)";

const std::string table_article::SQL_CREATE_ARTICLES_TABLE_V3 = "CREATE TABLE IF NOT EXISTS articles ("
			"id integer primary key, article_id integer, "
			"article blob, redirect integer)";

//const std::string table_article::SQL_GET_FIRST_UNPROCESSED_ARTICLE_ID = "select article_id from articles "
//			"where id=((select id from articles where article_id = "
//			"(select max(article_id) from articles where length(article) > 0 and redirect=0)) + 1)";

const std::string table_article::SQL_GET_FIRST_UNPROCESSED_ARTICLE_ID = "select min(id) from articles where length(article)=0 or article is null and redirect=0";

const std::string table_article::SQL_GET_UNPROCESSED_ARTICLE_NUMBER = "select count(*) from articles where article is null or length(article) == 0 and redirect=0";

const std::string table_article::SQL_GET_MIN_ID = "min(id)";

const std::string table_article::SQL_GET_MAX_ID = "max(id)";

const std::string table_article::SQL_GET_MAX_ID_NOT_REDIRECT = "select max(id) from articles where redirect=0";

int table_article::table_version = TABLE_VERSION_V2;

table_article::table_article(db_sqlite *db_ptr, const char *name) : db_table::db_table(db_ptr, name) {
	count = 0;

	table_name = TABLE_NAME;

	sql_create = SQL_CREATE_ARTICLES_TABLE_V3;
}

table_article::~table_article() {
}

void table_article::insert(article* art_ptr) {
	sqlite3_stmt *prepStmt = NULL;

    int rc;

    prepStmt = db->prepare_statment(SQL_INSERT_V2.c_str());
//            prepStmt.setInt(1, art_ptr->get_id());

    rc = db->bind_int(prepStmt, 1, art_ptr->get_article_id());

    void *blobAbs = NULL;
    void *blobArticle = NULL;
    int abs_len = art_ptr->get_abstract() ? strlen(art_ptr->get_abstract()) : 0;
    int article_len = art_ptr->get_article() ? strlen(art_ptr->get_article()) : 0;
    int abs_blob_len = abs_len;
    int article_blob_len = article_len;

    if (art_ptr->get_mode() == article::MODE_COMPRESSED_GZIP) {
    	if (abs_len > 0) {
    		blobAbs = new char[MIN_COMPRESSION_BUFFER_SIZE(abs_len)];
    		abs_blob_len = gzip_compress(art_ptr->get_abstract(), (char *)blobAbs);
    	}

    	if (article_len > 0) {
    		int buffer_size = MIN_COMPRESSION_BUFFER_SIZE(article_len);
        	blobArticle = new char[buffer_size];
        	article_blob_len = gzip_compress(art_ptr->get_article(), (char *)blobArticle);
    	}

    }
    else {
		if (art_ptr->get_abstract())
    		blobAbs = strnew(art_ptr->get_abstract());
    	blobArticle = strnew(art_ptr->get_article());
    }

    if (table_version == TABLE_VERSION_V1) {
        rc = db->bind_int(prepStmt, 1, art_ptr->get_article_id());
        rc = db->bind_text(prepStmt, 2, art_ptr->get_title().c_str());

		if (abs_blob_len == 0)
			rc = db->bind_null(prepStmt, 3);
		else
			rc = db->bind_blob(prepStmt, 3, blobAbs, abs_blob_len);

		if (article_blob_len == 0)
			rc = db->bind_null(prepStmt, 4);
		else
			rc = db->bind_blob(prepStmt, 4, blobArticle, article_blob_len);

		if (art_ptr->get_category().length() == 0)
			rc = db->bind_null(prepStmt, 5);
		else
			rc = db->bind_text(prepStmt, 5, art_ptr->get_category().c_str());
    }
    else {
        rc = db->bind_int(prepStmt, 1, art_ptr->get_id());
        rc = db->bind_int(prepStmt, 2, art_ptr->get_article_id());
    	rc = db->bind_blob(prepStmt, 3, blobArticle, article_blob_len);
    	rc = db->bind_int(prepStmt, 4, art_ptr->is_redirect());
    }

    db->execute(prepStmt);

    if (blobAbs) delete [] (char *)blobAbs;
    if (blobArticle) delete [] (char *)blobArticle;
}

int table_article::get_max_id() {
	return get_number_from_first_column(create_select_statment(SQL_GET_MAX_ID.c_str()).c_str());
}

int table_article::get_min_id() {
	return get_number_from_first_column(create_select_statment(SQL_GET_MIN_ID.c_str()).c_str());
}

void table_article::update_content(article* art_ptr) {
	update_content(art_ptr->get_article_id(), art_ptr->get_article(),
			art_ptr->get_article_len(), art_ptr->is_redirect());
}

void table_article::update_content(int article_id, const void* content,
		int article_len, int redirect) {

	sqlite3_stmt *prepStmt = NULL;

    int rc;

    prepStmt = db->prepare_statment(SQL_UPDATE_ARTICLE_ARTICLE.c_str());

    rc = db->bind_blob(prepStmt, 1, content, article_len);

    rc = db->bind_int(prepStmt, 2, redirect);

    rc = db->bind_int(prepStmt, 3, article_id);

    db->execute(prepStmt);
}

void table_article::insert2(article* art_ptr) {

	sqlite3_stmt *prepStmt = NULL;

    int rc;

    prepStmt = db->prepare_statment(SQL_INSERT_V2.c_str());

	 rc = db->bind_int(prepStmt, 1, art_ptr->get_id());

	 rc = db->bind_int(prepStmt, 2, art_ptr->get_article_id());

    rc = db->bind_blob(prepStmt, 3, art_ptr->get_article(), art_ptr->get_article_len());

    rc = db->bind_int(prepStmt, 4, art_ptr->is_redirect());

    db->execute(prepStmt);
}

int table_article::get_last_update() {
	return get_number_from_first_column(SQL_GET_FIRST_UNPROCESSED_ARTICLE_ID.c_str());
}

int table_article::get_max_article_id() {
	return get_number_from_first_column("select max(id) from articles");
}

int table_article::get_how_many_left() {
	return get_number_from_first_column(SQL_GET_UNPROCESSED_ARTICLE_NUMBER.c_str());
}

int table_article::get_max_not_redirect() {
	return get_number_from_first_column(SQL_GET_MAX_ID_NOT_REDIRECT.c_str());
}
/*
 *
You should be able access the table names from the sqlite_master table.

SELECT name FROM sqlite_master WHERE type='table';
The names of the columns are not directly accessible. The easiest way to get them is to query the table and get the column names from the query result.

SELECT * FROM table_name LIMIT 1;

 */
