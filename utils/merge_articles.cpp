/*
 * merge_sqlite_db.cpp
 *
 *  Created on: 01/07/2014
 *      Author: monfee
 */

#include "db_sqlite.h"
#include "db_table.h"
#include "table_article.h"

#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>

using namespace std;

void usage(char *program) {
	cerr << "usage: " << program << " from1.db from2.db to.db [record number for table in db2] [record number for table in db1]" << endl;
	exit(-1);
}

int main(int argc, char **argv)
{
	if (argc < 4) {
		usage(argv[0]);
	}

	long record_num;

	int param = 1;

	std::string table_name = "articles";

	for (; param < argc; ++param) {
		const char *opt;
		if (argv[param][0] == '-') {
			opt = argv[param] + 1;
			if (strcmp(opt, "table") == 0) {
				++param;
				table_name = argv[param];
			}
			else {
				fprintf(stderr, "unknown option: %s \n", argv[param]);
				usage(argv[0]);
			}
		}
		else
			break;
	}

	const char *from_db1 = argv[param++];
	const char *from_db2 = argv[param++];
	const char *to_db = argv[param++];

	const char *asname = "fromdb";
//	const char *table_name  =argv[4];


	db_sqlite db;

	db.create_in_memory_database();

	db.load_database_into_memory(from_db1);
	/*
	 * get .schema information
	 *

	  SELECT name, sql FROM sqlite_master
		WHERE type='table'
		ORDER BY name;

	 */

	if (db.is_connected()) {

	//	db.attach_database(from_db1, "fromdb1");
		db.attach_database(from_db2, asname);

		stringstream buf;
		buf.str("");

	//	if (argc > 5)
	//		record_num = atol(argv[5]);

		if (param < argc) {
			const char *opt = argv[param++];
			bool first_part = true;
			if (*opt == '+') {
				first_part = false;
				++opt;
			}
			record_num = atol(opt);
			table_article article_t(&db, asname);

			long long min_id = article_t.get_min_id();
			record_num += min_id;

			buf << " where id";

			if (first_part)
				buf << "<";
			else
				buf << ">=";
			buf << record_num;
		}
		db.inert_table(table_name.c_str(), asname, buf.str().c_str());

	}
	else {
		cerr << "could not attache database: " << from_db1 << endl;
	}

	db.save_in_memory_database_to_file(to_db);

	db.close();
	return -1;
}

