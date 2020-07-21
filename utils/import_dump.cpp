/*
 * dump2indexable.cpp
 *
 *  Created on: 30/06/2014
 *      Author: monfee
 */

/*
 * import_dump.cpp
 *
 *  Created on: Sep 17, 2010
 *      Author: monfee
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

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
#include "article.h"
#include "console.h"
#include "compression.h"
#include "task_thread.h"
#include "task_manager.h"
#include "wiki_api.h"
#include "file.h"

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

ANT_directory_iterator_file_buffered *buffered_file_iterator = NULL;
ANT_instream *file_stream = NULL, *decompressor = NULL, *instream_buffer = NULL;

int article_id;
//int task_manager::should_stop = FALSE;
task_manager* thread_manager_ptr;

#define BUFFER_SIZE (1 * 1024 * 1024)

ANT_memory file_buffer(BUFFER_SIZE);

int bucket_size = 100;
const int header_size = BUFFER_SIZE;
char *xml_dump_start = NULL;
const char *xml_dump_end = "</mediawiki>";

string temp_path = "/tmp";
string import_db;
string import_cmd_suffix = "php importDump.php --conf ";
string lang;

//class import_job : public task_thread {
//
//};

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
	if (xml_dump_start)
		delete [] xml_dump_start;

	if (thread_manager_ptr)
		thread_manager_ptr->stop();

	clear();

	exit(-1);
}

void read_file(char* file) {
	string input_name = file;
	file_stream = new ANT_instream_file(&file_buffer, file);
	if (ends_with(input_name, ".bz2")) {
		decompressor = new ANT_instream_bz2(&file_buffer, file_stream);
		instream_buffer = new ANT_instream_buffer(&file_buffer, decompressor);
	}
	else {
		instream_buffer = new ANT_instream_buffer(&file_buffer, file_stream);
	}
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

tyo::file* init_file(int thread_id) {
	static const char *name = "mediawiki-import-dump";
	stringstream name_str;
	name_str << temp_path << "/" << (name) << thread_id << ".xml";

	tyo::file* file = new tyo::file();
	file->open(name_str.str().c_str(), "w");

		// set buffer size
	file->setBufferSize(100 * 1024 * 1024);
	file->writeString(xml_dump_start);
	file->writeString("\n");
	return file;
}

void import(tyo::file* file) {
	file->write(xml_dump_end, strlen(xml_dump_end));
	file->close();

	// now run the importDump.php
	string import_cmd = "export MEDIAWIKI_EXCLUDE_EXTENSIONS=true; " + import_cmd_suffix + "  /data/mediawiki/all/" + lang + "wiki/LocalSettings.php " + file->get_filename() + " " + import_db;
	get_console_out(import_cmd);
}

void process(int thread_id, task_manager *tm_ptr) {
	cerr << "start article processing thread #" << thread_id/*std::this_thread::get_id()*/ << endl << flush;

	article *art_ptr = NULL;
	std::thread::id id = std::this_thread::get_id();
	tm_ptr->signin(thread_id);
	int count = 0;

	tyo::file *file = init_file(thread_id);

	while ((art_ptr = tm_ptr->get_aticle_to_parse()) != NULL) {

		file->writeString(art_ptr->get_article());

		++count;

		if (count >= bucket_size) {
			import(file);

			delete file;
			file = init_file(thread_id);
			count = 0;
		}

		// delete the article in the database thread
		// after it is inserted into table
		//delete art_ptr;

		if (count % 500 == 0 && thread_id == 0)
			fprintf(stderr, "parsed %d files.\n", count);

	}

	if (count > 0) {
		import(file);
		delete file;
	}

	fprintf(stderr, "received stop signal, article parsing thread #%d existing...\n", thread_id);
	tm_ptr->signoff(thread_id);
}

void usage(char *program) {
	fprintf(stderr, "Usage: %s [-threads #] [-p /a/path/to/temporay/folder] [-n number of page per bucket] input #\n", program);
	exit(-1);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		usage(argv[0]);
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR)
		fprintf(stderr, "\ncan't catch SIGINT\n");

	if (signal(SIGTERM, sig_handler) == SIG_ERR)
		fprintf(stderr, "\ncan't catch SIGINT\n");

	if (signal(SIGKILL, sig_handler) == SIG_ERR)
		fprintf(stderr, "\ncan't catch SIGKILL\n");

	int param = 1;

	int fix_mode = 0;

	int use_wikipedia = 0;
	bool use_pipe = FALSE;

	task_manager manager;
	thread_manager_ptr = &manager;

	for (; param < argc; ++param) {
		char *opt;
		if (argv[param][0] == '-') {
			if (strlen(argv[param]) == 1) {
				use_pipe = TRUE;
				continue;
			}

			opt = argv[param] + 1;
			if (strcmp(opt, "threads") == 0) {
				++param;
				int num = atoi(argv[param]);
				if (num > 1)
					manager.set_thread_number(num);
			}
			else if (strcmp(opt, "n") == 0) {
				++param;
				int num = atoi(argv[param]);
				if (num > 1)
					bucket_size = (num);
			}
			else if (strcmp(opt, "path") == 0) {
				++param;
				temp_path = (argv[param]);
			}
			else if (strcmp(opt, "fix") == 0) {
				fix_mode = 1;
			}
			else if (strcmp(opt, "pipe") == 0) {
				use_pipe = TRUE;
			}
			else if (strcmp(opt, "lang") == 0) {
				lang = (argv[++param]);
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

	FILE* fp = stdin;

	if (use_pipe) {
		/* code */
		printf("Reading data from pipe");
		title_file = "/dev/stdin";
	}
	else
	{
		if (param >= argc /*|| import_db.length() == 0*/)
			usage(argv[0]);

		if (param < argc) {
			title_file = argv[param++];
			char *tmp = strdup(title_file);
			char *file = basename(tmp);
			char *pos = strchr(file, '-');
			std::string root = string(file, pos - file);
			lang = root.substr(0, root.size() - 4);
			if (use_wikipedia) {
				wiki_api::root = "wiki";
				wiki_api::host = std::string("http://") + lang + ".wikipedia.org/";
			}
			else {
				wiki_api::root = "/";
				wiki_api::host = std::string("http://") + lang + "wiki";
			}

			if (import_db.length() == 0)
				import_db = "wikipedia_" + lang;

			wiki_api::initialize();
			fprintf(stderr, "wiki api: %s\n", wiki_api::url_template.c_str());
			free(tmp);
		}
	}
	

	/*
	 * set up worker function
	 */
	runnable::set_run_func((void (*)(int, ...))process);
//	task_thread::set_task_func(&process_aritcle);


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
	 * 2. get the wikipedia dump xml structure
	 */
	xml_dump_start = new char[BUFFER_SIZE];
	ANT_directory_iterator_object file_object, *current_file;

	string input_name = title_file;
	file_stream = new ANT_instream_file(&file_buffer, title_file);
	ANT_instream *in = NULL;
	if (ends_with(input_name, ".bz2")) {
		in = new ANT_instream_bz2(&file_buffer, file_stream);
	}
	else {
		in = new ANT_instream_buffer(&file_buffer, file_stream);
	}
	in->read(xml_dump_start, header_size);
	string snippet(xml_dump_start, 100000);
	std::cout << snippet << std::endl;

	char *pos = strstr(xml_dump_start, "<page");
	int start_len = 0;

	delete in;

	if (pos != NULL) {
		clear();

		start_len = pos - xml_dump_start;
		*pos = '\0';
	}
	else {
		puts("couldn't find head information. Maybe you could try the uncompressed file\n");
		quit();
	}

	/*
	 * 2. now read in the dump file
	 */
	read_file(title_file);

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
	thread_manager_ptr->start();

	/*
	 * 4. insert full page
	 */
//	if (false)
	try {
		int where_we_up_to = -1;
		int where_we_stop = -1;

//		if (fix_mode && where_we_up_to == dbs.get_current_db()->get_min())
//			fix_mode = false;

		int do_it = FALSE;

		current_file = disk->first(&file_object);

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

//			article_id = atoi(current_file->filename);

			if (where_we_stop < 0 || file_count <= where_we_stop){

				if (!do_it && file_count >= where_we_up_to)
					do_it = TRUE;

	//				continue;
				if (do_it) {
					article *art_ptr = new article();
					art_ptr->set_article(strdup(current_file->file));

					if (!thread_manager_ptr->is_full(file_count))
						thread_manager_ptr->add_article_for_parsing(art_ptr);
				}
				else
					thread_manager_ptr->should_stop = TRUE;
				// do something with the article here

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

	}

	thread_manager_ptr->stop();

	quit();

	return 0;
}



