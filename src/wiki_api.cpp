/*
 * wiki_api.cpp
 *
 *  Created on: 30/06/2014
 *      Author: monfee
 */

#include "wiki_api.h"
#include "urlcode.h"

#include <stdlib.h>
#include <stdio.h>

#include <string>

using namespace std;

int wiki_api::count = 0;

/*
 * original
const std::string wiki_api::URL_TEMPLATE = "/api.php?action=mobileview&redirect=yes&sections=all"
		"&prop=id|text|sections|normalizedtitle|lastmodified|lastmodifiedby|revision|languagecount|hasvariants|displaytitle"
		"&format=json&sectionprop=toclevel|level|line&noheadings=yes"
		"&page=";
*/
const std::string wiki_api::URL_TEMPLATE = "api.php?action=mobileview&redirect=yes&sections=all"
		"&prop=id|text|sections|normalizedtitle|revision|languagecount|hasvariants|displaytitle"
		"&format=json&sectionprop=toclevel|level|line&noheadings=yes"
		"&page=";

std::string wiki_api::host = "http://localhost/";
std::string wiki_api::root = "w";
std::string wiki_api::url_template = host + root + URL_TEMPLATE;

wiki_api::wiki_api() {


}

wiki_api::~wiki_api() {

}

std::string wiki_api::get_wiki_page(const char* title) {
	char *encoded = url_encode(title);
	std::string url = url_template + url_encode(title);

	string page = wget.retrieve(url.c_str());

	free(encoded);

//	if (wget.get_response_code() != 200)
//		return ""; // don't do this, maybe page = "";

	return page;
}

void wiki_api::initialize() {
	url_template = host + root + URL_TEMPLATE;
}
