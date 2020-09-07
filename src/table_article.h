/*
 * article_table.h
 *
 *  Created on: 27/06/2014
 *      Author: monfee
 */

#ifndef ARTICLE_TABLE_H_
#define ARTICLE_TABLE_H_

#include <string>

#include "db_sqlite.h"
#include "article.h"
#include "db_table.h"

class table_article : public db_table {
public:

	static const std::string COLUMN_ID;
	static const std::string COLUMN_ARTICLE_ID;
//	public static final String COLUMN_TYPE;
	static const std::string COLUMN_TITLE;
	static const std::string COLUMN_ABSTRACT;
	static const std::string COLUMN_ARTICLE;
	static const std::string COLUMN_LAST_UPDATE;
	static const std::string COLUMN_LAST_VISIT;
	static const std::string COLUMN_CATEGORY;
	static const std::string COLUMN_REDIRECT;

	static const std::string TABLE_NAME;

	static const std::string SQL_CREATE_ARTICLES_TABLE;
	static const std::string SQL_CREATE_ARTICLES_TABLE_V2;
	static const std::string SQL_CREATE_ARTICLES_TABLE_V3;

	static const std::string SQL_INSERT;
	static const std::string SQL_INSERT_V2;
	static const std::string SQL_INSERT_ARTICLE_TITLE;

	static const std::string SQL_UPDATE_ARTICLE;
	static const std::string SQL_UPDATE_ARTICLE_ABSTRACT;
	static const std::string SQL_UPDATE_ARTICLE_ARTICLE;

	static const std::string SQL_GET_FIRST_UNPROCESSED_ARTICLE_ID;

	static const std::string SQL_GET_UNPROCESSED_ARTICLE_NUMBER;

	static const std::string SQL_GET_MIN_ID;
	static const std::string SQL_GET_MAX_ID;

	static const std::string SQL_GET_MAX_ID_NOT_REDIRECT;

	static const int TABLE_VERSION_V1 = 1;

	static const int TABLE_VERSION_V2 = 2;

	static int table_version;

private:
	int count;

public:
	table_article(db_sqlite *db_ptr, const char *db_name="main");
	virtual ~table_article();

	void insert(article *art_ptr);
	void insert2(article *art_ptr);

	int get_max_id();
	int get_min_id();

	void update_content(article *art_ptr);

	void update_content(int article_id, const void *content, int content_len, int redirect = 0);

	int get_last_update();

	int get_max_article_id();

	int get_how_many_left();

	int get_max_not_redirect();
};


#endif /* ARTICLE_TABLE_H_ */
