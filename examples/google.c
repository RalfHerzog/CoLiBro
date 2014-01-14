#include <stdio.h>
#include <stdlib.h>

#include "colibro.h"

int main()
{
	struct HTTP http;
	struct HTML html;
	struct HTML_LIST* list;
	struct HTML_LIST* list_it;

	char* content, *html_a;
	int size;

	http_init( &http );

	http_set_opt( &http, HTTP_OPTION_VERBOSE, HTTP_BOOL_TRUE );
	// http_set_opt( &http, HTTP_OPTION_DOWNLOAD_FILES, 1 );
	http_set_opt( &http, HTTP_OPTION_USER_AGENT_MOBILE, 1 );
	http_set_opt( &http, HTTP_OPTION_LOG_ENABLED, 1 );
	http_set_opt( &http, HTTP_OPTION_LOG_RESPONSE, 1 );
	http_set_opt( &http, HTTP_OPTION_LOG_OVERWRITE, 1 );

	http_get_page( &http, "https://www.google.com/", &content, &size );
	http_free( &http );

	printf( "Received %i bytes of data\n\n", size );

	html_init( &html );
	html_parse( &html, content );
	if ( html.tags != NULL )
	{
		html_list_create( html.tags, html.tags->end, "a", &list );
		HTML_LIST_FOREACH( list, list_it )
		{
			html_a = html_tag_get_content( &html, list_it->tag );
			if ( html_a != NULL )
			{
				printf("%s\n", html_a);
				fflush( stdout );
				free( html_a );
			}
		}
	}
	html_free( &html );

	free( content );
	return 0;
}


