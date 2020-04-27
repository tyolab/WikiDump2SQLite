/*
 * math2db.cpp
 *
 *  Created on: 05/08/2014
 *      Author: monfee
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "db_sqlite.h"
#include "sys_files.h"
#include "table_math.h"

using namespace std;

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef TRUE
	#define TRUE (!FALSE)
#endif

void usage(const char *program) {
	fprintf(stderr, "Usage: %s -type [1|2] path [db]\n", program);
	fprintf(stderr, "\t\t -type 1 math \n");
	fprintf(stderr, "\t\t	 	2 timeline \n");
	exit(-1);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		usage(argv[0]);
	}

	db_sqlite *db_ptr;

	int param = 1;
	int type = -1;

	for (; param < argc; ++param) {
		char *opt;
		if (argv[param][0] == '-') {
			opt = argv[param] + 1;
			if (strcmp(opt, "type") == 0) {
				++param;
				int num = atoi(argv[param]);
				if (num >= table_math::IMAGE_TYPE_MATH && num <= table_math::IMAGE_TYPE_TIMELINE) // 1 for math, 2 for timeline
					type = num;
			}
		}
		else
			break;
	}

	if (type == -1)
		usage(argv[0]);

	const char *path = argv[param++];

	string db_name;
	if (param >= argc) {
		db_name = "wikipedia-";

		if (type == table_math::IMAGE_TYPE_MATH)
			db_name.append("math");
		else
			db_name.append("timeline");
		db_name.append(".db");
	}
	else
		db_name = argv[param];

	db_ptr = new db_sqlite(db_name.c_str());

	db_ptr->create_in_memory_database();

	if (sys_file::exist(db_name.c_str()))
		db_ptr->load_database_into_memory(db_name.c_str());

	db_ptr->begin_transaction();

	table_math table(db_ptr);
	table.create();

//	if (sys_file::exist(path))
//		db_ptr->load_database_into_memory(path);

	sys_files disk;
	disk.list(path);
	int len = strlen(path);
	const char *last_char = path + (len - 1);
	if (*(last_char) != '/' )
		++len;
//	++len;

	const char *name = disk.first();

	int count = 0;
	while (name) {
		cout << "#" << ++count << ":" << name << endl;
		table.insert(name, type);
		name = disk.next();
	}

	db_ptr->end_transaction();

	db_ptr->write_to_disk();

	db_ptr->close();

//	db_ptr->save_in_memory_database_to_file(argc >= 3 ? argv[2] : "math.db");

	delete db_ptr;

	return 0;
}


