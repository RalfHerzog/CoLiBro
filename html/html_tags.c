#include "html_tags.h"

void html_tag_alloc( struct HTML_TAG** tag, struct HTML_TAG* prev )
{
  *tag = (struct HTML_TAG*)malloc( sizeof( struct HTML_TAG ) );
  memset( *tag, 0, sizeof( struct HTML_TAG ) );

  (*tag)->prev = prev;
  (*tag)->state = HTML_TAG_STATE_UNKNOWN;
  return;
}
void html_tag_get_parameter( const char* content, unsigned int length, const char* field, char** data, unsigned int* size )
{
  char* field_begin;
  char* field_value_end;
  int internSize;

  if ( size != NULL )
  {
    *size = 0;
  }
  *data = NULL;

  if ( length == 0 )
  {
    return;
  }

  if ( field == NULL )
  {
    *data = (char*)malloc( length );
    memcpy( *data, content, length-1 );
    memset( *data+length-1, 0, 1 );

    if ( size != NULL )
    {
      *size = length;
    }
    return;
  }

  field_begin = strstr( (char*)content, field );

  if ( field_begin == NULL )
  {
    return;
  }
  if ( field_begin - content > length )
  {
    return;
  }
  field_begin += my_strlen( field );
  if ( *field_begin == '=' || *( content+my_strlen( content )-1 ) == '=' )
  {
    field_begin += 1;
  }

  if ( *field_begin == '\"' )
  {
    field_begin += 1;
    field_value_end = strstr( field_begin, "\"" );

    internSize = field_value_end - field_begin;
    if ( size != NULL )
    {
      *size = internSize;
    }

    *data = (char*)malloc( field_value_end - field_begin + 1 );
    memcpy( *data, field_begin, internSize );
    memset( *data+internSize, 0, 1 );
  }
  else
  {
    field_value_end = strstr( field_begin, " " );
    if ( field_value_end - field_begin < length )
    {
      internSize = field_value_end - field_begin;
      if ( internSize < 0 )
      {
        return;
      }

      if ( size != NULL )
      {
        *size = internSize;
      }

      *data = (char*)malloc( internSize+1 );
      memcpy( *data, field_begin, internSize );
      memset( *data+internSize, 0, 1 );
    }
  }
}
void html_tag_get_parameter_field( struct HTML_TAG* tag, const char* field, char** data )
{

}
void html_tag_get_name( const char* content, char** data, unsigned int* size )
{
  int i;

  i = 0;
  for ( i = 0 ; *(content+i) != ' ' &&
                *(content+i) != '>' &&
                //*(content+i) != '/' &&
                *(content+i) != '\0' ; i++ );
  if ( *(content+i) != '\0' && i > 0 )
  {
    *data = (char*)malloc( i+1 );
    memcpy( *data, content, i );
    memset( *data+i, 0, 1 );

    *size = i;
    return;
  }
  *data = NULL;
  *size = 0;
}
void html_tag_get_info( const char* content, unsigned int length, enum HTML_TAG_INFO info, char** data, unsigned int* size )
{
  switch( info )
  {
    case HTML_TAG_INFO_NAME:
      html_tag_get_name( content, data, size );
      break;
    case HTML_TAG_INFO_PARAM_NAME:
      html_tag_get_parameter( content, length, "name=", data, size );
      break;
    case HTML_TAG_INFO_PARAMETER:
      html_tag_get_parameter( content, length, NULL, data, size );
      break;
    case HTML_TAG_INFO_PARAM_VALUE:
      html_tag_get_parameter( content, length, "value=", data, size );
      break;
    case HTML_TAG_INFO_PARAM_ID:
      html_tag_get_parameter( content, length, "id=", data, size );
      break;
    case HTML_TAG_INFO_PARAM_SRC:
      html_tag_get_parameter( content, length, "src=", data, size );
      break;
    case HTML_TAG_INFO_PARAM_REALM:
      html_tag_get_parameter( content, length, "realm=", data, size );
      break;
    case HTML_TAG_INFO_PARAM_HREF:
      html_tag_get_parameter( content, length, "href=", data, size );
      break;
    case HTML_TAG_INFO_PARAM_ALT:
      html_tag_get_parameter( content, length, "alt=", data, size );
      break;
    case HTML_TAG_INFO_PARAM_ACTION:
      html_tag_get_parameter( content, length, "action=", data, size );
      break;
    case HTML_TAG_INFO_PARAM_STYLE:
      html_tag_get_parameter( content, length, "style=", data, size );
      break;
    default:
      *data = NULL;
      *size = 0;
      break;
  }
}
void html_tag_get_data( struct HTML* html, const char* content, struct HTML_TAG** tag, unsigned int* length )
{
  char* tag_begin;
  char* tag_end;
  char* data;
  unsigned int size;
  unsigned char fullParam;

  *length = 0;

  tag_begin = strstr( (char*)content, "<" );
  if ( tag_begin == NULL )
  {
    return;
  }
  tag_end = strstr( tag_begin, ">" );
  if ( tag_end == NULL )
  {
    return;
  }
  fullParam = html_get_opt( html, HTML_OPTION_EXTENDED_PARAMS );

  *length = tag_end - content + 1;

  html_tag_get_info( tag_begin+1, *length, HTML_TAG_INFO_NAME, &data, &size );
  (*tag)->name = data;

  if ( (*tag)->name == NULL )
  {
	  return;
  }

  html_tag_get_info( tag_begin+size+2, tag_end-tag_begin-size-1, HTML_TAG_INFO_PARAMETER, &data, &size );
  if ( *length > size && size > 0 )
  {
    (*tag)->parameter = data;
  }

  if ( fullParam && (*tag)->parameter != NULL && !strcmpi( (*tag)->name, "input" ) )
  {
    if ( stristr( (*tag)->parameter, "name=" ) )
    {
      html_tag_get_info( tag_begin, *length, HTML_TAG_INFO_PARAM_NAME, &data, &size );
      if ( *length > size && size > 0 )
      {
        (*tag)->param_name = data;
      }
    }

    if ( stristr( (*tag)->parameter, "value=" ) )
    {
      html_tag_get_info( tag_begin, *length, HTML_TAG_INFO_PARAM_VALUE, &data, &size );
      if ( *length > size && size > 0 )
      {
        (*tag)->param_value = data;
      }
    }

    if ( stristr( (*tag)->parameter, "id=" ) )
    {
      html_tag_get_info( tag_begin, *length, HTML_TAG_INFO_PARAM_ID, &data, &size );
      if ( *length > size && size > 0 )
      {
        (*tag)->param_id = data;
      }
    }
  }
  return;
}
void html_tag_get_tag( struct HTML* html, enum HTML_TAG_INFO info, const char* fieldValue, struct HTML_TAG** tag_param )
{
  struct HTML_TAG* tag;
  char* data;

  tag = html->tags;

  if ( info != HTML_TAG_INFO_NONE )
  {
    while( tag->next != NULL )
    {
      data = NULL;
      switch( info )
      {
        case HTML_TAG_INFO_NAME:
          data = tag->name;
          break;
        case HTML_TAG_INFO_PARAM_ID:
          data = tag->param_id;
          break;
        case HTML_TAG_INFO_PARAM_NAME:
          data = tag->param_name;
          break;
        case HTML_TAG_INFO_PARAM_VALUE:
          data = tag->param_value;
          break;
        default:
          break;
      }
      if ( data != NULL )
      {
        if ( !strcmpi( data, fieldValue ) )
        {
          break;
        }
      }
      tag = tag->next;
    }
  }
  else
  {
    while( tag->next != NULL )
    {
      if ( tag->parameter != NULL )
      {
        if ( strstr( tag->parameter, fieldValue ) != NULL )
        {
          break;
        }
      }
      tag = tag->next;
    }
  }
  if ( tag->next == NULL )
  {
    *tag_param = NULL;
    return;
  }
  *tag_param = tag;
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
    printf( "\nhtml_tag_override_input: Not functional because of extended parameters missing. Set them via html_set_opt()\n" );
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
