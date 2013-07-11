#include "http_cookies.h"

/** Internal prototypes */
void http_header_cookie_get_data( const char* line, enum HTTP_COOKIE_FIELD cookieField, char** sValue, unsigned int* size );
void http_header_cookie_get_tokens( enum HTTP_COOKIE_FIELD cookieField, char** field, char* tokBeg, char* tokEnd );
struct HTTP_COOKIE* http_cookie_update( struct HTTP* http, struct HTTP_COOKIE* currentCookie );

int http_header_cookie_validate_expires( const char* cookie )
{
  regex_t regex;
  int ret;

  if ( cookie == NULL )
  {
    return REG_NOMATCH;
  }
  ret = regcomp(&regex, "(Mon|Tue|Wed|Thu|Fri|Sat|Sun), ([0-2][0-9]|[3][0-1])-(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)-[1-2][0-9]{3} ([0-9]|[0-1][0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9] GMT", REG_EXTENDED);
  ret = regexec(&regex, cookie, 0, NULL, 0);

  regfree(&regex);
  return ret;
}

void http_header_cookie_sqlite_expires_date_convert_month( const char* month_name, char** month_id )
{
  unsigned char i;
  char* months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

  for ( i = 0 ; i < ( sizeof( months ) / sizeof( char* ) ) ; i++ )
  {
    if ( !strcmp( months[i], month_name ) )
    {
      break;
    }
  }
  *month_id = (char*)malloc( 3 );
  sprintf( *month_id, "%.2i", i+1 );
  return;
}
void http_header_cookie_sqlite_expires_date( const char* date_str, char** sqlite_date )
{
  char* ptr, *day = NULL, *month = NULL, *year = NULL, *month_dec = NULL;
  unsigned char count;

  *sqlite_date = (char*)malloc( strlen( date_str ) );

  ptr = strtok( (char*)date_str, "-" );
  for ( count = 0 ; ptr != NULL ; count++ )
  {
    switch( count )
    {
      case 0: // Day
        day = new_string( ptr );
        break;
      case 1: // Month ( short )
        month = new_string( ptr );
        break;
      case 2: // Year
        year = new_string( ptr );
        break;
      default: break;
    }
    ptr = strtok( NULL, "-" );
  }
  http_header_cookie_sqlite_expires_date_convert_month( month, &month_dec );
  sprintf( *sqlite_date, "%s-%s-%s", year, month_dec, day );

  free( day );
  free( month );
  free( year );
  free( month_dec );

  return;
}
void http_header_cookie_sqlite_expires( const char* exp_str, char** sqlite_str )
{
  char* exp_str_date;
  char* sqlite_date, *sqlite_time;
  unsigned int size;

  exp_str_date = new_string_num( exp_str + 5, 11 );

  http_header_cookie_sqlite_expires_date( exp_str_date, &sqlite_date );
  sqlite_time = new_string_num( exp_str + 17, 8 );

  size = strlen( sqlite_date ) + 1 + strlen( sqlite_time );
  *sqlite_str = (char*)malloc( size + 1);

  sprintf( *sqlite_str, "%s %s", sqlite_date, sqlite_time );

  free( sqlite_date );
  free( sqlite_time );
}

void http_sqlite_cookie_server( const char* server, char** sqlite_server )
{
  int i, count_total;

  i = strlen( server ) - 1;
  count_total = 2;

  for ( ; i >= 0 && count_total > 0 ; i-- )
  {
    if ( server[i] == '.' )
      count_total--;
  }
  *sqlite_server = new_string( server+i+1 );
  /*
  for ( i = 0 ; server[i] != '\0' ; i++ )
  {
    if ( server[i] == '.' )
      count_total++;
  }

  count_sub = count_total - 1;
  for ( i = 0 ; server[i] != '\0' && count_sub > 0 ; i++ )
  {
    if ( server[i] == '.' )
      count_sub--;
  }
  *sqlite_server = new_string( server+i );
  */
}
void http_sqlite_cookie_path( const char* path, char** sqlite_path )
{
  //char* ptr;
  //int pos;

  *sqlite_path = new_string( "/" );
  return;

  /*ptr = strrchr( path, '/' );
  if( ptr == NULL )
  {
    *sqlite_path = NULL;
    return;
  }
  pos = ptr - path + 1;
  *sqlite_path = new_string_num( path, pos );*/
}

unsigned char http_header_cookie_user_add( struct HTTP* http,
  const char* name, const char* value, const char* domain, const char* path,
  const char* expires, unsigned char secure, unsigned char http_only )
{
  char* cookieStr;

  if ( name == NULL )
  {
    return 0;
  }
  cookieStr = (char*)malloc( 1024 );
  memset( cookieStr, 0, 1024 );

  sprintf( cookieStr, "Set-Cookie: %s=%s; ", name, ( value == NULL ) ? "" : value );

  strcat( cookieStr, "Domain=" );
  if ( domain )
  {
    strcat( cookieStr, domain );
  }
  else
  {
    if ( http->server )
    {
      strcat( cookieStr, http->server );
    }
    else
    {
      /// No domain to cookie
    }
  }
  strcat( cookieStr, "; " );

  strcat( cookieStr, "Path=" );
  if ( path )
  {
    strcat( cookieStr, path );
  }
  else
  {
    strcat( cookieStr, "/" );
  }
  strcat( cookieStr, "; " );

  if ( expires )
  {
    strcat( cookieStr, "Expires=" );
    strcat( cookieStr, expires );
    strcat( cookieStr, "; " );
  }

  if ( secure )
  {
    strcat( cookieStr, "Secure" );
    strcat( cookieStr, "; " );
  }

  if ( http_only )
  {
    strcat( cookieStr, "HttpOnly" );
  }
  http_header_cookie_add( http, cookieStr );
  free( cookieStr );
  return 1;
}
void http_header_cookie_add( struct HTTP* http, const char* line )
{
  unsigned int currentSize;
  struct HTTP_COOKIE* currentCookie;

  currentCookie = (struct HTTP_COOKIE*)malloc( sizeof( struct HTTP_COOKIE ) );
  memset( currentCookie, 0, sizeof( struct HTTP_COOKIE ) );

  http_header_cookie_get_data( line, HTTP_COOKIE_NAME,      &currentCookie->name,     &currentSize );
  http_header_cookie_get_data( line, HTTP_COOKIE_VALUE,     &currentCookie->value,    &currentSize );
  http_header_cookie_get_data( line, HTTP_COOKIE_DOMAIN,    &currentCookie->domain,   &currentSize );
  http_header_cookie_get_data( line, HTTP_COOKIE_PATH,      &currentCookie->path,     &currentSize );
  http_header_cookie_get_data( line, HTTP_COOKIE_EXPIRES,   &currentCookie->expires,  &currentSize );
  http_header_cookie_get_data( line, HTTP_COOKIE_SECURE,    &currentCookie->secure,  &currentSize );
  http_header_cookie_get_data( line, HTTP_COOKIE_HTTP_ONLY, &currentCookie->http_only,  &currentSize );

  if ( currentCookie->path == NULL )
  {
    currentCookie->path = new_string( "/" );
  }
  if ( currentCookie->domain == NULL )
  {
    currentCookie->domain = new_string( http->server );
  }
  http_sqlite_cookie_update( http, currentCookie );
  http_header_cookie_free( currentCookie );
}

void http_header_cookie_free( struct HTTP_COOKIE* cookie )
{
  if ( cookie == NULL )
  {
    return;
  }
  http_header_cookie_free( cookie->next );
  free( cookie->name );
  free( cookie->value );
  free( cookie->expires );
  free( cookie->path );
  free( cookie->domain );
  free( cookie->secure );
  free( cookie->http_only );
  free( cookie->next );

  cookie->name      = NULL;
  cookie->value     = NULL;
  cookie->expires   = NULL;
  cookie->path      = NULL;
  cookie->domain    = NULL;
  cookie->secure    = NULL;
  cookie->http_only = NULL;
  cookie->next      = NULL;
}
void http_header_cookie_get_data( const char* line, enum HTTP_COOKIE_FIELD cookieField, char** sValue, unsigned int* size )
{
  char* valueStart, *valueEnd, *field, *ptr;
  char tokBeg, tokEnd;
  int sizeTotal, i;

  if ( line == NULL )
  {
    *size = 0;
    *sValue = NULL;
    return;
  }
  sizeTotal = strlen( line );

  http_header_cookie_get_tokens( cookieField, &field, &tokBeg, &tokEnd );

  ptr = ( field == NULL ) ? (char*)line : stristr( line, field );

  if ( ptr == NULL )
  {
    *size = 0;
    *sValue = NULL;
    return;
  }
  if ( tokBeg == '\0' )
  {
    *sValue = (char*)malloc( 2 );
    **sValue = ( ptr != NULL ) ? '\001' : '\000';
    *(*sValue+1) = '\000';
    return;
  }

  valueStart = strchr( ptr, tokBeg ) + 1;
  valueEnd = strchr( ptr, tokEnd );
  if ( valueEnd == NULL )
  {
    valueEnd = ptr + strlen(ptr);
  }

  i = valueEnd - valueStart;
  if ( i <= sizeTotal )
  {
    *sValue = (char*)malloc( i+1 );
    memcpy( *sValue, valueStart, i );
    memset( *sValue+i, 0, 1 );
    *size = i;
  }
  else
  {
    *sValue = NULL;
    *size = 0;
  }
}
void http_header_cookie_get_tokens( enum HTTP_COOKIE_FIELD cookieField, char** field, char* tokBeg, char* tokEnd )
{
  *field = NULL;

  *tokBeg = '=';
  *tokEnd = ';';
  switch( cookieField )
  {
    case HTTP_COOKIE_NAME:
      *tokBeg = ' ';
      *tokEnd = '=';
      break;
    case HTTP_COOKIE_VALUE:
      break;
    case HTTP_COOKIE_DOMAIN:
      *field = new_string( "Domain=" );
      break;
    case HTTP_COOKIE_PATH:
      *field = new_string( "Path=" );
      break;
    case HTTP_COOKIE_EXPIRES:
      *field = new_string( "Expires=" );
      break;
    case HTTP_COOKIE_SECURE:
      *field = new_string( "Secure" );
      *tokBeg = '\0';
      break;
    case HTTP_COOKIE_HTTP_ONLY:
      *field = new_string( "HttpOnly" );
      *tokBeg = '\0';
      break;
  }
}
