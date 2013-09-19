#include "http_link_info.h"

/** Internal prototypes */
HTTP_BOOL http_is_alphanumeric( const char* sLink, unsigned int size );
HTTP_BOOL http_is_ip( const char* sLink );
void http_get_link_protocol( const char* sLink, char** sProtocol, int* length );
void http_get_link_host( const char* sLink, char** sHost, int* length );
void http_get_link_subdir( const char* sLink, char** sSubdir, int* length );
void http_get_link_file( const char* sLink, char** sFile, int* length );
void http_get_link_parameter( const char* sLink, char** sParameter, int* length );

HTTP_BOOL http_is_numeric( const char* sLink, unsigned int size )
{
  unsigned int i;

  for ( i = 0 ; i < size ; i++ )
  {
    if ( *(sLink+i) < '0' || *(sLink+i) > '9' )
    {
      return HTTP_BOOL_FALSE;
    }
  }
  return HTTP_BOOL_TRUE;
}
HTTP_BOOL http_is_ip( const char* sLink )
{
  int size;
  unsigned int i;
  int dotCounter;
  char* ptrLink;
  char* ptrDot;

  size = strlen( sLink );

  dotCounter = 0;
  for ( i = 0 ; i < size ; i++ )
  {
    if ( *(sLink+i) == '.' )
    {
      dotCounter++;
    }
  }
  if ( dotCounter != 3 )
  {
    return HTTP_BOOL_FALSE;
  }

  for ( i = 0 ; i < size ; )
  {
    ptrLink = (char*)sLink+i;
    ptrDot = strstr( (char*)(sLink+i), "." );
    if ( ptrDot == NULL )
    {
      if ( http_is_numeric( ptrLink, strlen( ptrLink ) ) == HTTP_BOOL_FALSE )
      {
        return HTTP_BOOL_FALSE;
      }
      i += strlen( ptrLink ) + 1;
    }
    else if ( http_is_numeric( ptrLink, ptrDot-ptrLink ) == HTTP_BOOL_FALSE )
    {
      return HTTP_BOOL_FALSE;
    }
    else
    {
      i += ptrDot-ptrLink + 1;
    }
  }
  return HTTP_BOOL_TRUE;
}
void http_link_get_size( struct HTTP_LINK_INFO* http_link_info )
{
  http_link_info->size = 1;

  if ( http_link_info->subdir != NULL )
  {
    http_link_info->size += strlen( http_link_info->subdir );
  }
  if ( http_link_info->file != NULL )
  {
    http_link_info->size += strlen( http_link_info->file );
  }
  http_link_info->size += 1;
  if ( http_link_info->parameters != NULL )
  {
    http_link_info->size += strlen( http_link_info->parameters );
  }
}
void http_get_link_protocol( const char* sLink, char** sProtocol, int* length )
{
  int i, size;

  *sProtocol = NULL;
  *length = 0;
  size = strlen( sLink );

  for ( i = 0 ; i < size-3 && i <= 7 ; i++ )
  {
    if( sLink[i] == ':' && sLink[i+1] == '/' && sLink[i+2] == '/' )
    {
      break;
    }
  }
  if ( i <= 7 && size-i > 3 )
  {
    *sProtocol = (char*)malloc( i+1 );
    memcpy( *sProtocol, sLink, i );
    memset( *sProtocol+i, 0, 1 );
    *length = i;
  }
}
void http_get_link_host( const char* sLink, char** sHost, int* length )
{
  FILE *fTld;
  unsigned int i, size, j;
  char* sLine, *pointer;

  static const char* TLD_OFFLINE[] = {
    "de", "com", "org", "net", "co.uk", "at", "edu", "info", "gov", "tv", "ru", "ch", "to", "fr"
  };

  *sHost = NULL;
  *length = 0;
  size = strlen( sLink );

  for ( i = 0 ; i < size && sLink[i] != '/' && sLink[i] != '?' && sLink[i] != ':' ; i++ );
  if ( i > 0 )
  {
    *sHost = (char*)malloc( i+1 );
    memcpy( *sHost, sLink, i );
    memset( *sHost+i, 0, 1 );

    pointer = *sHost+i-3;
    if ( *pointer == '.' )
    {
      // For 2 char tld like "at"
      pointer++;
    }

    if ( http_is_ip( *sHost ) == HTTP_BOOL_TRUE )
    {
      *length = i;
      return;
    }

    fTld = fopen( "http_wrapper.tld", "r" );
    if ( fTld == NULL )
    {
      for ( j = 0 ; j < sizeof( TLD_OFFLINE ) / sizeof( char* ) ; j++ )
      {
        if ( !strncmpi( pointer, TLD_OFFLINE[j], strlen( TLD_OFFLINE[j] ) ) )
        {
          break;
        }
      }
      if ( j == sizeof( TLD_OFFLINE ) / sizeof( char* ) )
      {
        free( *sHost );
        *sHost = NULL;
        *length = 0;
        return;
      }
      *length = i;
      return;
    }
    sLine = (char*)malloc( 5 );
    while( fgets( sLine, 5, fTld ) != NULL )
    {
      if ( !strncmpi( pointer, sLine, strlen( pointer ) ) )
      {
        break;
      }
    }
    free( sLine );
    if ( feof( fTld ) )
    {
      free( *sHost );
      *sHost = NULL;
      *length = 0;
      return;
    }
    fclose( fTld );
    *length = i;
  }
}
void http_get_link_port( const char* sLink, char** sPort, int* length )
{
  int i, size;

  *sPort = NULL;
  *length = 0;
  size = strlen( sLink );

  for ( i = 0 ; i < size && sLink[i] != '/' ; i++ );
  if ( i > 0 && i < size )
  {
    *sPort = (char*)malloc( i+1 );
    memcpy( *sPort, sLink, i );
    memset( *sPort+i, 0, 1 );
    *length = i;
  }
}
void http_get_link_subdir( const char* sLink, char** sSubdir, int* length )
{
  int i, size, numOfSubdirs;

  *sSubdir = NULL;
  *length = 0;

  if ( sLink == NULL || *sLink == '\0' )
  {
    return;
  }
  size = strlen( sLink );

  numOfSubdirs = 0;
  for ( i = 0 ; i < size ; i++ )
  {
    if( sLink[i] == '/' )
    {
      numOfSubdirs++;
    }
  }
  for ( i = 0 ; numOfSubdirs > 0 ; i++ )
  {
    if( sLink[i] == '/' )
    {
      numOfSubdirs--;
    }
  }
  if ( i > 0 )
  {
    *sSubdir = (char*)malloc( i+1 );
    memcpy( *sSubdir, sLink, i );
    memset( *sSubdir+i, 0, 1 );
    *length = i-1;
  }
}
void http_get_link_file( const char* sLink, char** sFile, int* length )
{
  int i, size;

  *sFile = NULL;
  *length = 0;
  size = strlen( sLink );

  for ( i = 0 ; i < size && sLink[i] != '?' ; i++ );
  if ( i > 0 )
  {
    *sFile = (char*)malloc( i+1 );
    memcpy( *sFile, sLink, i );
    memset( *sFile+i, 0, 1 );
    *length = i;
  }
}
void http_get_link_parameter( const char* sLink, char** sParameter, int* length )
{
  *sParameter = NULL;
  *length = 0;

  html_replace_special_chars( sLink, sParameter );
  *length = strlen( sLink );
}
void http_link_get_info ( struct HTTP* http, struct HTTP_LINK_INFO* http_info, const char* sLink )
{
  char* sProtocol, *sHost, *sPort, *sSubdir, *sFile, *sParameter, *sSubdir_old, *sTmp;
  int size, currentSize, totalSize;

  totalSize = strlen( sLink );

  memset( http_info, 0, sizeof( struct HTTP_LINK_INFO ) );

  http_get_link_protocol( sLink, &sProtocol, &currentSize );
  size = currentSize;
  if ( sProtocol != NULL )
  {
    size += 3;
  }

  sPort = NULL;
  http_get_link_host( sLink+size, &sHost, &currentSize );
  if ( sHost != NULL )
  {
    size += currentSize;

    if ( size < totalSize )
    {
      if ( *(sLink+size) == ':' )
      {
        size += 1;
        http_get_link_port( sLink+size, &sPort, &currentSize );
        if ( sPort != NULL && http_is_numeric( sPort, strlen( sPort ) ) == 0 )
        {
          sPort = NULL;
        }
        else
        {
          size += currentSize;
        }
      }
    }
  }

  http_get_link_subdir( sLink+size, &sSubdir, &currentSize );
  if ( sSubdir != NULL )
  {
    size += currentSize + 1;
  }

  http_get_link_file( sLink+size, &sFile, &currentSize );
  if ( sFile != NULL )
  {
    size += currentSize + 1;
  }

  sParameter = NULL;
  if ( size < totalSize )
  {
    if ( *(sLink+size) == '?' )
    {
      size += 1;
    }
    http_get_link_parameter( sLink+size, &sParameter, &currentSize );
    size += currentSize;
  }

  /** Afterprocessing data */
  if ( sProtocol != NULL )
  {
    //size -= strlen( sProtocol ) + 3;

    if ( strcmpi( sProtocol, "http" ) && strcmpi( sProtocol, "https" ) )
    {
      http->error.errorId = HTTP_ERROR_ONLY_HTTP_SUPPORTED;
      http->error.line = __LINE__;
      http->error.file = __FILE__;
      return;
    }
  }
  else
  {
    if ( http->port == 80 || http->port == 0 )
    {
      sProtocol = new_string( "http" );
    }
    else if ( http->port == 443 )
    {
      sProtocol = new_string( "https" );
    }
    else
    {
      http->error.errorId = HTTP_ERROR_ONLY_HTTP_SUPPORTED;
      http->error.line = __LINE__;
      http->error.file = __FILE__;
      return;
    }
  }

  if ( sHost == NULL )
  {
    sHost = new_string( http->server );
    if ( sHost != NULL )
    {
      size += strlen( sHost );
    }
  }

  if ( sSubdir == NULL )
  {
    http_get_link_subdir( http->header->remote_file, &sSubdir_old, &currentSize );
    sSubdir = sSubdir_old;
    size += currentSize + 1;
  }
  else if ( *sSubdir != '/' )
  {
    sSubdir_old = sSubdir;

    sSubdir = NULL;
    http_get_link_subdir( http->header->remote_file, &sSubdir, &currentSize );

    sTmp = (char*)malloc( currentSize + strlen( sSubdir_old ) + 1 );
    strcpy( sTmp, sSubdir );
    strcat( sTmp, sSubdir_old );

    free( sSubdir );
    free( sSubdir_old );
    sSubdir = sTmp;
    sTmp = NULL;
  }

  if ( sPort == NULL )
  {
    if ( !strcmpi( sProtocol, "https" ) )
    {
      sPort = new_string("443");
    }
    else if ( !strcmpi( sProtocol, "http" ) )
    {
      sPort = new_string("80");
    }
  }

  http_info->protocol   = sProtocol;
  http_info->host       = sHost;
  http_info->subdir     = sSubdir;
  http_info->file       = sFile;
  http_info->port       = sPort;
  http_info->parameters = sParameter;

  http_link_get_size( http_info );
  return;
}
void http_link_info_free( struct HTTP_LINK_INFO* http_link_info )
{
  free( http_link_info->file );
  free( http_link_info->host );
  free( http_link_info->parameters );
  free( http_link_info->protocol );
  free( http_link_info->subdir );
  free( http_link_info->port );
  memset( http_link_info, 0, sizeof( struct HTTP_LINK_INFO ) );
}
