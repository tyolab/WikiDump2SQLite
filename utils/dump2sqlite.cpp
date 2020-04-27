#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>
#include <fstream>

#include "db_sqlite.h"
#include "article.h"
#include "table_article.h"
#include "string_utils.h"

using namespace std;

char title_buffer[1024 * 1024];
char *title_buffer_ptr = title_buffer;
int rows_count = 0;


char *goto_next_line(char *mem, int size) {
	int count = 0;
	while (*mem++ != '\n' && count < size)
		++count;

	return mem;
}

void process(char *memblock, int buffer_size, db_sqlite& db, table_article& table, int count) {
	char *curr = memblock;
	char *new_title = NULL;

	new_title = goto_next_line(memblock, buffer_size);

	if (strlen(title_buffer) > 0) {
		title_buffer_ptr = strncat(title_buffer, curr, new_title - curr);
		curr = new_title;
	}
	else {
		if (count == 0) {
			curr = new_title;
			*new_title = '\0';
			new_title = goto_next_line(memblock, buffer_size);
		}

		if (rows_count % 500 == 0) {
			db.end_transaction();
			db.begin_transaction();
		}

		/*
		 * for DEBUG
		 */
		string title = curr;
		string real_t = find_and_replace(title, string("_"), string(" "));
		article art = article(article::MODE_COMPRESSED_GZIP);
		art.set_id(rows_count++);
		art.set_title(real_t.c_str());

		art.set_article_id(331143);
		art.set_article("I am not a superman");
//		art.set_abstract("this is a long abstract, longer than the article.");
		art.set_title("sefile");

		table.insert(&art);
	}
}

int main(int argc, char **argv) {

	static const char *db_name = "./wikipedia.db";
	static const long BUFFER_SIZE = 20971520;

	char *memblock;
//	remove(db_name);
	char *title_file = NULL;
	title_buffer[0] = '\0';

	if (argc > 1)
		title_file = argv[1];

	if (argc > 2)
		db_name = argv[2];

	db_sqlite db;

	db.create_in_memory_database();

	db.load_database_into_memory(db_name);

	if (db.is_connected()) {
		table_article table(&db);
		table.create();

		rows_count = db.count_rows(table_article::TABLE_NAME.c_str());

		db.begin_transaction();

		cerr << "current table has " << rows_count << " records." << endl;

	//		table.clear();

		if (title_file != NULL) {
			long bytes_read = 0;
			ifstream file;
			file.open(title_file,  ios::in|ios::binary|ios::ate);

			if (file.is_open()) {
				file.seekg (0, ios::beg);
				long file_size = file.tellg();
				long buffer_size = file_size > BUFFER_SIZE ? BUFFER_SIZE : file_size;

				memblock = new char [static_cast<int>(buffer_size)];
//					file.read (memblock, buffer_size);
//					bytes_read += buffer_size;
//				}
//				else {
//				memblock = new char [static_cast<int>(file_size) + 1];
//				file.read (memblock, file_size);
//				bytes_read = file_size;
//				memblock[file_size] = '\0';
//				}
//
//			  process(memblock, db, table);
				int count = 0;
				while (bytes_read < file_size) {
					file.read (memblock, buffer_size);
					bytes_read += buffer_size;

					process(memblock, buffer_size, db, table, count++);

					if ((file_size - bytes_read) < buffer_size)
					  buffer_size = file_size - bytes_read;
				}
			}
		}
		else {
			cerr << "stdin not implemented yet!" << endl;
		}

		db.end_transaction();

		string new_name = increase_file_num(db_name);
		cerr << "saving in-memory database to file: " << new_name << endl;

		db.save_in_memory_database_to_file(new_name.c_str());
	}
	else
		cerr << "could not connect to the database." << endl;

	db.close();

	return -1;
}
