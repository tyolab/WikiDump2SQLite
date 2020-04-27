/*
 * table_history.h
 *
 *  Created on: 04/07/2014
 *      Author: monfee
 */

#ifndef TABLE_HISTORY_H_
#define TABLE_HISTORY_H_

#include "db_table.h"

#include <string>

class table_history : public db_table {
public:
	static const std::string COLUMN_ID;
	static const std::string COLUMN_ARTICLE_ID;
//	public static final String COLUMN_TYPE;
	static const std::string COLUMN_TITLE;
	static const std::string COLUMN_LAST_VISIT;

	static const std::string TABLE_NAME;

	static const std::string SQL_CREATE;
	static const std::string SQL_INSERT;

	static const std::string SQL_UPDATE;

public:
	table_history(db_sqlite *db_ptr);
	virtual ~table_history();

	void save(int id, int art_id, const char *title);

	void insert_default_row();

	int get_last_update();
};

#endif /* TABLE_HISTORY_H_ */
