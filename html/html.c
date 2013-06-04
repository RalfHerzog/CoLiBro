#include "html.h"

void html_free_tags( struct HTML_TAG* tag );

void html_init( struct HTML* html )
{
  memset( html, 0, sizeof( struct HTML ) );
}
void html_parse( struct HTML* html, const char* content )
{
  struct HTML_TAG* current;
  struct HTML_TAG* current2;
  unsigned int length, level;
  unsigned char locked;
  char* ptr_content, *nextTag;

  ptr_content = (char*)content;
  if ( ptr_content == NULL )
  {
    return;
  }

  locked = 0;

  html_tag_alloc( &html->tags, NULL );
  current = html->tags;
  do{
    html_tag_get_data( html, ptr_content, &current, &length );
    current->textStart = ptr_content-content+length;
    ptr_content += length;

    nextTag = strstr( ptr_content, "<" );
    if ( nextTag != NULL )
    {
      current->textLength = nextTag - content - current->textStart;
    }

    if ( current->name == NULL || length == 0 )
    {
      break;
    }
    if ( !strcmp( current->name, "script" ) )
    {
      locked = 1;
      html_tag_alloc( &current->next, current );
      current = current->next;
    }
    else if ( !strcmp( current->name+1, "script" ) )
    {
      locked = 0;
    }

    if ( locked == 0 )
    {
      if ( length > 0 )
      {
        html_tag_alloc( &current->next, current );
        current = current->next;
      }
    }
    else
    {
      html_free_tags( current );
    }

  }while( length > 0 );

  current = current->prev;
  while( current != NULL )
  {
    if ( *current->name == '/' )
    {
      current2 = current->prev;
      while( current2 != NULL )
      {
        if ( !strcmp( current->name+1, current2->name ) )
        {
          current2->end = current;
          current->start = current2;
          break;
        }
        current2 = current2->prev;
      }

      if ( current2 != NULL )
      {
        current->start->state = HTML_TAG_STATE_OPEN;
        current->state = HTML_TAG_STATE_CLOSE;

      }
    }
    else
    {
      if ( current->end == NULL )
      {
        current->state = HTML_TAG_STATE_AUTOCLOSE;
      }
    }
    current = current->prev;
  }

  level = 0;

  current = html->tags;
  while( current->next != NULL )
  {
    if ( current->state==HTML_TAG_STATE_OPEN )
    {
      current->level = level;
      current->end->level = level;
      level += 1;
    }
    else if ( current->state==HTML_TAG_STATE_CLOSE )
    {
      if ( level > 0 )
      level--;
    }
    else
    {
      current->level = level;
    }
    current = current->next;
  }
}
void html_set_opt( struct HTML* html, enum HTML_OPTIONS option, unsigned char value )
{
  if ( value )
    html->options |= 1<<option;
  else
    html->options &= ~(1<<option);
}
unsigned char html_get_opt( struct HTML* html, enum HTML_OPTIONS option )
{
  return ( html->options & ( 1<<option ) );
}
void html_free_tags( struct HTML_TAG* tag )
{
  if ( tag == NULL )
  {
    return;
  }
  html_free_tags( tag->next );
  free( tag->name );        tag->name         = NULL;
  free( tag->parameter );   tag->parameter    = NULL;
  free( tag->param_name );  tag->param_name   = NULL;
  free( tag->param_value ); tag->param_value  = NULL;
  free( tag->param_id );    tag->param_id     = NULL;
  free( tag->next );        tag->next         = NULL;
}
void html_free( struct HTML* html )
{
  html_free_tags( html->tags );
  free( html->tags );
}
