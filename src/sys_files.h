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
 * sys_files.h
 *
 *  Created on: Sep 30, 2009
 *
 */

#ifndef SYS_FILES_H_
#define SYS_FILES_H_

#include "sys_file.h"
#include <vector>
#include <string>

/*
 * This is a class for reading system files recursively
 */
class sys_files: public sys_file
{
private:
	std::vector<std::string>			list_;
	//std::vector<std::string>::iterator	cur_file_;

public:
	sys_files(const char *name);
	sys_files();
	virtual ~sys_files();

	const char *first();
	const char *next() ;
	const char *last() ;
	virtual void list(const char *name);
	virtual void list();
};

#endif /* SYS_FILES_H_ */
