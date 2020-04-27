/*
 * article.cpp
 *
 *  Created on: 27/06/2014
 *      Author: monfee
 */

#include "article.h"
#include "xml.h"
#include "wiki.h"
#include "compression.h"

#include <stdlib.h>

#include <sstream>

using namespace std;

article::article() : article::article(MODE_PLAIN_TEXT) {
}

article::~article() {
//	if (abs)
//		delete abs;
//
	if (article_len > 0)
		free(content);
//		delete [] ((char *)content);
}

article::article(int mode) : abs(NULL), content(NULL) {
	this->mode = mode;
	this->article_id = 0;
	this->id = 0;
	this->article_len = 0;
	this->abs_len = 0;
	this->redirect = 0;
	this->category_ids = 0;
	this->result = 1;
	this->in_fix_mode = 0;
}

std::string article::abstract_to_string() {
	return bytes_to_string(abs);
}

std::string article::toXml() {
	std::stringstream buffer;
	buffer << ("<doc>\n");
	buffer << "<docid>" << id << "<docid>\n";
	buffer << "<title>" << title << "</title>\n";
	buffer << "<abstract>" << abstract_to_string() << "</abstract>\n";
	buffer << "</doc>\n";

	return buffer.str();
}

std::string article::bytes_to_string(const char *bytes) {
	std::string temp;
	if (mode == MODE_COMPRESSED_GZIP) {

	}
	else
		temp = std::string(bytes);
	return temp;
}

std::string article::to_string() {
	return bytes_to_string(content);
}

int article::get_id() {
	return id;
}

void article::set_id(int id) {
	this->id = id;
}

//	int getType() {
//		return type;
//	}
//
//	void setType(int type) {
//		this->type = type;
//	}

const char *article::get_article() {
	return content;
}

void article::set_article(char *article) {
	this->content = article;
}

int article::get_mode() {
	return mode;
}

void article::set_mode(int mode) {
	this->mode = mode;
}

std::string& article::get_title() {
	return title;
}

void article::set_title(const char *title) {
	this->title = title;
}

int article::get_article_id() {
	return article_id;
}

void article::set_article_id(int article_id) {
	this->article_id = article_id;
}

char *article::get_abstract() {
	return abs;
}

void article::set_abstract(char *abs) {
	this->abs = abs;
}

int article::get_abs_len() const {
	return abs_len;
}

void article::set_abs_len(int abslen) {
	abs_len = abslen;
}

int article::get_article_len() const {
	return article_len;
}

void article::set_article_len(int contentlen) {
	article_len = contentlen;
}

article *article::parse_abstract(const char *doc, int mode) {
	article *art = new article(mode);

	char *text = between(doc, "title");

	char *title = get_wiki_title(text);

	art->set_title(title);

	text = between(doc, "docid");
	if (strlen(text) > 0)
		art->set_article_id(atoi(text));

	text = between(doc, "abstract");
	if (strlen(text) > 0) {
		int len = strlen(text);
		char *blob = new char[len + 1];
		if (mode == MODE_COMPRESSED_GZIP) {
			art->set_article_len(gzip_compress(text, blob));
		}
		else {
			strncpy(blob, text, len);
			blob[len] = '\0';
			art->set_article_len(len);
		}
		art->set_abstract(blob);
		return art;
	}

	delete text;
	return NULL;
}

const std::string& article::get_category() const {
	return category;
}

void article::set_category(const std::string& category) {
	if (category.length() > 0)
		this->category = this->category + "|" + category;
	this->category = category;
}

int article::get_category_ids() const {
	return category_ids;
}

int article::is_redirect() const {
	return redirect;
}

void article::set_redirect(int redirect) {
	this->redirect = redirect;
}

void article::set_category_ids(int category_ids) {
	this->category_ids |= category_ids;
}

int article::get_result() const {
	return result;
}

void article::set_result(int result) {
	this->result = result;
}

void article::process_redirect_text(const char* text) {
	this->content = between(text, "title=\"", "\"", 0);
	this->article_len = strlen(content);
}
