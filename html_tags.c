#include "html_tags.h"

void html_tag_alloc( struct HTML_TAG** tag, struct HTML_TAG* prev )
{
  *tag = (struct HTML_TAG*)malloc( sizeof( struct HTML_TAG ) );
  memset( *tag, 0, sizeof( struct HTML_TAG ) );

  (*tag)->prev = prev;
  (*tag)->state = HTML_TAG_STATE_UNKNOWN;
  return;
}

void html_tag_init_name( const char* start, const char* end, char** data, unsigned int* size )
{
  char* tag_start = (char*)start;
  char* tag_name_end;

  *data = NULL;
  *size = 0;

  if ( tag_start == NULL )
  {
    return;
  }
  if ( *tag_start == '<' )
  {
    tag_start++;
  }
  tag_name_end = strchr( tag_start, ' ' );

  if ( tag_name_end == NULL || tag_name_end > end )
  {
    *size = end - tag_start;
  }
  else
  {
    *size = tag_name_end - tag_start;
  }

  *data = (char*)malloc( *size+1  );

  memcpy( *data, tag_start, *size );
  memset( *data+*size, '\0', 1 );
  return;
}
void html_tag_init_parameter( const char* start, const char* end, char** data, unsigned int* size )
{
  char* param_start = (char*)start;

  *data = NULL;
  *size = 0;

  if ( param_start == NULL || *param_start == '>' )
  {
    return;
  }
  if ( *param_start == ' ' )
  {
    param_start++;
  }

  *size = end - param_start;
  *data = (char*)malloc( *size+1  );

  memcpy( *data, param_start, *size );
  memset( *data+*size, '\0', 1 );
  return;
}
void html_tag_get_parameter_field( const char* parameter, const char* field, char** data, unsigned int* size )
{
  char* field2;
  char* ptr;
  char* ptr_end;
  int length;

  *data = NULL;
  if ( size != NULL )
  {
    *size = 0;
  }

  if ( parameter == NULL )
  {
    return;
  }
  field2 = (char*)field;

  length = strlen( field2 );
  if ( *( field2+length-1 ) != '=' )
  {
    field2 = (char*)malloc( length+2 );
    memcpy( field2, field, length );
    memcpy( field2+length, "=\0", 2 );
    length += 1;
  }

  ptr = stristr( parameter, field2 );

  if ( ptr )
  {
    // Field found
    ptr += length;
    if ( *ptr == '\"' )
    {
      ptr += 1;
      ptr_end = strchr( ptr, '\"' );
    }
    else
    {
      ptr_end = strchr( ptr, ' ' );
      if ( ptr_end == NULL )
      {
        ptr_end = strchr( ptr, '>' );
        if( ptr_end == NULL )
        {
          ptr_end = ptr + strlen( ptr );
        }
      }
    }

    length = ptr_end - ptr;
    *data = (char*)malloc( length + 1 );
    memcpy( *data, ptr, length );
    memset( *data+length, 0, 1 );

    if( size != NULL )
    {
      *size = length;
    }
  }
  if( field2 != field )
  {
    free( field2 );
  }
}
void html_tag_init( struct HTML* html, const char* content, struct HTML_TAG** tag )
{
  char* tag_begin;
  char* tag_end;
  unsigned int size;

  (*tag)->length = 0;

  tag_begin = (char*)content;
  if ( tag_begin == NULL )
  {
    return;
  }
  tag_end = strchr( tag_begin, '>' );
  if ( tag_end == NULL )
  {
    return;
  }

  (*tag)->length = tag_end - tag_begin - 1;

  html_tag_init_name( tag_begin+1, tag_end, &(*tag)->name, &size );

  if ( (*tag)->name == NULL )
  {
	  return;
  }
  tag_begin += size + 1;

  html_tag_init_parameter( tag_begin, tag_end, &(*tag)->parameter, &size );

  if ( !strcmpi( (*tag)->name, "input" ) )
  {
    html_tag_get_parameter_field( (*tag)->parameter, "name", &(*tag)->param_name, NULL );
    html_tag_get_parameter_field( (*tag)->parameter, "value", &(*tag)->param_value, NULL );
    html_tag_get_parameter_field( (*tag)->parameter, "id", &(*tag)->param_id, NULL );
  }
  return;
}
void html_tag_generate_post( struct HTML_TAG* tagForm, char** postData )
{
  struct HTML_LIST* list_element;
  struct HTML_LIST* list;
  unsigned int size;

  if ( tagForm == NULL )
  {
    *postData = NULL;
    return;
  }
  html_list_create( tagForm, tagForm->end, "input", &list );

  size = 0;
  HTML_LIST_FOREACH( list, list_element )
  {
    size += my_strlen( list_element->tag->param_name );
    size += 1; /** '=' */
    size += my_strlen( list_element->tag->param_value );
    size += 1; /** '&' */
  }

  *postData = (char*)malloc( size + 1 );
  memset( *postData, 0, size+1 );

  HTML_LIST_FOREACH( list, list_element )
  {
    if ( list_element->tag->param_name != NULL )
    {
      strcat( *postData, list_element->tag->param_name );
    }
    else
    {
      continue;
    }
    strcat( *postData, "=" );
    if ( list_element->tag->param_value != NULL )
    {
      strcat( *postData, list_element->tag->param_value );
    }
    if ( list_element->next->tag != NULL )
    {
      strcat( *postData, "&" );
    }
  }
  html_list_free( list );

  html_escape_string( postData );
}
void html_tag_override_input( struct HTML* html, struct HTML_TAG* tagForm, unsigned char count, ... )
{
  struct HTML_LIST* list;
  struct HTML_LIST* listElement;
  unsigned int iPos;
  char* line;
  va_list va;

  if ( tagForm == NULL )
  {
    return;
  }
  if ( html_get_opt( html, HTML_OPTION_EXTENDED_PARAMS ) == 0 )
  {
    //printf( "\nhtml_tag_override_input: Not functional because of extended parameters missing. Set them via html_set_opt()\n" );
  }

  html_list_create( tagForm, NULL, "input", &list );

  va_start( va, count );
  for ( ; count > 0 ; count-- )
  {
    line = va_arg( va, char* );
    for ( iPos = 0 ; *(line+iPos) != '=' ; iPos++ );

    HTML_TAG_FOREACH( list, listElement )
    {
      if ( iPos == my_strlen( listElement->tag->param_name ) &&
           !strncmpi( listElement->tag->param_name, line, iPos ) )
      {
        /** Input tag found */
        free( listElement->tag->param_value );
        listElement->tag->param_value = new_string( line+iPos+1 );
        break;
      }
    }
  }
  va_end( va );

  html_list_free( list );
}
