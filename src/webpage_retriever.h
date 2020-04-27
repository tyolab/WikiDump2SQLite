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
 * webpage_retriever.h
 *
 *  Created on: Dec 31, 2009
 *
 */

#ifndef WEBPAGE_RETRIEVER_H_
#define WEBPAGE_RETRIEVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
//#include <curl/types.h>
#include <curl/easy.h>

#include "pattern_singleton.h"

//namespace QLINK {
	struct MemoryStruct {
	  char *memory;
	  size_t size;
	};

	class webpage_retriever : public pattern_singleton<webpage_retriever>
	{
	private:
		static const char 			*ERROR_PAGE;

		  CURL 								*curl_handle;
		  struct curl_slist 				*headers;
		  struct MemoryStruct 		chunk;
		  int									response_code;

	public:
		webpage_retriever();
		virtual ~webpage_retriever();

		char *retrieve(const char *url, int *response_code);
		char *retrieve(const char *url);

		void add_header(const char *header);

		int get_response_code();

	private:
		void free_chunk();
	};

//}

#endif /* WEBPAGE_RETRIEVER_H_ */
