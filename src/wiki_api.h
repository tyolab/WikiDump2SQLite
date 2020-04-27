/*
 * wiki_api.h
 *
 *  Created on: 30/06/2014
 *      Author: monfee
 */

#ifndef WIKI_API_H_
#define WIKI_API_H_

#include <string>

#include "webpage_retriever.h"

class wiki_api {
public:
	static const std::string URL_TEMPLATE;
	static std::string host;
	static std::string root;
	static std::string url_template;

private:
	webpage_retriever wget;

	static int count;

public:
	wiki_api();
	virtual ~wiki_api();

	std::string get_wiki_page(const char *title);
	static void initialize();
};

#endif /* WIKI_API_H_ */
