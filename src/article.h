/*
 * article.h
 *
 *  Created on: 27/06/2014
 *      Author: monfee
 */

#ifndef ARTICLE_H_
#define ARTICLE_H_

#include <string>

#include <stdio.h>
#include <stdlib.h>

class article {
public:
	static const int MODE_PLAIN_TEXT = 0;
	static const int MODE_COMPRESSED_GZIP = 1;

private:
	int id;

//	int type; // 0 for abstract, 1 for full article
	int article_id;

	std::string title;

	char *content; // article blob, may in gzip compressed mode
	int  article_len;

	int mode;

	char *abs;
	int abs_len;

	std::string category;

	int category_ids;

	int redirect;  // 0 false, 1 true

	int in_fix_mode;

	int result; // 1 good, otherwise not

public:
	article();

	virtual ~article();

	article(int mode);

	std::string abstract_to_string();

	std::string toXml();

	std::string bytes_to_string(const char *bytes);

	std::string to_string();

	int get_id();

	void set_id(int id);

	const char *get_article();

	void set_article(char *article);

	int get_mode();

	void set_mode(int mode);

	std::string& get_title();

	void set_title(const char *title);

	int get_article_id();

	void set_article_id(int article_id);

	char *get_abstract();

	void set_abstract(char *abs);

	static article *parse_abstract(const char *doc, int mode);

	int get_abs_len() const;

	void set_abs_len(int abslen);

	int get_article_len() const;

	void set_article_len(int contentlen);

	const std::string& get_category() const;

	void set_category(const std::string& category);

	int get_category_ids() const;

	void set_category_ids(int category_ids);

	int is_redirect() const;

	void set_redirect(int redirect);

	void process_redirect_text(const char* text);

	int get_in_fix_mode() const {
		return in_fix_mode;
	}

	void set_in_fix_mode(int infixmode) {
		in_fix_mode = infixmode;
	}

	int get_result() const;
	void set_result(int result);
};

#endif /* ARTICLE_H_ */
