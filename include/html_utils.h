#ifndef HTML_UTILS_H_INCLUDED
#define HTML_UTILS_H_INCLUDED

#include "html_tags.h"

#define HTML_LIST_FOREACH( list, listIt )\
					if ( list != NULL )\
						for ( listIt = list ; listIt->tag != NULL ; listIt = listIt->next )

struct HTML_LIST{
	struct HTML_TAG* tag;
	struct HTML_LIST* next;
};

int html_min( int a, int b );
int html_max( int a, int b );
void html_list_create( struct HTML_TAG* start, struct HTML_TAG* end, const char* name, struct HTML_LIST** list );
void html_list_free( struct HTML_LIST* list );
void html_escape_string( char** sPostData );
void html_replace_special_chars( const char* input, char** output );
void html_sanitize_post_data( char** content );

#endif // HTML_UTILS_H_INCLUDED
