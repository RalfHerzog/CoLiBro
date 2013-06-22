#include <stdio.h>
#include <stdlib.h>

#include "src/colibro.h"

int main()
{
  struct HTTP http;
  struct HTML html;
  struct HTML_LIST* list;
  struct HTML_LIST* list_it;

  char* content;
  int size;

  http_init( &http, HTTP_INIT );

  http_set_opt( &http, HTTP_OPTION_VERBOSE, HTTP_BOOL_TRUE );
  http_set_opt( &http, HTTP_OPTION_DOWNLOAD_FILES, 1 );
  http_set_opt( &http, HTTP_OPTION_USER_AGENT_MOBILE, 1 );

  http_get_page( &http, "https://www.google.com/", &content, &size );

  http_free( &http );

  html_init( &html );
  html_parse( &html, content );

  html_list_create( html.tags, html.tags->end, "a", &list );
  HTML_LIST_FOREACH( list, list_it )
  {
    fwrite( content+(list_it->tag->textStart), 1, list_it->tag->textLength, stdout );
    printf("\n");
  }

  html_free( &html );

  //printf("%s\n", content);
  free( content );


  return 0;
}


