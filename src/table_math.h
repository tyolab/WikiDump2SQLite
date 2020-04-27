/*
 * table_math.h
 *
 *  Created on: 05/08/2014
 *      Author: monfee
 */

#ifndef TABLE_MATH_H_
#define TABLE_MATH_H_

#include "db_table.h"

class table_math  : public db_table {
public:
	static const int IMAGE_TYPE_MATH = 1;
	static const int IMAGE_TYPE_TIMELINE = 2;

	static const std::string COLUMN_ID;
	static const std::string COLUMN_A;
	static const std::string COLUMN_B;
	static const std::string COLUMN_C;
	static const std::string COLUMN_D;
	static const std::string COLUMN_CONTENT;

	static const std::string TABLE_NAME;

	static const std::string SQL_CREATE;
	static const std::string SQL_INSERT;

	static const std::string SQL_UPDATE;

public:
	table_math(db_sqlite *db_ptr);
	virtual ~table_math();

	void insert(const char *path, int type);
	void reset() { row_count = 0; }
};

#endif /* TABLE_MATH_H_ */
