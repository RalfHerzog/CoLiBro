#ifndef HTML_TAGS_H_INCLUDED
#define HTML_TAGS_H_INCLUDED

struct HTML;

#include <stdarg.h>

#include "html.h"
#include "html_utils.h"

#define HTML_TAG_FOREACH( start, tag )\
            HTML_TAG_FOREACH_SUB( start, NULL, tag )

#define HTML_TAG_FOREACH_SUB( start, end, tag )\
            if ( start != NULL )\
            for ( tag = start ; tag->next != NULL && tag->next != end ; tag = tag->next )

enum HTML_TAG_STATE{
  HTML_TAG_STATE_OPEN,
  HTML_TAG_STATE_CLOSE,
  HTML_TAG_STATE_AUTOCLOSE,
  HTML_TAG_STATE_UNKNOWN
};

struct HTML_TAG{
  char* name;
  char* parameter;

  char* param_name;
  char* param_value;
  char* param_id;

  unsigned int length;

  unsigned char state;
  unsigned int  level;

  unsigned int  textStart;
  unsigned int  textLength;

  struct HTML_TAG* start;
  struct HTML_TAG* end;
  struct HTML_TAG* prev;
  struct HTML_TAG* next;
};

void  html_tag_init( struct HTML* html, const char* content, struct HTML_TAG** tag );
void  html_tag_override_input( struct HTML* html, struct HTML_TAG* tagForm, unsigned char count, ... );
void  html_tag_generate_post( struct HTML_TAG* tagForm, char** postData );
void  html_tag_get_parameter_field( const char* parameter, const char* field, char** data, unsigned int* size );
char* html_tag_get_content( struct HTML* html, struct HTML_TAG* tag );

/** External function export */
extern void html_tag_alloc( struct HTML_TAG** tag, struct HTML_TAG* prev );
extern void html_tag_get_data( struct HTML* html, const char* content, struct HTML_TAG** tag, unsigned int* length );

#endif // HTML_TAGS_H_INCLUDED
