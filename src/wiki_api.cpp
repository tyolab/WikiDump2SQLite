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

#include <stdexcept>
#include <iostream>

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

std::string wiki_api::host = "http://localhost";
std::string wiki_api::root = "/w";
std::string wiki_api::url_template = host + root + URL_TEMPLATE;

wiki_api::wiki_api() {


}

wiki_api::~wiki_api() {

}

std::string wiki_api::get_wiki_page(const char* title) {
	char *encoded = url_encode(title);
	std::string url = url_template + encoded;
	free(encoded);

	string page = wget.retrieve(url.c_str());
	int response_code = wget.get_response_code();

	if (response_code >= 500) {
		cerr << page << endl;
		throw std::runtime_error("couldn't get the article, web server may need to be restarted");
	}
	else if (response_code != 200) {
		cerr << "Title: " << title << endl;
		cerr << "Url: " << url << endl;
		cerr << "Response code: " << response_code << endl;
		return ""; 
	}

	return page;
}

void wiki_api::initialize() {
	url_template = host + root + URL_TEMPLATE;
}
