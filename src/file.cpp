/*
 * file.cpp
 *
 *  Created on: Jan 19, 2015
 *      Author: Eric Tang
 */

#include "file.h"
#include "string_utils.h"

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include <stdexcept>
#include <sstream>

#define _FILE_OFFSET_BITS 64

using namespace tyo;

/*
 *
 * most code is adopted from ATIRE ANT_file implementation (BSD licensed), which is written by Andrew Trotman @ Otago University, New Zealand
 *
 */

file::file() {
	_position = 0;
	_count = 0;
	_bufferUsed = 0;
	_bufferReadIn = 0;

	_file = NULL;
	_buffer = NULL;
	_current = NULL;

	_length = 0;

	setBufferSize(1024 * 1024);
	_filename = new char[1024 * 1024];
	*_filename = '\0';
}

file::~file() {
	delete [] _filename;
	delete [] _buffer;

	if (_file != NULL)
		close();
}

/**
 * when do the read/write operation, we are expecting the input/output data stream in char mode not a custom class object
 */
long file::write(const void* data, long long size, long writeAll) {
	const void *from;
	long long block_size;

	/*
		Keep track of the total number of bytes we've been asked to write to the file
	*/
	_count += size;

	/*
		Update the file pointer
	*/
	_position += size;

	if (_bufferUsed + size < _bufferSize)
		{
		/*
			The data fits in the internal buffers
		*/
		char *dest = _buffer + _bufferUsed;
		if (writeAll)
			memset(dest, *(int *)data, size);
		else
			memcpy(dest, data, (size_t)size);
		_bufferUsed += size;
		}
	else
		{
		/*
			The data does not fit in the internal buffers so it is
			necessary to flush the buffers and then do the write
		*/
		from = data;
		do
			{
			flush();
			block_size = size <= _bufferSize ? size : _bufferSize;
			if (writeAll)
				memset(_buffer, *(int *)from, (size_t)block_size);
			else
				memcpy(_buffer, from, (size_t)block_size);
			_bufferUsed += block_size;
			from = block_size + (char *)from;
			size -= block_size;
			}
		while (size > 0);
		}

	return size;
}

/*
 * don't need these
 */
/*
void file::writeInt(int v) {
	checkIfFlushNeeded(sizeof(int));
	memory_copy(_buffer + _bufferUsed, &v, sizeof(int));
}

void file::writeBool(bool v) {
	checkIfFlushNeeded(sizeof(bool));
	memory_copy(_buffer + _bufferUsed, &v, sizeof(bool));
}

void file::writeLong(long v) {
	checkIfFlushNeeded(sizeof(long));
	memory_copy(_buffer + _bufferUsed, &v, sizeof(long));
}

void file::writeLongLong(long long v) {
	checkIfFlushNeeded(sizeof(long long));
	memory_copy(_buffer + _bufferUsed, &v, sizeof(long long));
}

void file::writeUInt16T(uint16_t v) {
	checkIfFlushNeeded(sizeof(uint16_t));
	memory_copy(_buffer + _bufferUsed, &v, sizeof(uint16_t));
}

void file::writeUInt64T(uint64_t v) {
	checkIfFlushNeeded(sizeof(uint64_t));
	memory_copy(_buffer + _bufferUsed, &v, sizeof(uint64_t));
}

void file::writeUInt32T(uint32_t v) {
	checkIfFlushNeeded(sizeof(uint32_t));
	memory_copy(_buffer + _bufferUsed, &v, sizeof(uint32_t));
}

void file::writeUInt8T(uint8_t v) {
	checkIfFlushNeeded(sizeof(uint8_t));
	memory_copy(_buffer + _bufferUsed, &v, sizeof(uint8_t));
}

void file::writeString(const char* str) {
	write(str, strlen(str));
}


void file::checkIfFlushNeeded(long long size) {
	if ((size + _bufferUsed) > _bufferSize)
		flush();
}
*/

void file::flush() {
	if (_bufferUsed > 0) {
		fwrite(_buffer, _bufferUsed, 1, _file);
		_bufferUsed = 0;
	}
}

long file::open(const char *filename, char* mode) {
	strcpy(_filename, filename);

	_file = fopen(filename, mode);

	long ret = true;

	if (!_file) {
		std::stringstream msg;
		msg << "Cannot open file: " << filename << "\n";
		fputs(msg.str().c_str(), stderr);
		ret = false;
	}
	else
		if (strchr(mode, 'r')) {
			jumpToEnd();
			_length = ftello(_file);
			jumpToBegin();

			// and we need bigger buffer size
		}
	return ret;
}

long file::writeInt(int v) {
	return write(&v, sizeof(int));
}

long file::writeBool(bool v) {
	return write(&v, sizeof(bool));
}

long file::writeLong(long v) {
	return write(&v, sizeof(long));
}

long file::writeLongLong(long long v) {
	return write(&v, sizeof(long long));
}

long file::writeUInt16T(uint16_t v) {
	return write(&v, sizeof(uint16_t));
}

long file::writeUInt64T(uint64_t v) {
	return write(&v, sizeof(uint64_t));
}

long file::writeUInt32T(uint32_t v) {
	return write(&v, sizeof(uint32_t));
}

long file::writeUInt8T(uint8_t v) {
	return write(&v, sizeof(uint8_t));
}

long file::writeString(const char* str) {
	return write(str, strlen(str));
}

void file::jumpFromBegin(long where) {
	fseek(_file, where, SEEK_SET);   // seek to the beginning of the file
	_position = where;
}

void file::jumpFromCurrent(long where) {
	fseeko(_file, where, SEEK_CUR);   // seek to the beginning of the file
	_position = ftello(_file);
}

void file::jumpToBegin() {
	jumpFromBegin(0);
}

void file::jumpToEnd() {
	fseeko(_file, 0, SEEK_END);
	_position = _length - 1;
}

long long file::getPosition() {
	long long tempPos = ftello(_file);
	assert(tempPos == _position);
	return tempPos;
}

long long file::getReadInPosition() {
	return _position - _bufferReadIn;
}

/*
 * always allocate one more byte for indicating the end of it
 */
void file::setBufferSize(long long size) {
	if (_buffer)
		delete [] _buffer;
	_bufferSize = size;
	_buffer = new char[_bufferSize + 1];
	_current = _buffer;
}

long file::read(long long size) {
	_bufferUsed = 0;
	_current = _buffer;

	if ((_length  - _position) < size)
		_bufferReadIn = _length  - _position;
	else
		_bufferReadIn = size;

	long bytesRead = read(_buffer, _bufferReadIn);

	long ret = 0;

	if (bytesRead > 0) {
		_position += bytesRead;
		ret = 1;
	}

	if (bytesRead < _bufferReadIn)
		_bufferReadIn = bytesRead;

	return ret;
}

long file::read() {
	return read(_bufferSize);
}

int file::readInt() {
	int v;
	readBuffer(v);
	return v;
}

bool file::readBool() {
	bool v;
	readBuffer(v);
	return v;
}

long file::readLong() {
	long v;
	readBuffer(v);
	return v;
}

long long file::readLongLong() {
	long long v;
	readBuffer(v);
	return v;
}

char *file::readString(long long length) {
	char *dest = strndup((const char *)_current, length);
	seekBuffer(length);
	return dest;
}

char *file::readLine() {
	char *tmp = (char *)_current;
	while (isEndOfBuffer() && *_current != '\n')
		++_current;

	if (_current == tmp)
		return NULL;

	*_current++ = '\0';
	return tmp;
}

uint8_t file::readUInt8T() {
	uint8_t v;
	readBuffer(v);
	return v;
}

uint16_t file::readUInt16T() {
	uint8_t v;
	readBuffer(v);
	return v;
}

uint32_t file::readUInt32T() {
	uint8_t v;
	readBuffer(v);
	return v;
}

uint64_t file::readUInt64T() {
	uint64_t v;
	readBuffer(v);
	return v;
}

void file::jumpInBuffer(long long where) {
#ifdef DEBUG
	if (where < _bufferReadIn) {
#endif
		_bufferUsed = where;
		_current = _buffer + where;
#ifdef DEBUG
	}
	else
		throw std::runtime_error("invalid in-buffer jump.");
#endif
}

void file::seekBuffer(long where) {

	_bufferUsed += where;
	_current += where;

	if (_bufferUsed > _bufferReadIn)
		throw std::runtime_error("over used buffer, shouldn't reach here");
}

long long file::getUnreadSize() {
	return _bufferReadIn - _bufferUsed;
}

void file::close() {
	flush();
	fclose(_file);
	_file = NULL;
}

long long file::getBytesRead() {
	return _bufferReadIn;
}

long file::read(void* dest, long long size) {
	/*
		Keep track of the number of bytes we've been asked to write
	*/
	_count += size;

	/*
		And now perform the read
	*/
	static long long chunk_size = 1 * 1024 * 1024 * 1024;  // 1G of data

	long long in_one_go = 0;
	if (chunk_size < size)
		in_one_go =  chunk_size;
	else
		in_one_go = size;

	long long bytes_read = 0;
	long long bytes_left_to_read = size;
//	char *dest = (char *)destination;

//file_critical_section.enter();
	while (bytes_read < size)
		{
		if (bytes_left_to_read < in_one_go)
			in_one_go = bytes_left_to_read;

		bytes_read += fread((char *)dest + bytes_read, 1, in_one_go, _file);
		bytes_left_to_read = size - bytes_read;

//		if (ferror(fp))
//			return 0;
		}
//file_critical_section.leave();
	return bytes_read; // == size; // will return 0 (fail) or 1 (success)
}

bool file::isBufferNull() {
	return (*_current) == '\0';
}

long file::writeAll(int c, long long size) {
	write(&c, size, true);
}

bool file::isDirectory(const char* file) {
	struct stat st;
	if(stat(file,  &st) == 0)
		return S_ISDIR(st.st_mode);
	return false;
}

bool file::isEndOfBuffer() {
	return (_current - _buffer) < _bufferReadIn;
}

bool file::exists(const char* file) {
	struct stat st;
	return stat(file, &st) == 0;
}
