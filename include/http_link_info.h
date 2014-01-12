#ifndef HTTP_LINK_INFO_H_INCLUDED
#define HTTP_LINK_INFO_H_INCLUDED

#include "http_wrapper.h"

struct HTTP;

struct HTTP_LINK_INFO{
	char* protocol;
	char* host;
	char* port;
	char* subdir;
	char* file;
	char* parameters;
	int size;
};

void http_link_get_info ( struct HTTP* http, struct HTTP_LINK_INFO* http_info, const char* sLink );
void http_link_info_free( struct HTTP_LINK_INFO* http_link_info );

#endif // HTTP_LINK_INFO_H_INCLUDED
