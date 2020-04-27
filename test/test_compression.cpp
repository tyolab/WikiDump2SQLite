/*
 * test_compression.c
 *
 *  Created on: 27/06/2014
 *      Author: monfee
 */


#include "../include/compression.h"

#include <stdio.h>

/* Example text to print out. */

static const char * message =
"Shall I compare thee to a summer's day?\n"
"Thou art more lovely and more temperate:\n"
"Rough winds do shake the darling buds of May,\n"
"And summer's lease hath all too short a date;\n"
;

int main ()
{
//	puts(message);
//	printf("\n");

	char *buffer = new char[strlen(message)];

	int compressed_size = gzip_compress(message, buffer);

	for (int i = 0; i < compressed_size; ++i)
		putc(buffer[i], stdout);

	delete [] buffer;
	return 0;
}
