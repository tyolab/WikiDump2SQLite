/******************************************************************************
 * This file is part of the XLINK - The Link Discovery Toolkit.
 * (c) Copyright 2013 Queensland University of Technology
 *
 * This file may be distributed and/or modified under the terms of the
 * BSD 3-cause LICENSE appearing in the file BSD.txt included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *******************************************************************************
 *
 * @author				Eric Tang
 *
 *******************************************************************************/
 /*
 * sys_file.h
 *
 *  Created on: Aug 11, 2009
 *
 */

#ifndef SYS_FILE_H_
#define SYS_FILE_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <glob.h>

class sys_file
{
private:
	long			list_count_;

protected:
	const char 		*name_;  // could be pattern
	struct stat 	stat_;
	bool			is_diretory_;
	glob_t 			*data_;
	long long	length_;
	void			*content_;

public:
	static const char*	SEPARATOR;
	static const char*	DEFAULT_PATTERN;

private:
	void free();
	void init();
	int glob(const char *name, int flag);

public:
	sys_file(const char *name);
	sys_file();
	virtual ~sys_file();

	//virtual char *first();
	virtual const char *next() ;

	void read_entire_file();

	static char *read_entire_file(const char *filename, long long *len = 0);
	static char **buffer_to_list(char *buffer, long long *lines);

	static bool exist(const char *name);
	static size_t size(const char *name);
	static bool isdir(const char *name);
	static int stat(struct stat *st, const char *name);
	static int create_directory(const char *name);

	static int write(const char *content, const char *filename);

	char **list(char *dir, char **files_list, bool recursive = false);
	virtual void list(const char *name);

	void pattern(const char *wildcard) { name_ = wildcard; }

	long long length() {return length_; }

	void *content() { return content_; }

	bool exists();
};

#endif /* SYS_FILE_H_ */
