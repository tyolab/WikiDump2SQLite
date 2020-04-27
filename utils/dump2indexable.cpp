/*
 * dump2indexable.cpp
 *
 *  Created on: 30/06/2014
 *      Author: monfee
 */

/*
 * split_wikdump.cpp
 *
 *  Created on: Sep 17, 2010
 *      Author: monfee
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <instream.h>
#include <instream_buffer.h>
#include <instream_file.h>
#include <instream_bz2.h>
#include <memory.h>
#include <compress_text_bz2.h>
#include <directory_iterator_file_buffered.h>
#include <directory_iterator_object.h>

#include <string>
#include <iostream>
#include <fstream>
#include <exception>
#include <cmath>
#include <cassert>

#include "string_utils.h"
#include "sys_file.h"
#include "xml.h"

#include "db_sqlite.h"
#include "article.h"
#include "table_article.h"
#include "wiki_db_split.h"
#include "wiki_db.h"

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef TRUE
	#define TRUE (!FALSE)
#endif

using namespace std;

void compress_split(char *source, char *dest, unsigned long buffer_size, unsigned long len, string &filename)
{
	fprintf(stderr, "Creating a split file %s.\n", filename.c_str());
	static ANT_compress_text_bz2 compressor;
	unsigned long after_length = buffer_size;
	if (!compressor.compress((char *)dest, &after_length, (char *)source, len))
		throw "Compression error \n";
	ofstream my_file(filename/*, ios::out | ios::binary | ios::beg*/);
	my_file.write((char *)dest, after_length);
	my_file.close();
}

void usage(const char *program) {
	fprintf(stderr, "Usage: %s input #\n", program);
	exit(-1);
}

int main(int argc, char **argv)
{

	static const char *db_name = "./wikipedia0.db";

	int param = 1;
	int to_split = TRUE;

	for (; param < argc; ++param) {
		char *opt;
		if (argv[param][0] == '-') {
			opt = argv[param] + 1;
			if (strcmp(opt, "-nosplit") == 0) {
				to_split = FALSE;
			}
		}
		else
			break;
	}

	wiki_db_split dbs;
	char *input;

	if (param < argc)
		input = argv[param++];
	else
		usage(argv[0]);

	if (param < argc)
		dbs.set_base_name(argv[param]);

//	db_sqlite db;
//
//	db.create_in_memory_database();
//
//	db.attach_database(db_name);
	wiki_db *main = dbs.create_main_db();

	if (to_split) {
		main->set_max(main->get_limit());
		main->set_min(1);
		main->set_in_memory(1);
		main->open();
	}
	wiki_db *current = main;

//	if (db.is_connected()) {

		/*
		 * 1. init article table
		 */
//		table_article table(&db);
//		table.create();

//		int rows_count = db.count_rows(table_article::TABLE_NAME.c_str());

//		db.begin_transaction();

//		cerr << "current table has " << rows_count << " records." << endl;

		/*
		 * 2. wikipedia dump
		 */
		ANT_directory_iterator_object file_object, *current_file;

		ANT_memory file_buffer(1024 * 1024);
		ANT_instream *file_stream = NULL, *decompressor = NULL, *instream_buffer = NULL;

		string input_name = input;
		file_stream = new ANT_instream_file(&file_buffer, input);
		if (ends_with(input_name, ".bz2")) {
			decompressor = new ANT_instream_bz2(&file_buffer, file_stream);
			instream_buffer = new ANT_instream_buffer(&file_buffer, decompressor);
		}
		else {
			instream_buffer = new ANT_instream_buffer(&file_buffer, file_stream);
		}

		ANT_directory_iterator_file_buffered *buffered_file_iterator = new ANT_directory_iterator_file_buffered(instream_buffer, ANT_directory_iterator::READ_FILE);

		buffered_file_iterator->set_tags("page", "id");

	//	ANT_instream *source;
		ANT_directory_iterator *source = NULL;
		source = buffered_file_iterator;

		ANT_directory_iterator *disk = NULL;

		disk = source;

		/*
		 * 3. insert page basic information
		 */
		current_file = disk->first(&file_object);

		int file_count = 0;
		int article_count = 0;
		while (current_file != NULL)
			{
			/*
				 It could be an empty file without any text (content)
			 */
			if (current_file->file == NULL)
				continue;

			++file_count;

			char *ns = between(current_file->file, "ns");
			int ns_id = ns ? atoi(ns) : -99;
			if (ns_id == 0)
				{
				++article_count;

				if (to_split && article_count > current->get_max()) {
					cerr << "spliting database..." << endl;
					if (current->get_index() > 0) {
						current->end_transaction();
						current->write_to_disk();
						current->close();
					}
					dbs.create_new_db();
					wiki_db *tmp = dbs.get_current_db();
					tmp->set_max(current->get_max() + tmp->get_limit());
					tmp->set_min(current->get_max() + 1);

					current = tmp;
					current->set_in_memory(1);
					current->open();
					current->begin_transaction();
				}

				article art = article(article::MODE_COMPRESSED_GZIP);
				char *title = NULL;

				title = between(current_file->file, "title");
				art.set_title(title);
				unscape_xml(art.get_title());

				if (table_article::table_version == table_article::TABLE_VERSION_V1) {
					char *start = NULL, *finish = current_file->file;
					do {
						between(finish, "[[Category:", "]]", &start, &finish, 0);
						if (start) {
							string category = string(start, finish - start);
							art.set_category(category);
						}

					} while (start && finish);
				}

				art.set_id(article_count);
				art.set_article_id(atoi(current_file->filename));

				const char *start = NULL;
				if ((start = strstr(current_file->file, "<redirect")) != NULL) {
					art.process_redirect_text(start);
					art.set_redirect(1);
				}

				/*
				 * INSERT INTO TABLE
				 */
				current->get_article_table()->insert(&art);
				main->get_title_table()->insert(article_count, title);
//				art.set_article("I am not a superman");
		//		art.set_abstract("this is a long abstract, longer than the article.");

				puts(current_file->file);

				if (title != NULL ) delete [] title;

				delete [] current_file->file;
				delete [] current_file->filename;


				if (article_count % 500 == 0)
					cerr << article_count << " articles stored." << endl;
				}


			current_file = disk->next(&file_object);
			}


		current->end_transaction();
		current->write_to_disk();
		current->close();

		if (main->get_index() != current->get_index()) {
			main->end_transaction();
			main->write_to_disk();
			main->close();
		}



	//	*(++current) = '\0';
		delete buffered_file_iterator;
		delete file_stream;
		delete decompressor;
		delete instream_buffer;

//		db.end_transaction();
//
////		string new_name = increase_file_num(db_name);
////		cerr << "saving in-memory database to file: " << new_name << endl;
//
//		db.save_in_memory_database_to_file(db_name);
//	}
//	else
//		cerr << "could not connect to the database." << endl;

//	db.close();
//	free(buffer_source);
//	free(buffer_target);
	return 0;
}



