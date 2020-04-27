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
 * sys_files.cpp
 *
 *  Created on: Sep 30, 2009
 *
 */

#include "sys_files.h"

sys_files::sys_files(const char *name) : sys_file(name)
{

}

sys_files::sys_files() : sys_file()
{

}

sys_files::~sys_files()
{
}

void sys_files::list(const char *name)
{
	sys_file::list(name);

	int count = 0;
	while (count < data_->gl_pathc)
		list_.push_back(data_->gl_pathv[count++]);
//	if (count > 0)
//		cur_file_ = list_.end()--;
}

const char *sys_files::first()
{
	if (list_.size() > 0) {
		std::string laststr = list_.back();
		stat(&stat_, laststr.c_str());
		if (S_ISDIR(stat_.st_mode)) {
			list_.pop_back();
			list(laststr.c_str());
			return first();
		}
		return last();
	}
	return NULL;
}

const char *sys_files::next()
{
	list_.pop_back();
	if (list_.size() > 0) {

		std::string laststr = list_.back();
		stat(&stat_, laststr.c_str());
		if (S_ISDIR(stat_.st_mode)) {
			list_.pop_back();
			list(laststr.c_str());
			return first();
		}
		return last();
	}
	return NULL;
}

const char *sys_files::last()
{
	return list_.back().c_str();
}

void sys_files::list() {
	list(name_);
}

