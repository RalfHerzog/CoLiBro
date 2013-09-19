#include "http_post.h"

int http_post_encode(char **dest, const unsigned char *src, int length)
{
  char *d;
  int i;

  *dest = (char*)malloc( 3 * length + 1 );

  d = *dest;
  for( i = 0 ; i < length ; i++ )
  {
    if ( isCharAlphanumeric( *(src+i) ) || *(src+i) == '-' || *(src+i) == '_' || *(src+i) == '.' || *(src+i) == '~' )
    {
      *(d++) = *(src+i);
    }
    else
    {
      sprintf( d, "%%%.2X", *(src+i) );
      d += 3;
    }
  }
  *d = 0;
  return d-*dest;
}

unsigned char http_post_form_urlencoded_add( struct HTTP* http, const char* key, const unsigned char* value, unsigned int value_length )
{
  struct HTTP_POST_FORM_URLENCODED_DATA_ITEM* http_post_form_urlencoded_data_it;

  if ( http->post_form_urlencoded_data == NULL )
  {
    // Initial
    http->post_form_urlencoded_data = (struct HTTP_POST_FORM_URLENCODED_DATA_ITEM*)malloc( sizeof( struct HTTP_POST_FORM_URLENCODED_DATA_ITEM ) );
    memset( http->post_form_urlencoded_data, 0, sizeof( struct HTTP_POST_FORM_URLENCODED_DATA_ITEM ) );

    http_post_form_urlencoded_data_it = http->post_form_urlencoded_data;
  }
  else
  {
    // Seek to end
    http_post_form_urlencoded_data_it = http->post_form_urlencoded_data;
    while( http_post_form_urlencoded_data_it->next != NULL )
    {
      http_post_form_urlencoded_data_it = http_post_form_urlencoded_data_it->next;
    }
  }
  http_post_form_urlencoded_data_it->key = (unsigned char*)new_string( key );
  http_post_form_urlencoded_data_it->key_length = my_strlen( (char*)http_post_form_urlencoded_data_it->key );

  http_post_form_urlencoded_data_it->value_length = http_post_encode( (char**)&http_post_form_urlencoded_data_it->value, value, value_length );

  http_post_form_urlencoded_data_it->next = (struct HTTP_POST_FORM_URLENCODED_DATA_ITEM*)malloc( sizeof( struct HTTP_POST_FORM_URLENCODED_DATA_ITEM ) );
  memset( http_post_form_urlencoded_data_it->next, 0, sizeof( struct HTTP_POST_FORM_URLENCODED_DATA_ITEM ) );

  return 1;
}

unsigned int http_post_form_urlencoded_get_data( char** content, struct HTTP* http )
{
  struct HTTP_POST_FORM_URLENCODED_DATA_ITEM* http_post_form_urlencoded_data_it;
  unsigned int content_length;

  content_length = 0;

  http_post_form_urlencoded_data_it = http->post_form_urlencoded_data;
  while( http_post_form_urlencoded_data_it->next != NULL )
  {
    content_length +=
        http_post_form_urlencoded_data_it->key_length + 1
      + http_post_form_urlencoded_data_it->value_length + 1;
    http_post_form_urlencoded_data_it = http_post_form_urlencoded_data_it->next;
  }

  *content = (char*)malloc( content_length );
  memset( *content, 0, content_length );

  http_post_form_urlencoded_data_it = http->post_form_urlencoded_data;
  while( http_post_form_urlencoded_data_it->next != NULL )
  {
    strcat( *content, (char*)http_post_form_urlencoded_data_it->key );
    strcat( *content, "=" );
    strcat( *content, (char*)http_post_form_urlencoded_data_it->value );
    strcat( *content, "&" );
    http_post_form_urlencoded_data_it = http_post_form_urlencoded_data_it->next;
  }
  memset( *content+content_length-1, 0, 1 );

  return content_length-1;
}
