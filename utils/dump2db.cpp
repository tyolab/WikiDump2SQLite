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
#include <signal.h>

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
#include <thread>

#include "string_utils.h"
#include "sys_file.h"
#include "xml.h"
#include "urlcode.h"

#include "db_sqlite.h"
#include "article.h"
#include "table_article.h"
#include "console.h"
#include "compression.h"
#include "wiki_db.h"
#include "wiki_db_split.h"
#include "article_operation_thread.h"
#include "thread_manager.h"
#include "wiki_api.h"

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef TRUE
	#define TRUE (!FALSE)
#endif

#ifndef NULL
	#define NULL 0
#endif

using namespace std;

//int thread_manager::multi_threads_mode = TRUE;

thread_manager *thread_manager_ptr = NULL;
wiki_db_split& dbs = wiki_db_split::instance();
ANT_directory_iterator_file_buffered *buffered_file_iterator = NULL;
ANT_instream *file_stream = NULL, *decompressor = NULL, *instream_buffer = NULL;

int article_id;
//int thread_manager_ptr->should_stop = FALSE;

string import_cmd_suffix = "import.sh --conf ";

void clear() {
	if (buffered_file_iterator)
		delete buffered_file_iterator;
	if (instream_buffer)
		delete instream_buffer;
	if (decompressor)
		delete decompressor;
	if (file_stream)
		delete file_stream;
}

void quit() {
	if (thread_manager_ptr)
		thread_manager_ptr->stop();

	clear();

	exit(-1);
}

void sig_handler(int signo)
{
	if (signo == SIGINT)
		fputs("received SIGINT\n", stderr);

	if (signo == SIGTERM)
		fputs("received SIGTERM\n", stderr);

	if (signo == SIGKILL)
		fputs("received SIGKILL\n", stderr);

	thread_manager_ptr->should_stop = TRUE;
	quit();
}

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

void usage(char *program) {
	fprintf(stderr, "Usage: %s input #\n", program);
	exit(-1);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		usage(argv[0]);
	}

	static const char *db_name = "./wikipedia.db";

	if (signal(SIGINT, sig_handler) == SIG_ERR)
		fprintf(stderr, "\ncan't catch SIGINT\n");

	if (signal(SIGTERM, sig_handler) == SIG_ERR)
		fprintf(stderr, "\ncan't catch SIGINT\n");

	if (signal(SIGKILL, sig_handler) == SIG_ERR)
		fprintf(stderr, "\ncan't catch SIGKILL\n");

	int param = 1;

	int fix_mode = 0;

	int use_wikipedia = 0;

	for (; param < argc; ++param) {
		char *opt;
		if (argv[param][0] == '-') {
			opt = argv[param] + 1;
			if (strcmp(opt, "thread") == 0) {
				++param;
				int num = atoi(argv[param]);
				if (num > 1)
					thread_manager::instance().set_thread_number(num);
				else
					thread_manager::multi_threads_mode = FALSE;
			}
			else if (strcmp(opt, "milestone") == 0) {
				++param;
				int num = atoi(argv[param]);
				if (num > 0)
					wiki_db_split::milestone = num * 10000;
			}
			else if (strcmp(opt, "fix") == 0) {
				fix_mode = 1;
				thread_manager::multi_threads_mode = FALSE;
			}
			else if (strcmp(opt, "usewikipedia") == 0) {
				use_wikipedia = 1;
			}
			else {
				fprintf(stderr, "unknown option: %s \n", argv[param]);
				usage(argv[0]);
			}
		}
		else
			break;
	}

	char *memblock;
//	remove(db_name);
	char *title_file = NULL;

	int file_count = 0;
	int article_count = 0;

	if (param < argc) {
		title_file = argv[param++];
		char *tmp = strdup(title_file);
		char *file = basename(tmp);
		char *pos = strchr(file, '-');
		std::string root = string(file, pos - file);
		std::string lang = root.substr(0, root.size() - 4);
		if (use_wikipedia) {
			wiki_api::root = "wiki";
			wiki_api::host = std::string("http://") + lang + ".wikipedia.org/";
		}
		else
			wiki_api::root = root;

		wiki_api::initialize();
		fprintf(stderr, "wiki api: %s\n", wiki_api::url_template.c_str());
		free(tmp);
	}
	else
		usage(argv[0]);

	if (param < argc)
		dbs.set_base_name(argv[param]);

	if (fix_mode)
		dbs.set_up_to(-1);

	dbs.load_db();


//	if (argc > 3)
//		thread_manager::multi_threads_mode = atoi(argv[3]);

//	if (db.is_ready()) {

		/*
		 * 1. init article table
		 */
//		table_article table(db);
//		table.create();

//		article_operation_thread::article_table_ptr = &table;

	/*
	 * 2. wikipedia dump
	 */
	ANT_directory_iterator_object file_object, *current_file;

	ANT_memory file_buffer(1024 * 1024);

	string input_name = title_file;
	file_stream = new ANT_instream_file(&file_buffer, title_file);
	if (ends_with(input_name, ".bz2")) {
		decompressor = new ANT_instream_bz2(&file_buffer, file_stream);
		instream_buffer = new ANT_instream_buffer(&file_buffer, decompressor);
	}
	else {
		instream_buffer = new ANT_instream_buffer(&file_buffer, file_stream);
	}

	buffered_file_iterator = new ANT_directory_iterator_file_buffered(instream_buffer, ANT_directory_iterator::READ_FILE);

	buffered_file_iterator->set_tags("page", "id");

//	ANT_instream *source;
	ANT_directory_iterator *source = NULL;
	source = buffered_file_iterator;

	ANT_directory_iterator *disk = NULL;

	disk = source;

	/*
	 * 3. create thread pool
	 *
	 */
	if (thread_manager::multi_threads_mode) {
		thread_manager& tm = thread_manager::instance();
		thread_manager_ptr = &tm;
		tm.start();

		/*
		 *
		 */
		thread_manager::instance().check_if_db_ready();
//			tm.start_workers();

//			runnable::set_run_func(&wiki_db::process);
//			db.start(0);
	}
	else {
		dbs.get_current_db()->set_in_memory(1);
		dbs.get_current_db()->open();
		dbs.get_current_db()->begin_transaction();
	}

	/*
	 * 4. insert full page
	 */
	try {
		int where_we_up_to = dbs.where_we_are_up_to();
		int where_we_stop = dbs.where_we_stop();

//		if (fix_mode && where_we_up_to == dbs.get_current_db()->get_min())
//			fix_mode = false;

		int do_it = FALSE;

		current_file = disk->first(&file_object);

		int max = wiki_db_split::instance().get_current_db()->get_max();

		if (where_we_up_to == 0)
			do_it = TRUE;

		while (current_file != NULL)
			{
			/*
				 It could b			for (int i = 0; i < POOL_SIZE; ++i) {
				workers[i].join();
			}e an empty file without any text (content)
			 */
			++file_count;
			if (current_file->file == NULL)
				continue;

			article_id = atoi(current_file->filename);

			if (file_count <= where_we_stop){

				if (!do_it && file_count >= where_we_up_to)
					do_it = TRUE;

				if (do_it && strstr(current_file->file, "<redirect") == NULL) {
	//				continue;

					article *art_ptr = new article();
					art_ptr->set_in_fix_mode(1);
					art_ptr->set_id(file_count); // the count is the id, don't use it in the dump2indexable, but ok here

				/* only needs for once */
	//			if (strstr(current_file->file, "<redirect") == NULL) {
//					if (article_id == 77202)
//						fputs("stop here.\n", stderr);
					char *start = NULL, *finish = current_file->file;
					between(current_file->file, "title", "title", &start, &finish);

					if (start && finish) {
						string found_title(start, finish - start);
						art_ptr->set_title(found_title.c_str());
			//			char *encoded = url_encode(title);
	//					strcpy(title, found_title.c_str());
					}

					int blob_len = 0;

					art_ptr->set_article_id(article_id);
					art_ptr->set_article(current_file->file);

					if (thread_manager::multi_threads_mode) {
		//				while(1) {
							if (!thread_manager::instance().is_full(file_count)) {
								thread_manager::instance().add_article_for_parsing(art_ptr);
		//						break;
							}
		//					else {
		//						cerr << "[MAIN] The queue is full, sleep for a while." << endl;
		//						sleep(300);
		//					}
		//				}
					}
					else {
//						thread_manager::instance().add_article_for_parsing(art_ptr);
//						static wiki_api api;
						dbs.check_if_switching_db_needed(art_ptr);
						article_operation_thread::process_article(art_ptr);
						if (art_ptr->get_result() && dbs.get_current_db()) {
							dbs.get_current_db()->set_something_changed(1);
							dbs.get_current_db()->update_article(art_ptr);
						}
						delete art_ptr;
					}

					if (fix_mode) {
						do_it = FALSE;
						if (!thread_manager::multi_threads_mode || thread_manager::instance().has_file_updated(file_count))
							while (dbs.get_current_db() != 0 && (where_we_up_to = dbs.get_current_db()->get_article_table()->get_last_update()) == 0)
								dbs.switch_db();

						if (where_we_up_to == 0 || dbs.get_current_db() == 0)
							thread_manager_ptr->should_stop = TRUE;
					}
		//			free(encoded);
				}
				if (file_count % 10000 == 0)
					cerr << file_count << " files processed." << endl;

	//			delete [] title;
			}
			else
				thread_manager_ptr->should_stop = TRUE;

			delete [] current_file->file;
			delete [] current_file->filename;

			if (thread_manager_ptr->should_stop)
				break;

			current_file = disk->next(&file_object);
		}

	//	*(++current) = '\0';
	}
	catch (...) {
//		if (thread_manager_ptr)
//			thread_manager_ptr->stop();
//		throw;
	}

	if (!thread_manager::multi_threads_mode && dbs.get_current_db() != NULL) {
		dbs.get_current_db()->end_transaction();
		dbs.get_current_db()->write_to_disk();
		dbs.get_current_db()->close();
	}

	quit();

//		string new_name = increase_file_num(db_name);
//		cerr << "saving in-memory database to file: " << new_name << endl;
//		}
//		else {
//			cerr << "Empty wikipedia table." << endl;
//		}
//	}
//	else
//		cerr << "could not connect to the database." << endl;

//	free(buffer_source);
//	free(buffer_target);
	return 0;
}



