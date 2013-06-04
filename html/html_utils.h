#ifndef HTML_UTILS_H_INCLUDED
#define HTML_UTILS_H_INCLUDED

#include "html_tags.h"

#define HTML_LIST_FOREACH( start, listIt )\
          if ( start != NULL )\
            for ( listIt = start ; listIt->tag != NULL ; listIt = listIt->next )

struct HTML_LIST{
  struct HTML_TAG* tag;
  struct HTML_LIST* next;
};

void html_list_create( struct HTML_TAG* start, struct HTML_TAG* end, const char* name, struct HTML_LIST** list );
void html_list_free( struct HTML_LIST* list );
void html_escape_string( char** sPostData );
void html_replace_special_chars( const char* input, char** output );

#endif // HTML_UTILS_H_INCLUDED
