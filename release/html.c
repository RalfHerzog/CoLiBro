#include "html.h"

void html_free_tags( struct HTML_TAG* tag );
void html_check_charset( struct HTML* html );
void html_apply_charset( struct HTML* html, const char* charset );
void html_apply_charset_utf8( struct HTML* html );

void html_init( struct HTML* html )
{
  memset( html, 0, sizeof( struct HTML ) );
}
void html_parse( struct HTML* html, const char* content )
{
  struct HTML_TAG* current;
  struct HTML_TAG* current2;
  unsigned int level;
  unsigned char locked;
  char* ptr_content;

  html->content = new_string( content );
  ptr_content = (char*)html->content;
  if ( ptr_content == NULL )
  {
    return;
  }

  locked = 0;

  html_tag_alloc( &html->tags, NULL );
  current = html->tags;
  do{
    ptr_content = strchr( ptr_content, '<' );
    if ( ptr_content == NULL )
    {
      break;
    }

    html_tag_init( html, ptr_content, &current );

    current->textStart = ptr_content + current->length + 2 - html->content;
    if ( current->prev != NULL )
    {
      current->prev->textLength = current->textStart - current->prev->textStart - ( current->length + 2 );
    }

    ptr_content = (char*)html->content + current->textStart;

    if ( !strcmpi( current->name, "script" ) )
    {
      locked = 1;
      html_tag_alloc( &current->next, current );
      current = current->next;
    }
    else if ( !strcmpi( current->name+1, "script" ) )
    {
      locked = 0;
    }

    if ( locked == 0 )
    {
      if ( current->length > 0 )
      {
        html_tag_alloc( &current->next, current );
        current = current->next;
      }
    }
    else
    {
      html_free_tags( current );
    }
  }while( 1 );

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
  
  if ( html_get_opt( html, HTML_OPTION_CHARSET ) == 1 )
  {
    html_check_charset( html );
  }
}
void html_check_charset( struct HTML* html )
{
  struct HTML_LIST *list, *list_it;
  char* field, *charset;
  
  html_list_create( html->tags, NULL, "meta", &list );
  HTML_LIST_FOREACH( list, list_it )
  {
    html_tag_get_parameter_field( list_it->tag->parameter, "content", &field, NULL );
    if ( field != NULL )
    {
      html_tag_get_parameter_field( field, "charset", &charset, NULL );
      free( field );
      html_apply_charset( html, charset );
      break;
    }
  }
  html_list_free( list );
}
void html_apply_charset( struct HTML* html, const char* charset )
{
  if ( charset == NULL )
  {
    return;
  }
  if ( !strcmpi( "utf-8", charset ) )
  {
    html_apply_charset_utf8( html );
  }
  if ( !strcmpi( "iso-8859-1", charset ) )
  {
    // Not implemented yet
  }
}

void html_apply_charset_utf8( struct HTML* html )
{
  // UTF-8 decoding
  enum CODED {
    C_AE_G = 0xC4,
    C_AE_S = 0xE4,
    C_OE_G = 0xD6,
    C_OE_S = 0xF6,
    C_SS   = 0xDF,
    C_UE_G = 0xDC,
    C_UE_S = 0xFC,
    C_MINUS = 0x96,
    C_QUOTE_1 = 0x93,
    C_QUOTE_2 = 0x94
  };
  /*enum DECODED {
    D_AE_G = 196,
    D_AE_S = 228,
    D_OE_G = 214,
    D_OE_S = 246,
    D_SS   = 223,
    D_UE_G = 220,
    D_UE_S = 252
  };*/
  enum DECODED {
    D_AE_G = 'A',
    D_AE_S = 'a',
    D_OE_G = 'O',
    D_OE_S = 'o',
    D_SS   = 's',
    D_UE_G = 'U',
    D_UE_S = 'u',
    D_MINUS = '-',
    D_QUOTE = '\"'
  };
  
  struct HTML_TAG *tag_it;
  long count;
  char *c;
  
  HTML_TAG_FOREACH( html->tags, tag_it )
  {
    if ( tag_it->textLength > 0 )
    {
      // Do decoding
      for ( count = 0 ; count < tag_it->textLength ; count++ )
      {
        c = html->content+tag_it->textStart+count;
        switch( (unsigned char)*c )
        {
          case C_AE_S: *c = D_AE_S; break;
          case C_AE_G: *c = D_AE_G; break;
          case C_OE_S: *c = D_OE_S; break;
          case C_OE_G: *c = D_OE_G; break;
          case C_UE_S: *c = D_UE_S; break;
          case C_UE_G: *c = D_UE_G; break;
          case C_SS: *c = D_SS; break;
          case C_MINUS: *c = D_MINUS; break;
          case C_QUOTE_1: *c = D_QUOTE; break;
          case C_QUOTE_2: *c = D_QUOTE; break;
          default: break;
        }
      }
    }
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
  return ( html->options & ( 1<<option ) ) ? 1 : 0;
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
  free( html->content );
}
