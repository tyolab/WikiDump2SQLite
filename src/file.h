/*
 * file.h
 *
 *  Created on: Jan 19, 2015
 *      Author: Eric Tang
 *
 *      most code is ad
 */

#ifndef FILE_H_
#define FILE_H_

#include <stdio.h>
#include <stdint.h>

#include "string_utils.h"

namespace tyo {

class file {

protected:
	long long _position;
	char *_buffer;
	char *_current;

private:
	long long _bufferSize, _bufferUsed, _bufferReadIn;
	long long _count;

	char *_filename;
	FILE *_file;
	long long _length;

public:
	file();
	virtual ~file();

	const char *get_filename() { return _filename; }

	long write(const void *data, long long size, long writeAll = 0);
	long writeAll(int c, long long size);

//	long write(const char *data, long long size) { return write((long *)data, size); }
	long writeInt(int v);
	long writeBool(bool v);
	long writeLong(long v);
	long writeLongLong(long long v);
	long writeString(const char *str);
	long writeUInt64T(uint64_t v);
	long writeUInt32T(uint32_t v);
	long writeUInt16T(uint16_t v);
	long writeUInt8T(uint8_t v);

	template<typename T>
	long writeBuffer(T& v);

	void flush();
	void jumpFromBegin(long where);
	void jumpFromCurrent(long where);
	void jumpToBegin();
	void jumpToEnd();

	void jumpInBuffer(long long where);

	void setBufferSize(long long size);

	long long getBytesRead();
	long long getUnreadSize();
	long long getBufferSize() { return _bufferSize; }

	/*
	 * read from file to buffer
	 */
	long read(long long size);
	long read();

	/*
	 * read from buffer
	 */
	int readInt();
	bool readBool();
	long readLong();
	long long readLongLong();
	uint8_t readUInt8T();
	uint16_t readUInt16T();
	uint32_t readUInt32T();
	uint64_t readUInt64T();

	// needs to free the return string pointer
	char *readString(long long length);
	char *readLine();

	void seekBuffer(long where);
	bool isBufferNull();

	template<typename T>
	long readBuffer(T& v);

	template<typename T>
	long readBuffer(T& v, long size);

	bool isEndOfBuffer();

	long open(const char *filename, char *mode);
	void close();

	static bool isDirectory(const char *file);

	long long getPosition();
	long long getReadInPosition();

	static bool exists(const char *file);

private:
	long read(void *dest, long long size);
};

template<typename T>
long file::readBuffer(T& v) {
	long size = memory_copy(v, _current);

	seekBuffer(size);

	return size;
}

template<typename T>
long file::readBuffer(T& v, long size) {
	memory_copy(v, _current, size);

	seekBuffer(size);

	return size;
}

template<typename T>
long file::writeBuffer(T& v) {
	return write(&v, sizeof(T));
}
}

#endif /* FILE_H_ */
