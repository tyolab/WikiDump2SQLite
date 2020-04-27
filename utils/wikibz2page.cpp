/*
 * wikibz2page.cpp
 *
 *  Created on: Sep 17, 2010
 *      Author: monfee
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bzlib.h>

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
static const long TAR_BLOCK_SIZE = ONE_M * 10;
static const long TARGET_BLOCK_SIZE = ONE_M * 512;

static const char *DEFAULT_RAW_PATH = "raw/";

static char buffer[TAR_BLOCK_SIZE + 1];
static char buffer_page[TAR_BLOCK_SIZE + 1];
bool not_a_full_page = false;
FILE *handler;
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

//void compress_split(char *source, char *dest, unsigned long ONE_M, unsigned long len, string &filename)
//{
//	fprintf(stderr, "Creating a split file %s.\n", filename.c_str());
//	static ANT_compress_text_bz2 compressor;
//	unsigned long after_length = ONE_M;
//	if (!compressor.compress((char *)dest, &after_length, (char *)source, len))
//		throw "Compression error \n";
//	ofstream my_file(filename/*, ios::out | ios::binary | ios::beg*/);
//	my_file.write((char *)dest, after_length);
//	my_file.close();
//}

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
		filename = outpath + id2dir(id) + ".xml";
//		strcpy(dirname, filename.c_str());
//		if (sys_file::exist(filename.c_str()))
//			cerr << "already created file: " << filename << endl;

		handler = fopen(filename.c_str(), "w+");
		if (!handler) {
			cerr << "can't create file:" << filename << endl;
		}
		else {
//			cerr << "writing to file: " << filename << endl;
			int size_to_write = end - start;
			int size_written = fwrite(start, 1, size_to_write, handler);
			if (size_to_write != size_written)
				cerr << "short write for " << filename << endl;

			fclose(handler);
			++created_files_num;
			if ((created_files_num % 50000) == 0)
				cerr << created_files_num << " files created" << endl;
		}
	}
	else
		cerr << "Invalid id found" << endl;
}

void check_last_page() {
	const char *tag_end_pre = NULL;
	const char *tag_end = buffer;

	while ((tag_end = strstr(tag_end, "</page>"))) {
		tag_end += 7;
		tag_end_pre = tag_end;
	}

	if (tag_end_pre)
		cerr << "End of file: " << tag_end_pre << endl;
}

const char *read_buffer(long size) {
//	const char *pre = buffer;
	// go to the last </page> tag
	const char *pos = buffer;
	const char *tag_start = NULL;
	const char *tag_end = NULL;

//	if (strstr(buffer, "MediaWiki:Recentchanges-legend-bot"))
//		cerr << "got you" << endl;
	// first page in the buffer
	if (not_a_full_page) {
		tag_start = strstr(pos, "<page>");
		int len = strlen(buffer_page);
		int ext_len = 0;
		if (tag_start) {
			pos = tag_start;
			ext_len = tag_start - buffer;
		}
		else if ((tag_end = strstr(pos, "</page>"))) {
			pos = tag_end + 7;
			ext_len += tag_end - buffer + 7;
		}
		else {
			cerr << "someting very wrong happened:" << endl << buffer;
		}

		strncat(buffer_page, buffer, ext_len);
		len += ext_len;
		buffer_page[len] = '\0';

		create_page(buffer_page, buffer_page + len);
	}

	while ((tag_start = strstr(pos, "<page>"))) {
//		tag_start += 6;

		tag_end = strstr(tag_start + 6, "</page>");
		if (tag_end != NULL) {
			not_a_full_page = false;
			tag_end += 7;
			create_page(tag_start, tag_end);
		}
		else { // not a complete page
			long bytes_written = tag_start - buffer;
			long bytes_left = size - bytes_written;
			memory_copy(buffer_page, tag_start, bytes_left);
			buffer_page[bytes_left] = '\0';

			not_a_full_page = true;
//			if (last_piece)
//				cerr << pos << endl;
			break;
		}
		pos = tag_end;
	}

	return pos;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s input /a/path/for/output\n", argv[0]);
		exit(-1);
	}

	if (argc > 2) {

		outpath = string(argv[2]);
	}

	create_dirs();

	assert(TARGET_BLOCK_SIZE > 0);

	long long length_of_file_in_bytes;
	long long bytes_read = 0;
	long long total_read = 0;
	long long bytes_left = 0;

	long long size;
	memset(buffer, 0, sizeof(buffer));

	int bzError;
	BZFILE *bzf;
//	char buf[4096];
	FILE *bz_file_handler;

	bz_file_handler = fopen(argv[1], "r");

	  bzf = BZ2_bzReadOpen(&bzError, bz_file_handler, 0, 0, NULL, 0);
	  if (bzError != BZ_OK) {
	    fprintf(stderr, "E: BZ2_bzReadOpen: %d\n", bzError);
	    return -1;
	  }

	char *buffer_pos = (char *)buffer;
	long size_to_read = TAR_BLOCK_SIZE;
	while (bzError == BZ_OK) {
	    size = BZ2_bzRead(&bzError, bzf, buffer, size_to_read);
	    if (bzError == BZ_OK || bzError == BZ_STREAM_END) {
	    	buffer[size] = '\0';
	    	read_buffer(size);
//	      size_t nwritten = fwrite(buffer, 1, size, stdout);
//	      if (nwritten != (size_t) size) {
//	        fprintf(stderr, "E: short write\n");
//	        return -1;
//	      }

////			bytes_read += size;
//			total_read += size;
//
//			buffer_pos = (char *)read_buffer(false);
//			if (buffer_pos == buffer) {
//				cerr << "Finished reading file or the buffer size is too small!" << endl;
//				break;
//			}
//			bytes_read = buffer_pos - buffer;
//			bytes_left = size - bytes_read;
//			size_to_read = TAR_BLOCK_SIZE - bytes_left;
//			if (bytes_left > 0)
//				memmove(buffer, buffer_pos, bytes_left);
//			buffer_pos = buffer + bytes_left;
//			*buffer_pos = '\0';
	    }
	  }
//	read_buffer(true);
	check_last_page();

	  if (bzError != BZ_STREAM_END) {
	    fprintf(stderr, "E: bzip error after read: %d\n", bzError);
	    return -1;
	  }

	  BZ2_bzReadClose(&bzError, bzf);
	  return 0;

	return 0;
}
