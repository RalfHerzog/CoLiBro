#ifndef HTML_H_INCLUDED
#define HTML_H_INCLUDED

#include <stdlib.h>
#include <string.h>

struct HTML_TAG;

#include "html_tags.h"
#include "http_utils.h"

enum HTML_OPTIONS {
  HTML_OPTION_EXTENDED_PARAMS = 0,
  HTML_OPTION_CHARSET = 1
};

struct HTML{
  unsigned long options;
  char* content;
  struct HTML_TAG* tags;
};

void html_init( struct HTML* html );
void html_parse( struct HTML* html, const char* content );
void html_set_opt( struct HTML* html, enum HTML_OPTIONS option, unsigned char value );
unsigned char html_get_opt( struct HTML* html, enum HTML_OPTIONS option );
void html_free( struct HTML* html );

#endif // HTML_H_INCLUDED
