/*
 * test_article_insert.cpp
 *
 *  Created on: 27/06/2014
 *      Author: monfee
 */

#include "../src/db_sqlite.h"
#include "../src/article.h"
#include "../src/article_table.h"

int main ()
{
	db_sqlite db;
	db.connect("./test.db");



	db.close();
	return 0;
}


