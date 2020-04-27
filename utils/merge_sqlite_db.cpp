/*
 * merge_sqlite_db.cpp
 *
 *  Created on: 01/07/2014
 *      Author: monfee
 */

#include "db_sqlite.h"

#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
	if (argc < 3) {
		cerr << "usage: " << argv[0] << "1.db 2.db 3.db ... /a/path/to/merged/db" << endl;
		exit(-1);
	}

	db_sqlite db;

	db.create_in_memory_database();

	const char *dest_db = argv[argc - 1];

	/*
	 * get .schema information
	 *

	  SELECT name, sql FROM sqlite_master
		WHERE type='table'
		ORDER BY name;

	 */
	for (int i = 1; i < (argc - 1); ++i) {
		const char *db_name = argv[i];

		db.load_database_into_memory(db_name);

		if (db.is_connected()) {

		}
		else {
			cerr << "could not attache database: " << db_name << endl;
			break;
		}
	}

	db.save_in_memory_database_to_file(dest_db);

	db.close();
	return -1;
}

