#include "http_wrapper.h"

/** Internal prototypes */
void http_response_status( struct HTTP* http, const char* line, const int size );
int http_recv( struct HTTP* __http, void *__buf, size_t __n );
int http_send( struct HTTP* __http, const char *__buf, size_t __n );
void http_raw_send( struct HTTP* http, const char* sData, int iLength );
void http_recv_chunked( struct HTTP* http, char** content, int* size );
void http_recv_content( struct HTTP* http, char** pContent, int* size );
void http_reconnect( struct HTTP* http, const char* sNewHost, const unsigned short port );
void http_get_authorization_password( struct HTTP* http, char** str );
int http_save_data_to_file( struct HTTP* http, const char* file );

void http_print_errorcode(  struct HTTP* http )
{
  int length;
  char* text;

  if ( http->error.errorId == 0 )
  {
    return;
  }
  length = 200;
  text = (char*)malloc( length );

  if ( http->header->status.responseId != 0 )
  {
    printf( "\n\nLast header: %i %s\n", http->header->status.responseId, http->header->status.responseText );
  }

  switch( http->error.errorId )
  {
    case HTTP_ERROR_SOCKET_ERROR:
      snprintf( text, length, "SOCKET_ERROR: Socket not valid or closed unexpectly" );
      break;
    case HTTP_ERROR_HOST_MISSING:
      snprintf( text, length, "HOST_MISSING: Cannot connect to an empty host entry" );
      break;
    case HTTP_ERROR_HOST_LOOKUP_FAILURE:
      snprintf( text, length, "HOST_LOOKUP_FAILURE: The given host %s cannot be resolved", http->server );
      break;
    case HTTP_ERROR_CONNECT_FAILED:
      snprintf( text, length, "CONNECT_FAILED: Connection to %s has failed or timed out", http->server );
      break;
    case HTTP_ERROR_RAW_SEND_ERROR:
      snprintf( text, length, "RAW_SEND_ERROR: Data could not been sent because socket was invalid" );
      break;
    case HTTP_ERROR_RAW_RECV_ERROR:
      snprintf( text, length, "RAW_RECV_ERROR: Data could not been received because socket was invalid" );
      break;
    case HTTP_ERROR_RECV_ERROR:
      snprintf( text, length, "RECV_ERROR: Not all data could be received" );
      break;
    case HTTP_ERROR_EXPECT_LINEFEED:
      snprintf( text, length, "EXPECT_LINEFEED: Header was not in the right format" );
      break;
    case HTTP_ERROR_CONTENT_SIZE_UNKNOWN:
      snprintf( text, length, "CONTENT_SIZE_UNKNOWN: I don't know how much I should receive" );
      break;
    case HTTP_ERROR_CONNECTION_CLOSED:
      snprintf( text, length, "CONNECTION_CLOSED: Connection was closed unexpectly" );
      break;
    case HTTP_ERROR_ONLY_HTTP_SUPPORTED:
      snprintf( text, length, "ONLY_HTTP/HTTPS_SUPPORTED: In this version only HTTP(S) is supported" );
      break;
    case HTTP_ERROR_NO_STATUS_CODE_RECIEVED:
      snprintf( text, length, "NO_STATUS_CODE_RECIEVED: Server does not send the header in right format" );
      break;
    case HTTP_ERROR_NO_POST_DATA_PRESENT:
      snprintf( text, length, "NO_POST_DATA_PRESENT: You have to try sending post data. But there is not any data" );
      break;
    case HTTP_ERROR_NOT_IMPLEMENTED_YET:
      snprintf( text, length, "NOT_IMPLEMENTED_YET: The function the server requests or client has to apply, is not implemented" );
      break;
    case HTTP_ERROR_UNEXPECTED_RESPONSE:
      snprintf( text, length, "UNEXPECTED_RESPONSE: I expect another response from the server" );
      break;
    case HTTP_ERROR_DOWNLOAD_SAVE_FILE:
      snprintf( text, length, "DOWNLOAD_SAVE_FILE: Cannot save the requested file to disk. Check write permissions" );
      break;
    case HTTP_ERROR_DOWNLOAD_FILE_TOO_BIG:
      snprintf( text, length, "DOWNLOAD_FILE_TOO_BIG: The response of the query is too big to fit in memory" );
      break;
    case HTTP_ERROR_POST_METHOD_UNKNOWN:
      snprintf( text, length, "HTTP_ERROR_POST_METHOD_UNKNOWN: Unknown post method detected" );
    case HTTP_ERROR_SSL_INIT_FAILED:
      snprintf( text, length, "HTTP_ERROR_SSL_INIT_FAILED: Initialization of (polarssl's) ssl-socket failed" );
    case HTTP_ERROR_SSL_ENTROPY_INIT_FAILED:
      snprintf( text, length, "HTTP_ERROR_SSL_ENTROPY_INIT_FAILED: Initialization of (polarssl's) ssl entropy failed" );
    default:
      snprintf( text, length, "ERROR: %i %s", http->header->status.responseId, http->header->status.responseText );
      break;
  }
  if ( http->header->status.responseId == 0 )
  {
    printf( "\n" );
  }
  printf( "%s\n", text );
  printf( "Error set on line %d in \"%s\"\n", http->error.line, http->error.file );
  if ( !http_get_opt( http, HTTP_OPTION_VERBOSE ) )
  {
    printf( "\nIncrease verbosity to get more information about the error\n");
  }
  fflush( stdout );
  free( text );
  return;
}
void http_init( struct HTTP* http )
{
  http_alloc( http, HTTP_INIT );
}
void http_alloc( struct HTTP* http, HTTP_HEX reset )
{
  unsigned long options;
  int lastResult;
  char* server, *download_folder, *post_data;
  int socket;
  unsigned int post_data_size;
  struct HTTP_HEADER* header;
  struct HTTP_POST_FORM_URLENCODED_DATA_ITEM* post_form_urlencoded_data;

  void (*connect_func)(struct HTTP*);
  int (*recv_func)(struct HTTP*, void*, size_t);
  int (*send_func)(struct HTTP*, const char*, size_t);
  void (*close_func)(struct HTTP*);

  if ( reset & HTTP_RESET )
  {
    memset( http, 0, sizeof( struct HTTP ) );
    http->socket = -1;
    http->connect_func = &http_raw_connect;
    http->recv_func    = &http_recv;
    http->send_func    = &http_send;
    http->close_func   = &http_close;
  }

  if ( reset & HTTP_FREE_WITHOUT_PERSISTENT_DATA )
  {
    return;
  }
  if ( reset & HTTP_INIT )
  {
    http_alloc( http, HTTP_RESET );

    firedns_init( &http->firedns );
    firedns_add_server( &http->firedns, "8.8.8.8" );

    sqlite3_open( HTTP_SQLITE_DB, &http->sqlite_handle );

    http_sqlite_db_create( http );
    http_sqlite_db_startup( http );

    // Release postdata by default
    http_set_opt( http, HTTP_OPTION_POST_DATA_RELEASE, 1 );

    http_header_init( &http->header, HTTP_HEADER_INIT );

#if defined _WIN32 || defined _WIN64
    WSADATA wsa;
    WSAStartup( MAKEWORD( 2,0 ), &wsa );
#endif
  }
  if ( reset & HTTP_FREE )
  {
    if ( !http_get_opt( http, HTTP_OPTION_SQLITE_DB_DISABLED ) )
    {
      sqlite3_close( http->sqlite_handle );
    }

    http_header_init( &http->header, HTTP_HEADER_FREE );
    free( http->server );
    free( http->download_folder );
    free( http->post_data );
    free( http->header );
    http_post_form_urlencoded_free( http->post_form_urlencoded_data );

    http_alloc( http, HTTP_RESET );
  }
  if ( reset & HTTP_FREE_WITHOUT_PERSISTENT_DATA )
  {
    server = http->server;
    socket = http->socket;
    options = http->options;
    download_folder = http->download_folder;
    lastResult = http->last_result;
    post_data = http->post_data;
    post_data_size = http->post_data_size;
    post_form_urlencoded_data = http->post_form_urlencoded_data;

    connect_func = http->connect_func;
    recv_func = http->recv_func;
    send_func = http->send_func;
    close_func = http->close_func;

    http->server = NULL;
    http->download_folder = NULL;
    http->post_data = NULL;
    http->post_form_urlencoded_data = NULL;

    if ( reset & HTTP_HEADER_FREE_WITHOUT_PERSISTENT_DATA )
    {
      http_header_init( &http->header, HTTP_HEADER_FREE_WITHOUT_PERSISTENT_DATA );
      header = http->header;
      http_alloc( http, HTTP_RESET );
      http->header = header;
    }
    else
    {
      http_header_init( &http->header, HTTP_HEADER_FREE );
      http_alloc( http, HTTP_RESET );
    }
    http->server = server;
    http->socket = socket;
    http->options = options;
    http->download_folder = download_folder;
    http->last_result = lastResult;
    http->post_data = post_data;
    http->post_data_size = post_data_size;
    http->post_form_urlencoded_data = post_form_urlencoded_data;

    http->connect_func = connect_func;
    http->recv_func = recv_func;
    http->send_func = send_func;
    http->close_func = close_func;
  }
}
void http_free( struct HTTP* http )
{
  http_alloc( http, HTTP_FREE );
  return;
}

void http_raw_connect( struct HTTP* http )
{
  http->dns_result = firedns_resolveip4( &http->firedns, http->server );
  if ( http->dns_result == NULL )
  {
    http->error.errorId = HTTP_ERROR_HOST_LOOKUP_FAILURE;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
    return;
  }

  memset( &http->addr, 0, sizeof( http->addr ) );

  http->addr.sin_family = AF_INET;
  memcpy( &http->addr.sin_addr.s_addr, &http->dns_result->s_addr, sizeof( http->dns_result->s_addr ) );
  http->addr.sin_port = htons( http->port );

  http->last_result = connect( http->socket, (struct sockaddr*)&http->addr, sizeof( http->addr ) );
  if ( http->last_result == -1 )
  {
    http->error.errorId = HTTP_ERROR_CONNECT_FAILED;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
  }
  if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
  {
    printf("%s\n", http->last_result != -1 ? " success" : " failed" );
    fflush( stdout );
  }
}

void http_connect( struct HTTP* http, const char* host, const unsigned short port )
{
  if ( http->error.errorId != 0 )
  {
    return;
  }
  if ( host == NULL )
  {
    http->error.errorId = HTTP_ERROR_HOST_MISSING;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
    return;
  }
  http->server = new_string( host );
  http->port = port;

  http_set_opt( http, HTTP_OPTION_APPLY_ALL, 0 );

  http->socket = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
  if ( http->socket < 0 )
  {
    http->error.errorId = HTTP_ERROR_SOCKET_ERROR;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
    return;
  }

  http_set_opt( http, HTTP_OPTION_APPLY_ALL, 0 );

  if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
  {
    printf("Connecting to %s:%i ...", http->server, http->port );
    fflush( stdout );
  }

  http->connect_func( http );
  return;
}
HTTP_BOOL http_get_opt( struct HTTP* http, enum HTTP_OPTION_STATUS option )
{
  if ( http->error.errorId != 0 )
  {
    return 0;
  }
  http->last_result = 0;

  return http->options & (1<<option) ? HTTP_BOOL_TRUE : HTTP_BOOL_FALSE;
}
void http_set_opt( struct HTTP* http, enum HTTP_OPTION_STATUS option, ... )
{
  struct timeval timeout;
  va_list tags;
  void* value;

  if ( http->error.errorId != 0 )
  {
    return;
  }
  http->last_result = 0;

  va_start( tags, option );

  /** Options with special function follows here */
  switch( option )
  {
    case HTTP_OPTION_RECV_TIMEOUT:
      value = va_arg( tags, unsigned int* );
      if ( value ) http->options |= 1<<option;
      else http->options &= ~(1<<option);

      timeout.tv_sec  = (time_t)value;
      timeout.tv_usec = 0;
      http->last_result = setsockopt ( http->socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval) );
      break;
    case HTTP_OPTION_SEND_TIMEOUT:
      value = va_arg( tags, unsigned int* );
      if ( value ) http->options |= 1<<option;
      else http->options &= ~(1<<option);

      timeout.tv_sec  = (time_t)value;
      timeout.tv_usec = 0;
      http->last_result = setsockopt ( http->socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout) );
      break;
    case HTTP_OPTION_CONNECT_TIMEOUT:
      http->error.errorId = HTTP_ERROR_NOT_IMPLEMENTED_YET;
      http->error.line = __LINE__;
      http->error.file = __FILE__;
      break;
    case HTTP_OPTION_DOWNLOAD_FOLDER:
      value = va_arg( tags, char* );
      http->download_folder = new_string( (char*)value );

      http->options |= 1<<option;
      break;
    default:
      value = va_arg( tags, void* );
      if ( value ) http->options |= 1<<option;
      else http->options &= ~(1<<option);
      break;
  }
  va_end( tags );

  if ( http->last_result < 0 )
  {
    printf("Failed to set option: %i\n", option );
    fflush( stdout );
  }
  return;
}
void http_prepare_header( struct HTTP* http, char** query )
{
  struct HTTP_HEADER_FIELD_ITERATOR header_field_it = {0};
  struct HTTP_HEADER_FIELD* header_field = NULL;
  char* sqlite_query, *sqlite_server, *sqlite_path, *cookies;

  if ( http->error.errorId != 0 )
  {
    return;
  }

  *query = (char*)malloc( 1*1024*1024 );
  memset( *query, 0, 1*1024*1024 );

  /** Method */
  if ( http->header->method == NULL )
  {
    if ( http->post_method != HTTP_POST_METHOD_NONE )
    {
      http->header->method = new_string("POST");
    }
    else
    {
      http->header->method = new_string("GET");
    }
  }
  strcat( *query, http->header->method );
  strcat( *query, " " );

  /** Only for proxies */
  /** Potocol */
  /*strcat( *query, "http://" );*/

  /** Host */
  /*if ( http->server == NULL )
  {
    http->header->server = new_string("");
  }
  strcat( *query, http->server );
  */

  /** Remote file */
  if ( http->header->remote_file == NULL )
  {
    http->header->remote_file = new_string("/");
  }
  else if ( *http->header->remote_file != '/' )
  {
    strcat( *query, "/" );
  }
  strcat( *query, http->header->remote_file );

  /** Arguments */
  if ( http->header->arguments != NULL )
  {
    strcat( *query, "?" );
    strcat( *query, http->header->arguments );
  }
  strcat( *query, " " );

  /** Http version */
  if ( http->header->status.version == NULL )
  {
    http->header->status.version = new_string("HTTP/1.1");
  }
  strcat( *query, http->header->status.version );
  strcat( *query, HTTP_HEADER_NEWLINE );

  /** Host */
  strcat( *query, "Host: " );
  if ( http->header->server == NULL )
  {
    http->header->server = new_string( http->server );
  }
  strcat( *query, http->server );
  strcat( *query, HTTP_HEADER_NEWLINE );

  /** User-Agent */
  strcat( *query, "User-Agent: " );
  if ( http->header->user_agent == NULL )
  {
    if ( http_get_opt( http, HTTP_OPTION_USER_AGENT_MOBILE ) )
    {
      /** Mobile user-agent */
      http->header->user_agent = new_string( HTTP_HEADER_USER_AGENT_MOBILE );
    }
    else
    {
      /** Desktop user-agent */
      http->header->user_agent = new_string( HTTP_HEADER_USER_AGENT_DESKTOP );
    }
  }
  strcat( *query, http->header->user_agent );
  strcat( *query, HTTP_HEADER_NEWLINE );

  /** Accept */
  strcat( *query, "Accept: text/html, application/xml, application/xhtml+xml, */*;q=0.1" );
  strcat( *query, HTTP_HEADER_NEWLINE );

  /** Accept-Language */
  strcat( *query, "Accept-Language: de-DE, de;q=0.9" );
  strcat( *query, HTTP_HEADER_NEWLINE );

  /** Accept-Encoding */
  strcat( *query, "Accept-Encoding: gzip, deflate, identity, *;q=0.1" );
  strcat( *query, HTTP_HEADER_NEWLINE );

  http_sqlite_cookie_server( http->server, &sqlite_server );
  http_sqlite_cookie_path( http->header->remote_file, &sqlite_path );

  /** Cookies */
  if ( !http_get_opt( http, HTTP_OPTION_SQLITE_DB_DISABLED ) )
  {
    sqlite_query = sqlite3_mprintf( HTTP_SQLITE_COOKIE_SELECT_DOMAIN, sqlite_server, sqlite_path );
    free( sqlite_server );
    free( sqlite_path );

    cookies = (char*)malloc( 4*1024 );
    memset( cookies, 0, 4*1024 );
    memcpy( cookies, "Cookie: ", 8 );

    sqlite3_prepare( http->sqlite_handle, sqlite_query, strlen( sqlite_query ), &http->stmt, NULL );
    while ( sqlite3_step( http->stmt ) != SQLITE_DONE )
    {
      if( strstr( http->server, (const char*)(sqlite3_column_text( http->stmt, 0 )+1) ) )
      {
        if ( strlen( cookies ) > 8 )
        {
          strcat( cookies, "; " );
        }
        strcat( cookies, (const char*)sqlite3_column_text( http->stmt, 2 ) ); // Cookie name
        strcat( cookies, "=" );
        strcat( cookies, (const char*)sqlite3_column_text( http->stmt, 3 ) ); // Cookie value
      }
    }
    sqlite3_finalize( http->stmt );
    sqlite3_free( sqlite_query );

    if ( strlen( cookies ) > 8 )
    {
      strcat( *query, cookies );
      strcat( *query, HTTP_HEADER_NEWLINE );
    }
    free( cookies );
  }

  /** Connection */
  strcat( *query, "Connection: " );
  if ( http->header->connection_state == NULL )
  {
    http->header->connection_state = new_string("Keep-Alive");
  }
  strcat( *query, http->header->connection_state );
  strcat( *query, HTTP_HEADER_NEWLINE );

  /** Append user-defined fields */
  http_header_field_iterator_init( &header_field_it, http, &http->header->additional_client_fields );
  while ( http_header_field_iterator_hasNext( &header_field_it ) )
  {
    header_field = http_header_field_iterator_next( &header_field_it );
    if ( !strcmp( http->server, header_field->domain ) )
    {
      switch( header_field->state )
      {
        case HTTP_HEADER_FIELD_VOID:
          break;
        case HTTP_HEADER_FIELD_NON_PERSISTENT:
          header_field->state = HTTP_HEADER_FIELD_VOID;
        case HTTP_HEADER_FIELD_PERSISTENT:
          strcat( *query, header_field->key );
          strcat( *query, ": " );
          strcat( *query, header_field->value );
          strcat( *query, HTTP_HEADER_NEWLINE );
          break;
      }
    }
  }
  http_header_field_iterator_free( &header_field_it );


  if ( http->post_method != HTTP_POST_METHOD_NONE )
  {
    // Do not append additional newline because post data is going to be sent
  }
  else
  {
    /// End of header
    strcat( *query, HTTP_HEADER_NEWLINE );
  }

  free( http->header->method );
  http->header->method = NULL;

  return;
}
void http_log_write( struct HTTP* http, const char* str, unsigned int line_wraps )
{
  FILE* fFile;
  unsigned char i;

  if ( !http_get_opt( http, HTTP_OPTION_LOG_ENABLED ) )
  {
    return;
  }
  fFile = fopen( HTTP_LOG_FILE, "a+" );
  if ( fFile != NULL )
  {
    fprintf( fFile, "%s", str );
    for ( i = 0 ; i < line_wraps ; i++ )
    {
      fprintf( fFile, "\n" );
    }
    fclose( fFile );
  }
}
void http_prepare_header_static( struct HTTP* http, char** query )
{
  *query = NULL;

  if ( http->error.errorId != 0 )
  {
    return;
  }
  if ( http->socket == -1 )
  {
    if ( http->server != NULL )
    {
      http_connect( http, http->server, http->port );
    }
    else
    {
      http->error.errorId = HTTP_ERROR_HOST_MISSING;
      http->error.line = __LINE__;
      http->error.file = __FILE__;
      if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
      {
        printf("Error reusing connection. No host specified\n");
        fflush( stdout );
      }
      return;
    }
    http_set_opt( http, HTTP_OPTION_CONNETION_CLOSE_AFTER_TRANSMISSION, 0 );
  }

  http_prepare_header( http, query );

  if ( *query == NULL )
  {
    if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
    {
      printf("Prepare header to be send...failed\n" );
      fflush( stdout );
    }
    return;
  }

  if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
  {
    printf("Sending... %.4s [ %s ]", *query, http->header->remote_file );
    fflush( stdout );
  }

  if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
  {
    printf("\n" );
    fflush( stdout );
  }
}
void http_read_response( struct HTTP* http )
{
  char* line;
  int size;

  if ( http->error.errorId != 0 )
  {
    return;
  }
  if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
  {
    printf("Wait for response..." );
    fflush( stdout );
  }

  /** First line presents status */
  http_header_recv_line( http, &line, &size );
  if ( line == NULL )
  {
    http->error.errorId = HTTP_ERROR_RECV_ERROR;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
    return;
  }
  if ( *line == '\0' )
  {
    free( line );
    http_header_recv_line( http, &line, &size );
  }
  http_log_write( http, line, 1 );
  if ( size == 0 )
  {
    http->error.errorId = HTTP_ERROR_RECV_ERROR;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
    return;
  }

  http_response_status( http, line, size );
  if ( http->error.errorId != 0 )
  {
    return;
  }

  do
  {
    http_header_recv_line( http, &line, &size );
    if ( line != NULL )
    {
      http_log_write( http, line, 1 );

      // The magic goes here
      http_header_assign_line( http, line, size );
      free( line );
    }
  }
  while ( size != 0 );

  http_log_write( http, "", 1 );

  if ( http->header->connection_state == NULL || !strcmpi( http->header->connection_state, "close" ) )
  {
    http_set_opt( http, HTTP_OPTION_CONNETION_CLOSE_AFTER_TRANSMISSION, 1 );
  }

  if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
  {
    if ( http->error.errorId != 0 )
    {
      printf( "failed\n" );
    }
    else
    {
      printf( "%i %s ", http->header->status.responseId, http->header->status.responseText );

      if ( ( http->header->status.responseId / 100 ) % 10 == 3 )
      {
        printf( "[ location: %s ]", http->header->location );
      }
      else if ( ( http->header->status.responseId / 100 ) % 10 == 2 )
      {
        if ( http->header->transfer_encoding != NULL )
        {
          printf("[ %s, %s ( %s ) ]", http->header->remote_file, http->header->content_type, http->header->transfer_encoding );
        }
        else
        {
          if ( http->header->content_length > 0 )
          {
            printf("[ %s, %s ( %i Bytes ) ]", http->header->remote_file, http->header->content_type, http->header->content_length );
          }
          else
          {
            printf("[ %s, %s ( 0 byte or unknown size ) ]", http->header->remote_file, http->header->content_type );
          }
        }
      }
      printf( "\n" );
    }
    fflush( stdout );
  }
}
void http_response_status( struct HTTP* http, const char* line, const int size )
{
  char* tmp;

  if ( http->error.errorId != 0 )
  {
    return;
  }
  if ( *line == '<' )
  {
    http->error.errorId = HTTP_ERROR_NO_STATUS_CODE_RECIEVED;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
    return;
  }
  if ( size <= HTTP_HEADER_STATUS_VERSION_SIZE+HTTP_HEADER_STATUS_ID_SIZE )
  {
    http->error.errorId = HTTP_ERROR_NO_STATUS_CODE_RECIEVED;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
    return;
  }

  tmp = (char*)malloc( size );

  /** Copy status version (array) */
  memcpy( tmp, line+0, HTTP_HEADER_STATUS_VERSION_SIZE );
  memset( tmp+HTTP_HEADER_STATUS_VERSION_SIZE, 0, 1 );
  http->header->status.version = new_string( tmp );

  /** Copy status code (int) */
  memcpy( tmp, line+HTTP_HEADER_STATUS_VERSION_SIZE+1, HTTP_HEADER_STATUS_ID_SIZE );
  memset( tmp+HTTP_HEADER_STATUS_ID_SIZE, 0, 1 );
  http->header->status.responseId = atoi( tmp );

  /** Copy status text (array) */
  memcpy( tmp, line+HTTP_HEADER_STATUS_VERSION_SIZE+1+HTTP_HEADER_STATUS_ID_SIZE+1, size-(HTTP_HEADER_STATUS_VERSION_SIZE+1)-(HTTP_HEADER_STATUS_ID_SIZE+1) );
  memset( tmp+size-9-4, 0, 1 );
  http->header->status.responseText = new_string( tmp );

  free( tmp );
}
int http_send( struct HTTP* __http, const char *__buf, size_t __n )
{
  return send( __http->socket, __buf, __n, 0 );
}
void http_raw_send( struct HTTP* http, const char* sData, int iLength )
{
  int iBytesSend, iBytesSendCurrent;

  if ( http->error.errorId != 0 )
  {
    return;
  }

  iBytesSend = 0;
  do
  {
    iBytesSendCurrent = http->send_func( http, sData+iBytesSend, iLength-iBytesSend );
    iBytesSend += iBytesSendCurrent;
  }while( iBytesSendCurrent > 0 && iBytesSend < iLength );

  if ( iBytesSendCurrent == 0 )
  {
    http->error.errorId = HTTP_ERROR_RAW_SEND_ERROR;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
  }
  return;
}
int http_recv( struct HTTP* __http, void *__buf, size_t __n )
{
  return recv( __http->socket, __buf, __n, 0 );
}
void http_raw_recv( struct HTTP* http, char** sData, int iBytesToRead, int* iBytesRead )
{
  int iBytesRecvTotal, iBytesRecvCurrent;

  if ( http->error.errorId != 0 )
  {
    return;
  }

  iBytesRecvTotal = iBytesRecvCurrent = 0;
  do
  {
    iBytesRecvCurrent = http->recv_func( http, *sData + iBytesRecvTotal, iBytesToRead - iBytesRecvTotal );
    if ( iBytesRecvCurrent < 0 )
    {
      // Error on transmission
      http->last_result = iBytesRecvCurrent;

      http->error.errorId = HTTP_ERROR_RAW_RECV_ERROR;
      http->error.line = __LINE__;
      http->error.file = __FILE__;
      break;
    }
    iBytesRecvTotal += iBytesRecvCurrent;
  }while( iBytesRecvCurrent > 0 && iBytesRecvTotal < iBytesToRead );
  if ( iBytesRead != NULL )
  {
    *iBytesRead = iBytesRecvTotal;
  }

  if ( iBytesRecvTotal < iBytesToRead )
  {
    memset( *sData + iBytesRecvTotal, 0, iBytesToRead - iBytesRecvTotal );
  }

  if ( iBytesRecvTotal < 0 )
  {
    http->error.errorId = HTTP_ERROR_RAW_RECV_ERROR;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
  }
  return;
}

void __http_send_request_data( struct HTTP* http, const char* header_static )
{
  char* header_remaining_data;
  char* content_type = NULL;
  char* post_data_size_str;

  http_raw_send( http, header_static, strlen( header_static ) );

  if ( http->post_method != HTTP_POST_METHOD_NONE )
  {
    header_remaining_data = (char*)malloc( 100 );
    memset( header_remaining_data, 0, 100 );

    switch( http->post_method )
    {
      case HTTP_POST_METHOD_PLAIN:
        if ( http->post_data_size == 0 )
        {
          http->post_data_size = my_strlen( http->post_data );
        }

        if ( !strncmpi( http->post_data, "<?xml", 5 ) )
        {
          content_type = "text/xml;charset=utf-8";
        }
        /** Add more magic number types here */

        break;
      case HTTP_POST_METHOD_URLENCODED:
        content_type = "application/x-www-form-urlencoded";

        http->post_data_size = http_post_form_urlencoded_get_data( &http->post_data, http );

        if ( http->post_data_size == 0 )
        {
          if ( http->post_data != NULL )
          {
            http->post_data_size = my_strlen( http->post_data );
          }
          else
          {
            http->post_data_size = 0;
            http->post_data = NULL;
          }
        }

        break;
      case HTTP_POST_METHOD_MULTIPART_DATA:
        content_type = "multipart/form-data";

        break;
      default:
        http->error.errorId = HTTP_ERROR_POST_METHOD_UNKNOWN;
        http->error.line = __LINE__;
        http->error.file = __FILE__;
        return;
    }

    strcat( header_remaining_data, "Content-Type: " );
    if ( content_type == NULL )
    {
      strcat( header_remaining_data, "application/octet-stream" );
    }
    else
    {
      strcat( header_remaining_data, content_type );
    }
    strcat( header_remaining_data, HTTP_HEADER_NEWLINE );

    post_data_size_str = my_itoa( http->post_data_size );
    strcat( header_remaining_data, "Content-Length: " );
    strcat( header_remaining_data, post_data_size_str );
    strcat( header_remaining_data, HTTP_HEADER_NEWLINE );
    strcat( header_remaining_data, HTTP_HEADER_NEWLINE );

    free( post_data_size_str );

    switch( http->post_method )
    {
      case HTTP_POST_METHOD_PLAIN:
        // Nothing to be done here
        break;
      case HTTP_POST_METHOD_URLENCODED:
        http_post_form_urlencoded_free( http->post_form_urlencoded_data );
        break;
      case HTTP_POST_METHOD_MULTIPART_DATA:
        // TODO: Free multipart data
        break;
      default:
        http->error.errorId = HTTP_ERROR_POST_METHOD_UNKNOWN;
        http->error.line = __LINE__;
        http->error.file = __FILE__;
        return;
    }

    http_log_write( http, header_static, 0 );
    if ( http_get_opt( http, HTTP_OPTION_LOG_POST_DATA ) )
    {
      http_log_write( http, header_remaining_data, 0 );
      http_log_write( http, http->post_data, 2 );
    }
    else
    {
      http_log_write( http, header_remaining_data, 1 );
    }

    http_raw_send( http, header_remaining_data, strlen( header_remaining_data ) );
    http_raw_send( http, http->post_data, http->post_data_size );

    free( header_remaining_data );

    http->post_method = HTTP_POST_METHOD_NONE;
    if ( http_get_opt( http, HTTP_OPTION_POST_DATA_RELEASE ) )
    {
      free( http->post_data );
      http->post_data = NULL;
      http->post_data_size = 0;
    }
  }
  else
  {
    http_log_write( http, header_static, 1 );
  }

  http_header_init( &http->header, HTTP_HEADER_FREE_WITHOUT_PERSISTENT_DATA );
}

void http_recv_unknown_size( struct HTTP* http, char** content, int* size )
{
  struct HTTP_LIST* list;
  struct HTTP_LIST* currentList;
  char* str;
  int bytesRead = 0;
  int bytesReadTotal;
  int sizeTotal;

  list = (struct HTTP_LIST*)malloc( sizeof( struct HTTP_LIST ) );
  memset( list, 0, sizeof( struct HTTP_LIST ) );

  list->data = (char*)malloc( 1024 );
  memset( list->data, 0, 1024 );

  str = (char*)malloc( 2 );
  memset( str, 0, 2 );

  currentList = list;
  bytesReadTotal = 0;
  do
  {
    //http_header_recv_line( http, &str, &bytesRead );
    http_raw_recv( http, &str, 1, &bytesRead );
    *(currentList->data+bytesReadTotal) = *str;
    bytesReadTotal += bytesRead;
    if ( bytesReadTotal >= 1023 )
    {
      currentList->size = bytesReadTotal;

      currentList->next = (struct HTTP_LIST*)malloc( sizeof( struct HTTP_LIST ) );
      currentList = currentList->next;
      memset( currentList, 0, sizeof( struct HTTP_LIST ) );

      currentList->data = (char*)malloc( 1024 );
      memset( currentList->data, 0, 1024 );

      bytesReadTotal = 0;
    }
  }while( bytesRead > 0 );
  currentList->size = bytesReadTotal;

  currentList = list;
  sizeTotal = 0;
  while( currentList != NULL )
  {
    sizeTotal += currentList->size;
    currentList = currentList->next;
  }

  if ( content != NULL )
  {
    *content = (char*)malloc( sizeTotal+1 );

    currentList = list;
    sizeTotal = 0;
    while( currentList != NULL )
    {
      memcpy( *content+sizeTotal, currentList->data, currentList->size );
      sizeTotal += currentList->size;
      currentList = currentList->next;
    }
    memset( *content+sizeTotal, 0, 1 );

    http_list_free( list );
  }

  if ( bytesRead == -1 )
  {
    http->error.errorId = HTTP_ERROR_SOCKET_ERROR;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
  }
  if ( bytesRead == 0 )
  {
    http->close_func( http );
  }
  return;
}
void http_recv_chunked( struct HTTP* http, char** content, int* size )
{
  char* chunkLine;
  int currentSize;
  int totalSize;
  unsigned int contentPtr;

  struct HTTP_LIST* http_list;
  struct HTTP_LIST* list;

  http_list = (struct HTTP_LIST*)malloc( sizeof( struct HTTP_LIST ) );
  memset( http_list, 0, sizeof( struct HTTP_LIST ) );

  totalSize = 0;

  chunkLine = (char*)malloc( 10 );
  while ( 1 )
  {
    http_header_recv_line( http, &chunkLine, &currentSize );
    currentSize = xtoi( chunkLine );

    if ( currentSize == 0 )
    {
      break;
    }

    list = http_list;
    while( list->next != NULL )
    {
      list = list->next;
    }
    list->data = (char*)malloc( currentSize+1 );

    http_raw_recv( http, &list->data, currentSize, &list->size );
    memset( list->data+list->size, 0, 1 );

    totalSize += list->size;

    list->next = (struct HTTP_LIST*)malloc( sizeof( struct HTTP_LIST ) );
    memset( list->next, 0, sizeof( struct HTTP_LIST ) );

    /** Dummy readline = "" */
    http_header_recv_line( http, &chunkLine, &currentSize );
  }
  free( chunkLine );

  *content = (char*)malloc( totalSize+1 );

  list = http_list;
  contentPtr = 0;
  while( list->next != NULL )
  {
    memcpy( *content+contentPtr, list->data, list->size );
    contentPtr += list->size;
    list = list->next;
  }
  if ( size != NULL )
  {
    *size = totalSize;
  }

  http_list_free( http_list );
  return;
}
void http_list_init( struct HTTP_LIST** list )
{
  *list = (struct HTTP_LIST*)malloc( sizeof( struct HTTP_LIST ) );
  memset( *list, 0, sizeof( struct HTTP_LIST ) );
}
struct HTTP_LIST* http_list_last( struct HTTP_LIST* list )
{
  struct HTTP_LIST* list_it;

  if ( list == NULL )
  {
    return NULL;
  }

  list_it = list;
  while( list_it->next != NULL )
  {
    list_it = list_it->next;
  }
  return list_it;
}
void http_list_free( struct HTTP_LIST* list )
{
  if ( list == NULL )
  {
    return;
  }
  http_list_free( list->next );
  free( list->data );
  free( list->next );
  memset( list, 0, sizeof( struct HTTP_LIST ) );
}
void http_recv_content( struct HTTP* http, char** pContent, int* size )
{
  char* content;
  unsigned char* content_tmp;
  int size_tmp, size_out;
  int size_left_to_recv;
  const int mem_size = 30 * 1024 * 1024;
  struct HTTP_LINK_INFO link;

  if ( pContent != NULL )
  {
    *pContent = NULL;
  }
  if ( size != NULL )
  {
    *size = 0;
  }

  if ( http->error.errorId != 0 )
  {
    return;
  }

  if ( http->header->content_length == 0 && strcmpi( http->header->transfer_encoding, "chunked" ) )
  {
    return;
  }

  http_set_opt( http, HTTP_OPTION_CONTENT_BINARY, 0 );
  if ( http->header->content_type != NULL && !strstr( http->header->content_type, "text" ) )
  {
    /** Content is binary */
    http_set_opt( http, HTTP_OPTION_CONTENT_BINARY, 1 );
  }

  if ( http_get_opt( http, HTTP_OPTION_DOWNLOAD_FILES ) )
  {
    http_link_get_info( http, &link, http->header->remote_file );
    if ( link.file == NULL )
    {
      link.file = new_string( "index.html" );
    }

    if ( size != NULL )
    {
      *size = http_save_data_to_file( http, link.file );
    }
    else
    {
      http_save_data_to_file( http, link.file );
    }
    http_link_info_free( &link );
    return;
  }

  if ( pContent == NULL )
  {
    /** Dummy read for clearing socket */
    if ( http->header->content_length != 0 )
    {
      content = (char*)malloc( mem_size );

      size_left_to_recv = http->header->content_length;
      do
      {
        if ( size_left_to_recv >= mem_size )
        {
          http_raw_recv( http, &content, mem_size, &size_tmp );
          size_left_to_recv -= size_tmp;
        }
        else
        {
          http_raw_recv( http, &content, size_left_to_recv, &size_tmp );
          size_left_to_recv -= size_tmp;
        }
      }while( size_tmp > 0 && size_left_to_recv > 0 );
      free( content );
    }

    if ( size != NULL )
    {
      *size = http->header->content_length;
    }
    return;
  }

  if ( http->header->transfer_encoding != NULL &&
        !strcmpi( http->header->transfer_encoding, "chunked" ) )
  {
    http_recv_chunked( http, &content, &size_tmp );
    http_header_recv_line( http, NULL, NULL );
  }
  else
  {
    if ( http->header->content_length != 0 && http->header->transfer_encoding == NULL )
    {
      if ( http->header->content_length > 100 * ( 1024 * 1024 ) )
      {
        http->error.errorId = HTTP_ERROR_DOWNLOAD_FILE_TOO_BIG;
        http->error.line = __LINE__;
        http->error.file = __FILE__;

        if ( pContent != NULL )
        {
          *pContent = NULL;
        }
        if ( size != NULL )
        {
          *size = 0;
        }

        return;
      }

      content = (char*)malloc( http->header->content_length+1 );
      http_raw_recv( http, &content, http->header->content_length, &size_tmp );
      memset( content+http->header->content_length, 0, 1 );
    }
    else
    {
      http_recv_unknown_size( http, &content, &size_tmp );
    }
  }

  /** If content is encoded, decode it */
  if ( http->header->content_encoding != NULL )
  {
    if ( inflateData( &content_tmp, &size_out, (unsigned char*)content, size_tmp ) == HTTP_BOOL_TRUE )
    {
      free( content );
      content = (char*)content_tmp;
    }
  }

  if ( http_get_opt( http, HTTP_OPTION_CONNETION_CLOSE_AFTER_TRANSMISSION ) )
  {
    http->close_func( http );
  }

  if ( http_get_opt( http, HTTP_OPTION_LOG_ENABLED ) && http_get_opt( http, HTTP_OPTION_LOG_RESPONSE ) )
  {
    http_log_write( http, content, 2 );
  }

  *pContent = content;
  if ( size != NULL )
  {
    *size = size_tmp;
  }
}
void http_reconnect( struct HTTP* http, const char* sNewHost, const unsigned short port )
{
  if ( http->error.errorId != 0 )
  {
    return;
  }

  if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
  {
    printf("\n");
    fflush( stdout );
  }

  if ( sNewHost == NULL )
  {
    http->error.errorId = HTTP_ERROR_HOST_MISSING;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
    printf("Error could not connect to an empty host" );
    fflush( stdout );
    return;
  }

  if ( http->socket < 0 )
  {
    if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
    {
      printf("Not connected to any host. Connect to %s...\n", sNewHost );
    }
    http_connect( http, sNewHost, port );
    if ( http_get_opt( http, HTTP_OPTION_HOST_CHANGED ) )
    {
      http_set_opt( http, HTTP_OPTION_HOST_CHANGED, 0 );
      http_set_opt( http, HTTP_OPTION_POTOCOL_CHANGED, 0 );
    }
    return;
  }

  if ( http->server != NULL )
  {
    if ( strcmpi( http->server, sNewHost ) ||
          http_get_opt( http, HTTP_OPTION_HOST_CHANGED ) == HTTP_BOOL_TRUE ||
          http_get_opt( http, HTTP_OPTION_POTOCOL_CHANGED ) == HTTP_BOOL_TRUE)
    {
      if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
      {
        printf("Host has changed to %s. Connect...\n", sNewHost );
      }
      http->close_func( http );
      http_connect( http, sNewHost, port );

      http_set_opt( http, HTTP_OPTION_HOST_CHANGED, 0 );
      http_set_opt( http, HTTP_OPTION_POTOCOL_CHANGED, 0 );
    }
    else
    {
      if ( http->header->connection_state != NULL && !strcmpi( http->header->connection_state, "keep-alive" ) )
      {
        if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
        {
          printf("Host not changed and connection keeped alive. Reuse connection...\n" );
        }
      }
      else
      {
        if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
        {
          printf("Connection not keeped alive. Reconnect...\n" );
        }
        http->close_func( http );
        http_connect( http, sNewHost, port );
      }
    }
  }
  else
  {
    if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
    {
      printf("No old server set. Connect...\n" );
    }
    http->close_func( http );
    http_connect( http, sNewHost, port );
  }
  fflush( stdout );
}
void http_handle_response( struct HTTP* http )
{
  unsigned char statusCode[3];
  char* authPassword;

  if ( http->error.errorId != 0 )
  {
    return;
  }

  if ( http->header->connection_state == NULL || !strcmpi( http->header->connection_state, "close") )
  {
    http_set_opt( http, HTTP_OPTION_CONNETION_CLOSE_AFTER_TRANSMISSION, 1 );
  }

  statusCode[0] = (http->header->status.responseId / 100) % 10;
  statusCode[1] = (http->header->status.responseId / 10) % 10;
  statusCode[2] = http->header->status.responseId % 10;
  switch( statusCode[0] )
  {
    case 1:
      /** Information (=100 Continue) */
      break;
    case 2:
      /** OK (=200 OK) */
      break;
    case 3:
      /** Redirection (=301 Moved Permanently or 302 Found) */
      if ( statusCode[1] == 0 )
      {
        switch( statusCode[2] )
        {
          case 1: /** 301 Moved Permanently */
            http_sqlite_moved_add( http );
          case 2: /** 302 Found */
          case 3: /** 303 See Other */
            free( http->post_data );
            http->post_data = NULL;
            http->post_data_size = 0;
          case 7: /** 307 Temporary Redirect ( Post -> Post ) */
            http_recv_content( http, NULL, NULL );

            // Set new query
            free( http->header->originalQuery );
            http->header->originalQuery = new_string( http->header->location );

            http_header_follow( http );
            break;
          default:
            http->error.errorId = http->header->status.responseId;
            http->error.line = __LINE__;
            http->error.file = __FILE__;
            break;
        }
      }
      break;
    case 4:
      /** Client-Error (=400 Bad Request or 401 Unauthorized or 404 Not Found) */
      if ( statusCode[1] == 0 )
      {
        switch( statusCode[2] )
        {
          case 1: /** 401 Unauthorized */
            http_recv_content( http, NULL, NULL );

            http_get_authorization_password( http, &authPassword );
            if ( authPassword != NULL )
            {
              http_header_add_client_field( http->header, "Authorization", authPassword, http->server, HTTP_HEADER_FIELD_PERSISTENT );
              http_set_opt( http, HTTP_STATUS_REQUERY, 1 );
            }
            break;
          default:
            http->error.errorId = http->header->status.responseId;
            http->error.line = __LINE__;
            http->error.file = __FILE__;
            break;
        }
      }
      return;
    case 5:
      /** Server-Error (=500 Internal Server Error) */
      http->error.errorId = http->header->status.responseId;
      http->error.line = __LINE__;
      http->error.file = __FILE__;
      return;
    default:
      /** Not been used */
      break;
  }
}
void http_get_authorization_password( struct HTTP* http, char** str )
{
  char* value;
  char* username;
  char* password;
  char* base64_decoded_string;
  char* tmpStr;
  int count;
  char c;

  *str = NULL;
  if ( !strncmpi( http->header->www_authenticate, "Basic", 5 ) )
  {
    html_tag_get_parameter_field( http->header->www_authenticate, "realm", &value, NULL );
    printf( "--- Note: To cancel login process leave username and password empty ---\n" );
    printf( "Login required ( \"%s\" ):\n", value );

    username = (char*)malloc( 70 );
    printf( "Username: " );
    fflush( stdout );
    username = fgets( username, 70, stdin );
    username[strlen(username)-1] = '\0';

    if ( *username == HTTP_KEY_ESC )
    {
      free( username );
      return;
    }

    password = (char*)malloc( 70 );
    printf( "Password: " );
    fflush( stdout );

    count = 0;
    while( count < 70 )
    {
      c = getch();
      if ( c == '\n' )
      {
        break;
      }
      if ( c == HTTP_KEY_BACKSPACE )
      {
        count--;
        *(password+count) = '\0';
        continue;
      }
      if ( c == HTTP_KEY_ESC )
      {
        printf("\n");
        fflush( stdout );
        free( password );
        return;
      }

      printf("x");
      fflush( stdout );

      *(password+count) = c;
      count++;
    }
    *(password+count) = '\0';

    printf("\n");
    fflush( stdout );

    if ( *username == '\0' && *password == '\0' )
    {
      free( username );
      free( password );
      return;
    }

    tmpStr = (char*)malloc( 140 );
    sprintf( tmpStr, "%s:%s", username, password );
    free( username );
    free( password );

    base64_decoded_string = base64_string_encode( tmpStr, 0 );
    free( tmpStr );

    *str = (char*)malloc( 6 + strlen( base64_decoded_string ) + 1 );
    strcpy( *str, "Basic " );
    strcat( *str, base64_decoded_string );
    free( base64_decoded_string );
  }
  if ( !strncmpi( http->header->www_authenticate, "Digest", 6 ) )
  {
    http->error.errorId = HTTP_ERROR_NOT_IMPLEMENTED_YET;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
  }
  return;
}
void http_parse_link( struct HTTP* http )
{
  struct HTTP_LINK_INFO link_info;
  char* remote_file;

  http_link_get_info( http, &link_info, http->header->originalQuery );

  remote_file = (char*)malloc( link_info.size );
  memset( remote_file, 0, link_info.size );

  if ( link_info.subdir != NULL )
  {
    strcat( remote_file, link_info.subdir );
  }
  if ( link_info.file != NULL )
  {
    strcat( remote_file, link_info.file );
  }
  if ( link_info.parameters != NULL )
  {
    if ( link_info.subdir == NULL && link_info.file == NULL )
    {
      strcat( remote_file, "/" );
    }
    strcat( remote_file, "?" );
    strcat( remote_file, link_info.parameters );
  }

  if ( !strcmpi( link_info.protocol, "https" ) )
  {
    http->connect_func = &http_ssl_connect;
    http->recv_func = &http_ssl_recv;
    http->send_func = &http_ssl_send;
    http->close_func = &http_ssl_close;

    if ( http_get_opt( http, HTTP_OPTION_SSL_ENABLED ) == HTTP_BOOL_FALSE )
    {
      http_set_opt( http, HTTP_OPTION_POTOCOL_CHANGED, 1 );
      http_set_opt( http, HTTP_OPTION_SSL_ENABLED, 1 );
    }
    http->port = HTTP_SSL_PORT;
  }

  if ( !strcmpi( link_info.protocol, "http" ) )
  {
    http->connect_func = &http_raw_connect;
    http->recv_func = &http_recv;
    http->send_func = &http_send;
    http->close_func = &http_close;

    if ( http_get_opt( http, HTTP_OPTION_SSL_ENABLED ) == HTTP_BOOL_TRUE )
    {
      http_set_opt( http, HTTP_OPTION_POTOCOL_CHANGED, 1 );
      http_set_opt( http, HTTP_OPTION_SSL_ENABLED, 0 );
    }
    if ( link_info.port != NULL )
    {
      http->port = atoi( link_info.port );
    }
    else
    {
      http->port = HTTP_PORT;
    }
  }

  if ( *remote_file == '\0' )
  {
    free( remote_file );
    remote_file = NULL;
  }

  if ( strcmpi( http->server, link_info.host ) )
  {
    http_set_opt( http, HTTP_OPTION_HOST_CHANGED, 1 );
    free( http->server );
    http->server = new_string( link_info.host );
  }
  free( http->header->remote_file );

  http_link_info_free( &link_info );
  http->header->remote_file = remote_file;
}
void http_get_page( struct HTTP* http, const char* link, char** content, int* size )
{
  char* header;

  if ( link == NULL || *link == '\0' )
  {
    http->error.errorId = HTTP_ERROR_HOST_MISSING;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
    return;
  }

  if ( http_get_opt( http, HTTP_OPTION_LOG_ENABLED ) == 1 && http_get_opt( http, HTTP_OPTION_LOG_OVERWRITE ) == 1 )
  {
    remove( HTTP_LOG_FILE );
  }

  free( http->header->originalQuery );
  http->header->originalQuery = new_string( link );

  http_sqlite_moved_check( http );
  http_parse_link( http );
  http_reconnect( http, http->server, http->port );

  // Data is sent here
  http_prepare_header_static( http, &header );
  __http_send_request_data( http, header );
  free( header );

  http_read_response( http );
  http_handle_response( http );

  http_recv_content( http, content, size );

  if ( http->error.errorId == HTTP_ERROR_RECV_ERROR ||
       http->error.errorId == HTTP_ERROR_RAW_SEND_ERROR ||
       http->error.errorId == HTTP_ERROR_RAW_RECV_ERROR  )
  {
    /** Try to reconnect and query again */
    if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
    {
      printf("\nConnection closed during transmission, send query again...\n");
      fflush( stdout );
    }

    http->error.errorId = 0;
    http->close_func( http );
    http_alloc( http, HTTP_FREE_WITHOUT_PERSISTENT_DATA | HTTP_HEADER_FREE_WITHOUT_PERSISTENT_DATA );

    http_reconnect( http, http->server, http->port );

    // Data is sent here
    http_prepare_header_static( http, &header );
    __http_send_request_data( http, header );
    free( header );

    http_read_response( http );
    http_handle_response( http );
    http_recv_content( http, content, size );
  }
  http_alloc( http, HTTP_FREE_WITHOUT_PERSISTENT_DATA | HTTP_HEADER_FREE_WITHOUT_PERSISTENT_DATA );

  if ( http_get_opt( http, HTTP_STATUS_REQUERY ) )
  {
    http_set_opt( http, HTTP_STATUS_REQUERY, 0 );
    http_get_page( http, link, content, size );
  }

  if ( http->error.errorId != 0 )
  {
    http_print_errorcode( http );
  }
  return;
}

int http_save_data_to_file( struct HTTP* http, const char* file )
{
  const int mem_size = 30 * 1024 * 1024;

  char* content, *folder_file;
  int size_left_to_recv, size_tmp = 0;
  FILE* fFile;

  if ( http->error.errorId != 0 )
  {
    return 0;
  }

  if ( http_get_opt( http, HTTP_OPTION_DOWNLOAD_FOLDER ) )
  {
    folder_file = (char*)malloc( 255 );
    snprintf( folder_file, 255, "%s%s", http->download_folder, file );

    fFile = fopen( folder_file, "wb" );
  }
  else
  {
    fFile = fopen( file, "wb" );
  }

  if ( fFile == NULL )
  {
    http->error.errorId = HTTP_ERROR_DOWNLOAD_SAVE_FILE;
    http->error.line = __LINE__;
    http->error.file = __FILE__;
    return 0;
  }

  content = (char*)malloc( mem_size );

  size_left_to_recv = http->header->content_length;
  do
  {
    if ( size_left_to_recv >= mem_size )
    {
      http_raw_recv( http, &content, mem_size, &size_tmp );
      size_left_to_recv -= size_tmp;
    }
    else
    {
      http_raw_recv( http, &content, size_left_to_recv, &size_tmp );
      size_left_to_recv -= size_tmp;
    }
    fwrite( content, size_tmp, 1, fFile );
  }while( size_tmp > 0 && size_left_to_recv > 0 );

  fflush( fFile );
  fclose( fFile );
  free( content );

  return http->header->content_length;
}
void http_close( struct HTTP* http )
{
  if ( http->error.errorId == 0 )
  {
    close( http->socket );
    http->socket = HTTP_ERROR_SOCKET_ERROR;
  }
}
void http_write_memory_dump( struct HTTP* http, FILE* fFile )
{
  //struct HTTP_COOKIE_ITERATOR cookie_it;
  //struct HTTP_COOKIE* cookie;

  struct HTTP_HEADER_FIELD_ITERATOR field_it;
  struct HTTP_HEADER_FIELD* field;

  fprintf( fFile, "HTTP-Wrapper version %s memory dump\n\n", HTTP_VERSION );

  fprintf( fFile, "File-location: %s\n\n", __FILE__ );
  fprintf( fFile, "-------HTTP-Section-------\n" );
  fprintf( fFile, "http adress: %p\n", http );
  fprintf( fFile, "socket: %i\n", http->socket );
  fprintf( fFile, "lastResult: %i\n", http->last_result );
  fprintf( fFile, "lastError: %i\n", http->error.errorId );
  fprintf( fFile, "port: %i\n", http->port );
  fprintf( fFile, "server: %s\n", http->server );
  fprintf( fFile, "download_folder: %s\n", http->download_folder );
  fprintf( fFile, "firedns address: %p\n", &http->firedns );
  fprintf( fFile, "addr: %p\n", &http->addr );

  fprintf( fFile, "\n-------HTTP-Header-Section-------\n" );
  fprintf( fFile, "header adress: %p\n", http->header );
  if ( http->header )
  {
    fprintf( fFile, "status-id: %i\n", http->header->status.responseId );
    fprintf( fFile, "status-text: %s\n", http->header->status.responseText );
    fprintf( fFile, "status-verion: %s\n", http->header->status.version );

    fprintf( fFile, "method: %s\n", http->header->method );
    fprintf( fFile, "remote_file: %s\n", http->header->remote_file );
    fprintf( fFile, "arguments: %s\n", http->header->arguments );
    fprintf( fFile, "server: %s\n", http->header->server );
    fprintf( fFile, "connection_state: %s\n", http->header->connection_state );
    fprintf( fFile, "content_length: %i\n", http->header->content_length );
    fprintf( fFile, "content_type: %s\n", http->header->content_type );
    fprintf( fFile, "transfer_encoding: %s\n", http->header->transfer_encoding );
    fprintf( fFile, "user_agent: %s\n", http->header->user_agent );
    fprintf( fFile, "www_authenticate: %s\n", http->header->www_authenticate );
    fprintf( fFile, "post_data: %s\n", http->post_data );
    fprintf( fFile, "cookies: %p\n", http->header->cookies );

    /*http_header_cookie_iterator_init( &cookie_it, http );
    while( http_header_cookie_iterator_hasNext( &cookie_it ) )
    {
      cookie = http_header_cookie_iterator_next( &cookie_it );
      fprintf( fFile, "  %s=%s, %s, %s, %s\n", cookie->name, cookie->value, cookie->domain, cookie->path, cookie->expires );
    }
    http_header_cookie_iterator_free( &cookie_it );
    */

    fprintf( fFile, "additional_client_fields: %p\n", &http->header->additional_client_fields );
    http_header_field_iterator_init( &field_it, http, &http->header->additional_client_fields );
    while( http_header_field_iterator_hasNext( &field_it ) )
    {
      field = http_header_field_iterator_next( &field_it );
      fprintf( fFile, "  %s=%s, state: %i\n", field->key, field->value, field->state );
    }
    http_header_field_iterator_free( &field_it );

    fprintf( fFile, "additional_server_fields: %p\n", &http->header->additional_server_fields );
    http_header_field_iterator_init( &field_it, http, &http->header->additional_server_fields );
    while( http_header_field_iterator_hasNext( &field_it ) )
    {
      field = http_header_field_iterator_next( &field_it );
      fprintf( fFile, "  %s=%s, state: %i\n", field->key, field->value, field->state );
    }
    http_header_field_iterator_free( &field_it );
  }

}

