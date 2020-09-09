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
 * webpage_retriever.cpp
 *
 *  Created on: Dec 31, 2009
 *
 */

#include "webpage_retriever.h"

const char *webpage_retriever::ERROR_PAGE = "<html><title>Error</title><body>Internal Error while retrieving %s with code %d.<br>Below is the response (if any) from the server. <br>==============================================================<br>%s</body></html>\n";

//namespace QLINK {
	static void *myrealloc(void *ptr, size_t size);

	static void *myrealloc(void *ptr, size_t size)
	{
	  /* There might be a realloc() out there that doesn't like reallocing
		 NULL pointers, so we take care of it here */
	  if(ptr)
		return realloc(ptr, size);
	  else
		return malloc(size);
	}

	static size_t
	WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
	{
	  size_t realsize = size * nmemb;
	  struct MemoryStruct *mem = (struct MemoryStruct *)data;

	  mem->memory = (char *)myrealloc(mem->memory, mem->size + realsize + 1);
	  if (mem->memory) {
		memcpy(&(mem->memory[mem->size]), ptr, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;
	  }
	  return realsize;
	}

	webpage_retriever::webpage_retriever()
	{
		headers = NULL;

		  chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
		  chunk.size = 0;    /* no data at this point */

		  curl_global_init(CURL_GLOBAL_ALL);

		  /* init the curl session */
		  curl_handle = curl_easy_init();

		  /* send all data to this function  */
		  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

		  /* we pass our 'chunk' struct to the callback function */
		  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

		  /* some servers don't like requests that are made without a user-agent
			 field, so we provide one */
		  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "CROSSLINK/XLINK1.0 (https://github.com/crosslink/xlink) Mozilla/5.0 (Linux) AppleWebKit/534.30 (KHTML, like Gecko");
		  curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
	}

	webpage_retriever::~webpage_retriever()
	{
		free_chunk();

		/* cleanup curl stuff */
		curl_easy_cleanup(curl_handle);

		/* we're done with libcurl, so clean it up */
		curl_global_cleanup();
	}

int webpage_retriever::get_response_code() {
	// int response_code = 0;
	// curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
	return response_code;
}

void webpage_retriever::free_chunk()
	{
		  if(chunk.memory) {
			free(chunk.memory);
			chunk.memory = NULL;
		  }
		  chunk.size = 0;
	}

	char *webpage_retriever::retrieve(const char *url, int *this_response_code)
	{
		response_code = 200;
		
		free_chunk();

	  /* specify URL to get */
	  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	  curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 4800000);

	  /* get it! */
	  curl_easy_perform(curl_handle);
	  curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);

	  if (this_response_code)
		  *this_response_code = response_code;
	  /*
	   * Now, our chunk.memory points to a memory block that is chunk.size
	   * bytes big and contains the remote file.
	   *
	   * Do something nice with it!
	   *
	   * You should be aware of the fact that at this point we might have an
	   * allocated data block, and nothing has yet deallocated that data. So when
	   * you're done with it, you should free() it as a nice application.
	   */
	  return chunk.memory;
	}

	char *webpage_retriever::retrieve(const char *url)
	{
		retrieve(url, NULL);

		if (response_code != 200) {
			long size = strlen(url) + 4 /*4 digits for the response code*/ + strlen(ERROR_PAGE) + ((chunk.size > 0) ? chunk.size : 0) - 6 /*%s %d %s*/;
			char *buf = (char *) myrealloc(NULL, size);
			sprintf(buf, ERROR_PAGE, url, response_code,  ((chunk.size > 0) ? chunk.memory : ""));
			free_chunk();
			chunk.memory = buf;
			chunk.size = size;
		}
		  return chunk.memory;
	}

	void webpage_retriever::add_header(const char *header) {
		//headers = curl_slist_append(headers, "Content-Type: text/xml");
		headers = curl_slist_append(headers, header);
		curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
	}

//}
