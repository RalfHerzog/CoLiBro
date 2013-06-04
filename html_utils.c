#include "html_utils.h"

void html_list_create( struct HTML_TAG* start, struct HTML_TAG* end, const char* name, struct HTML_LIST** list )
{
  struct HTML_TAG* currentTag;
  struct HTML_LIST* listElement;

  if ( start == NULL )
  {
    *list = NULL;
    return;
  }

  *list = (struct HTML_LIST*)malloc( sizeof( struct HTML_LIST ) );
  memset( *list, 0, sizeof( struct HTML_LIST ) );
  listElement = *list;

  currentTag = start;
  while( currentTag != end && currentTag->next != NULL )
  {
    if ( !strcmpi( currentTag->name, name ) )
    {
      listElement->tag = currentTag;
      listElement->next = (struct HTML_LIST*)malloc( sizeof( struct HTML_LIST ) );

      listElement = listElement->next;
      memset( listElement, 0, sizeof( struct HTML_LIST ) );
    }
    currentTag = currentTag->next;
  }
}
void html_list_free( struct HTML_LIST* list )
{
  if ( list == NULL )
  {
    return;
  }
  html_list_free( list->next );
  free( list );
}
void html_escape_string( char** sPostData )
{
  const char search[] = {':', '/'};
  const char* replace[] = { "%3A", "%2F" };

  char* sPostReturn;

  unsigned int i, j, k, size, sizeNew;

  if ( sPostData == NULL || *sPostData == '\0' )
  {
    return;
  }
  size = strlen( *sPostData );

  sizeNew = 0;
  for ( i = 0 ; i < size ; i++, sizeNew++ )
  {
    for ( j = 0 ; j < sizeof( search ) ; j++ )
    {
      if ( *(*sPostData+i) == search[j] )
      {
        sizeNew += my_strlen( replace[j] ) - 1;
        continue;
      }
    }
  }
  sPostReturn = (char*)malloc( sizeNew + 1 );

  for ( i = 0, k = 0 ; i < size ; i++, k++ )
  {
    for ( j = 0 ; j < sizeof( search ) ; j++ )
    {
      if ( *(*sPostData+i) == search[j] )
      {
        memcpy( sPostReturn+k, replace[j], my_strlen( replace[j] ) );
        k += my_strlen( replace[j] ) - 1;
        break;
      }
    }
    if ( j == sizeof( search ) )
    {
      memcpy( sPostReturn+k, *sPostData+i, 1 );
    }
  }
  free( *sPostData );

  memset( sPostReturn+sizeNew, 0, 1 );
  *sPostData = sPostReturn;
}
void html_replace_special_chars( const char* input, char** output )
{
  const char* special[] = {"&uuml;", "&Uuml;", "&ouml;", "&Ouml;", "&auml;", "&Auml;",
                            "&szlig;", "&amp;", "&euro;", "&lt;", "&gt;", "&quot;",
                            "&copy;"};
  const char* replace[] = {"ü", "Ü", "ö", "Ö", "ä", "Ä", "ß", "&", "€", "<", ">", "\"", "(c)" };

  int i, j, k;
  unsigned int length;
  unsigned char found;
  char* ret;

  length = my_strlen( input );
  ret = (char*)malloc( length+1 );
  memset( ret, 0, length+1 );

  for ( i = 0, j = 0 ; i < length ; i++ )
  {
    if ( *(input+i) < 0x20 || *(input+i) > 0x7E )
    {
      i++;
    }
    else if ( *(input+i) == '&' )
    {
      found = 0;

      // Propably special sequence
      for ( k = 0 ; k < ( sizeof( special ) / sizeof( char* ) ) ; k++ )
      {
        if ( !strncmp( input+i, special[k], my_strlen( special[k] ) ) )
        {
          found = 1;
          strcat( ret, replace[k] );
          j += my_strlen( replace[k] );
          i += my_strlen( special[k] ) - 1;
          break;
        }
      }
      if ( !found )
      {
        *(ret+j++) = *(input+i);
      }
    }
    else
    {
      *(ret+j++) = *(input+i);
    }
  }
  memset( ret+j, 0, 1 );
  *output = ret;
}
