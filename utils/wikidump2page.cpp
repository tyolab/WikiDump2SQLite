/*
 * wikidump2page.cpp
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

#include <string>
#include <iostream>
#include <fstream>
#include <exception>
#include <cmath>
#include <cassert>

#include <sstream>
#include <iomanip>

#include "string_utils.h"
#include "sys_file.h"

#define ARTICLE_ID_SIGNITURE "<id>"
#define ARTICLE_NAME_SIGNITURE "title"

using namespace std;

static const long ONE_M = 1024 * 1024;
static const long TAR_BLOCK_SIZE = ONE_M * 2;
static const long TARGET_BLOCK_SIZE = ONE_M * 512;

static const char *DEFAULT_RAW_PATH = "raw/";

char buffer[TAR_BLOCK_SIZE + 1];
string outpath(DEFAULT_RAW_PATH);
//const char *buffer = buffer;
int created_files_num = 0;

/*
 * From Andrew Trotman's code
 */
inline static long get_doc_id(const char *file)
{
const char *pos;

pos = strstr(file, ARTICLE_ID_SIGNITURE);
if (pos == NULL)
	{
	fprintf(stderr, "Cannot find DOC id <name id=...> or <id> in file\n");
	return -1;
	}
pos += strlen(ARTICLE_ID_SIGNITURE);
while (!isdigit(*pos))
	pos++;

return atol(pos);
}

std::string id2parentdir(unsigned long id)
{
	long last3 = id >= 1000 ? (id - (id /1000) * 1000) : id;
	std::ostringstream stm;
	stm << std::setw(3) << std::setfill('0') << last3 << sys_file::SEPARATOR;
	return stm.str();
}

std::string id2dir(unsigned long id)
{
	std::ostringstream stm;
	stm << id2parentdir(id) << id;
	return stm.str();
}

void compress_split(char *source, char *dest, unsigned long ONE_M, unsigned long len, string &filename)
{
	fprintf(stderr, "Creating a split file %s.\n", filename.c_str());
	static ANT_compress_text_bz2 compressor;
	unsigned long after_length = ONE_M;
	if (!compressor.compress((char *)dest, &after_length, (char *)source, len))
		throw "Compression error \n";
	ofstream my_file(filename/*, ios::out | ios::binary | ios::beg*/);
	my_file.write((char *)dest, after_length);
	my_file.close();
}

void create_dirs() {
	string parentdir = outpath; // + "/" + DEFAULT_RAW_PATH;
	if (!sys_file::exist(parentdir.c_str()))
		sys_file::create_directory(parentdir.c_str());
	for (int i = 0; i < 1000; ++i) {
		string dirname = parentdir + id2parentdir(i);
		if (!sys_file::exist(dirname.c_str()))
			sys_file::create_directory(dirname.c_str());
	}

}

void create_page(const char *start, const char *end) {
	long id = get_doc_id(start);
	string filename;
//	char dirname[ONE_M];

	if (id > -1) {
		if (id == 21501)
			cerr << "got you" << endl;

		filename = outpath + id2dir(id) + ".xml";
//		strcpy(dirname, filename.c_str());
//		if (sys_file::exist(filename.c_str()))
//			cerr << "already created file: " << filename << endl;

		FILE *handler = fopen(filename.c_str(), "w+");
		if (!handler) {
			cerr << "can't create file:" << filename << endl;
		}
		else {
			fwrite(start, end - start, 1, handler);
			fclose(handler);
			++created_files_num;
			if ((created_files_num % 50000) == 0)
				cerr << created_files_num << " files created" << endl;
		}
	}
	else
		cerr << "Invalid id found" << endl;
}

const char *read_buffer(bool last_piece) {
//	const char *pre = buffer;
	// go to the last </page> tag
	const char *pos = buffer;
	const char *tag_start = NULL;
	const char *tag_end = NULL;

	if (strstr(buffer, "MediaWiki:Recentchanges-legend-bot"))
		cerr << "got you" << endl;

	while ((tag_start = strstr(pos, "<page>"))) {
//		tag_start += 6;

		tag_end = strstr(tag_start + 6, "</page>");
		if (tag_end != NULL) {
			tag_end += 7;
			create_page(tag_start, tag_end);
		}
		else {
			if (last_piece)
				cerr << pos << endl;
			break;
		}
		pos = tag_end;
	}
	return pos;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s input /a/path/for/output", argv[0]);
		exit(-1);
	}

//	int number_of_split = 1;
//	const char *outpath = ".";

	ANT_memory file_buffer(ONE_M);
	ANT_instream *file_stream = NULL, *decompressor = NULL, *instream_buffer = NULL;

	file_stream = new ANT_instream_file(&file_buffer, argv[1]);
	decompressor = new ANT_instream_bz2(&file_buffer, file_stream);
	instream_buffer = new ANT_instream_buffer(&file_buffer, decompressor);

	if (argc > 2) {
//		number_of_split = atoi(argv[2]);
//
//		if (number_of_split < 0)
//			number_of_split = 1;
//
//		long split_size = 0;
//		if (number_of_split > 1) {
//			split_size = ceil(sys_file::size(argv[1]) / number_of_split);
//			//number_of_split = ceil(file_stream->length() / split_size);
//			TARGET_BLOCK_SIZE = split_size * 10;
//		}
		//
		outpath = string(argv[2]);
	}

	create_dirs();

	assert(TARGET_BLOCK_SIZE > 0);
	ANT_instream *source = instream_buffer;
	//ANT_directory_iterator_tar_file_header header;

//	char target_buffer[ONE_M * 10];
//	char buffer_source[TARGET_BLOCK_SIZE + 1];
//	char buffer_target[TARGET_BLOCK_SIZE + 1];
//	char *buffer_source = (char *)malloc(sizeof(char) * (TARGET_BLOCK_SIZE + 1)); //new char[TARGET_BLOCK_SIZE + 1];
//	char *buffer_target = (char *)malloc(sizeof(char) * (TARGET_BLOCK_SIZE + 1)); //new char[TARGET_BLOCK_SIZE + 1];
	long long length_of_file_in_bytes;
	long long bytes_read = 0;
	long long total_read = 0;
	long long bytes_left = 0;

	long long size;
	memset(buffer, 0, sizeof(buffer));
//	const char *current;

//	const string root_tag_closed("\n</mediawiki>");
	//source->read(buffer, TAR_BLOCK_SIZE);

//	size = source->read((unsigned char *)buffer, TAR_BLOCK_SIZE);
//	buffer[size] = '\0';
//	char *pos_tag_open = strchr((char *)buffer, '>');
//	string root_tag_open(buffer, ++pos_tag_open);


	/*
	 * Step 2. copy read buffer to the compression buffer
	 */
//	current = (char *)memory_copy(buffer_source, buffer, size);
//	current += size;
	//++current;

//	bytes_read += size;
//	total_read += size;
//	string split_name(argv[1]);
//	char *pos = NULL;
//	char *pre = NULL;
	 {
		/*
		 * Step 3. continue to read into compression buffer until full
		 */
		char *buffer_pos = (char *)buffer;
		long size_to_read = TAR_BLOCK_SIZE;
		while ((size = source->read((unsigned char *)buffer_pos, size_to_read)) > 0) {
			buffer[size] = '\0';
//			bytes_read += size;
			total_read += size;

			buffer_pos = (char *)read_buffer(false);
			if (buffer_pos == buffer) {
				cerr << "Finished reading file or the buffer size is too small!" << endl;
				break;
			}
			bytes_read = buffer_pos - buffer;
			bytes_left = size - bytes_read;
			size_to_read = TAR_BLOCK_SIZE - bytes_left;
			memmove(buffer, buffer_pos, bytes_left);
			buffer_pos = buffer + bytes_left;
			buffer[bytes_left] = '\0';
//			if ((TARGET_BLOCK_SIZE - bytes_read) <= (TAR_BLOCK_SIZE * 3)) {
//			while ((current - buffer) < size) {
//				pre = current;
//				// go to the last </page> tag
//				pos = current;
//				if ((pre = strstr(pre, "<page>"))) {
//					current
//					if ((pos = strstr(current, "</page>"))) {
//						pos += 7;
//						create_page(pre, pos);
////						pre = pos;
//					}
//				}
//				else
//
//
////				continue;
//			}
//			if ((TARGET_BLOCK_SIZE - bytes_read) <= (TAR_BLOCK_SIZE * 3)) {
//
//
//				/*
//				 * handle the content before </page>
//				 */
//				if (tag_end == NULL) {
//					pos = pre;
//					long temp_size = pos - buffer;
//					long left_size = size - temp_size;
//					memory_copy(current, buffer, temp_size);
//					current += temp_size;
//					memory_copy(current, root_tag_closed.c_str(), root_tag_closed.length());
//					current += root_tag_closed.length();
//					*current = '\0';
//
//					split_name = increase_file_num(split_name, true);
//
//					// compress
//					compress_split(buffer_source, buffer_target, TARGET_BLOCK_SIZE + 1, bytes_read - left_size + root_tag_closed.length(), split_name);
//
//					//now deal with the leftover, move the leftover to the front
//					//memmove(buffer, pos, left_size);
//					strcpy(buffer_source, root_tag_open.c_str());
//					current = buffer_source + root_tag_open.length();
//					memory_copy(current, pos, left_size);
//					current += left_size;
//					buffer[0] = '\0';
//					*current = '\0';
//					bytes_read = left_size + root_tag_open.length();
//				}
//				else
//					throw "The buffer size is too small to fit an article.";
//
//				continue;
//			}
		}
		read_buffer(true);
//			memory_copy(current, buffer, size);
//			current += size;
			//cout << buffer;
	}

//		if (bytes_read > 0/**buffer_source != '\0'*/) {
//			*current = '\0';
//			split_name = increase_file_num(split_name, true);
//			compress_split(buffer_source, buffer_target, TARGET_BLOCK_SIZE + 1, bytes_read, split_name);
//			//puts(buffer);
//		}


//	*(++current) = '\0';

	delete file_stream;
	delete decompressor;
	delete instream_buffer;

//	free(buffer_source);
//	free(buffer_target);
	return 0;
}
