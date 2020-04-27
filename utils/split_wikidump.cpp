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

#include <string>
#include <iostream>
#include <fstream>
#include <exception>
#include <cmath>
#include <cassert>

#include "string_utils.h"
#include "sys_file.h"

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

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s input #", argv[0]);
		exit(-1);
	}

	int number_of_split = 1;


	ANT_memory file_buffer(1024 * 1024);
	ANT_instream *file_stream = NULL, *decompressor = NULL, *instream_buffer = NULL;

	file_stream = new ANT_instream_file(&file_buffer, argv[1]);
	decompressor = new ANT_instream_bz2(&file_buffer, file_stream);
	instream_buffer = new ANT_instream_buffer(&file_buffer, decompressor);


	long TAR_BLOCK_SIZE = 1024 * 1024;
	long TARGET_BLOCK_SIZE = 1024 * 1024 * 512;

	if (argc > 2) {
		number_of_split = atoi(argv[2]);

		if (number_of_split < 0)
			number_of_split = 1;

		long split_size = 0;
		if (number_of_split > 1) {
			split_size = ceil(sys_file::size(argv[1]) / number_of_split);
			//number_of_split = ceil(file_stream->length() / split_size);
			TARGET_BLOCK_SIZE = split_size * 10;
		}
		//
	}
	assert(TARGET_BLOCK_SIZE > 0);
	ANT_instream *source = instream_buffer;
	//ANT_directory_iterator_tar_file_header header;
	char buffer[TAR_BLOCK_SIZE * 2];
//	char buffer_source[TARGET_BLOCK_SIZE + 1];
//	char buffer_target[TARGET_BLOCK_SIZE + 1];
	char *buffer_source = (char *)malloc(sizeof(char) * (TARGET_BLOCK_SIZE + 1)); //new char[TARGET_BLOCK_SIZE + 1];
	char *buffer_target = (char *)malloc(sizeof(char) * (TARGET_BLOCK_SIZE + 1)); //new char[TARGET_BLOCK_SIZE + 1];
	long long length_of_file_in_bytes;
	long long bytes_read = 0;
	long long total_read = 0;

	long long size;
	memset(buffer, 0, sizeof(buffer));
	char *current;

	const string root_tag_closed("\n</mediawiki>");
	//source->read(buffer, TAR_BLOCK_SIZE);

	/*
	 * Step 1. get the root tag
	 */
	size = source->read((unsigned char *)buffer, TAR_BLOCK_SIZE);
	buffer[size] = '\0';
	char *pos_tag_open = strchr((char *)buffer, '>');
	string root_tag_open(buffer, ++pos_tag_open);

	/*
	 * Step 2. copy read buffer to the compression buffer
	 */
	current = (char *)memory_copy(buffer_source, buffer, size);
	current += size;
	//++current;

	bytes_read += size;
	total_read += size;
	string split_name(argv[1]);
	char *pos = NULL;
	char *pre = NULL;
	 {
		/*
		 * Step 3. continue to read into compression buffer until full
		 */
		while ((size = source->read((unsigned char *)buffer, TAR_BLOCK_SIZE)) > 0) {
			buffer[size] = '\0';
			bytes_read += size;
			total_read += size;

			if ((TARGET_BLOCK_SIZE - bytes_read) <= (TAR_BLOCK_SIZE * 3)) {
				pre = NULL;
				// go to the last </page> tag
				pos = buffer;
				while ((pos = strstr(pos, "</page>"))) {
					pos += 7;
					pre = pos;
				}

				/*
				 * handle the content before </page> and after
				 */
				if (pre) {
					pos = pre;
					long temp_size = pos - buffer;
					long left_size = size - temp_size;
					memory_copy(current, buffer, temp_size);
					current += temp_size;
					memory_copy(current, root_tag_closed.c_str(), root_tag_closed.length());
					current += root_tag_closed.length();
					*current = '\0';

					split_name = increase_file_num(split_name, true);

					// compress
					compress_split(buffer_source, buffer_target, TARGET_BLOCK_SIZE + 1, bytes_read - left_size + root_tag_closed.length(), split_name);

					//now deal with the leftover, move the leftover to the front
					//memmove(buffer, pos, left_size);
					strcpy(buffer_source, root_tag_open.c_str());
					current = buffer_source + root_tag_open.length();
					memory_copy(current, pos, left_size);
					current += left_size;
					buffer[0] = '\0';
					*current = '\0';
					bytes_read = left_size + root_tag_open.length();
				}
				else
					throw "The buffer size is too small to fit an article.";

				continue;
			}
			memory_copy(current, buffer, size);
			current += size;
			//cout << buffer;
		}

		if (bytes_read > 0/**buffer_source != '\0'*/) {
			*current = '\0';
			split_name = increase_file_num(split_name, true);
			compress_split(buffer_source, buffer_target, TARGET_BLOCK_SIZE + 1, bytes_read, split_name);
			//puts(buffer);
		}

	}

//	*(++current) = '\0';

	delete file_stream;
	delete decompressor;
	delete instream_buffer;

	free(buffer_source);
	free(buffer_target);
	return 0;
}
