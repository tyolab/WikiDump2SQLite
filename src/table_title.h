/*
 * table_title.h
 *
 *  Created on: 18 Sep 2014
 *      Author: monfee
 */

#ifndef TABLE_TITLE_H_
#define TABLE_TITLE_H_

#include "db_table.h"

class table_title  : public db_table {
public:
	static const std::string COLUMN_ID;
	static const std::string COLUMN_TITLE;

	static const std::string TABLE_NAME;

	static const std::string SQL_CREATE;
	static const std::string SQL_INSERT;

	static const std::string SQL_UPDATE;

public:
	table_title(db_sqlite *db_ptr);
	virtual ~table_title();

	void insert(long id, const char *title);
};

#endif /* TABLE_TITLE_H_ */
