#include "http_header.h"

void http_header_field_iterator_init( struct HTTP_HEADER_FIELD_ITERATOR* it, struct HTTP* http, void* current )
{
	it->http = http;
	//it->current = &http->header->additional_server_fields;
	it->current = (struct HTTP_HEADER_FIELD*)current;
}
HTTP_BOOL http_header_field_iterator_hasNext( struct HTTP_HEADER_FIELD_ITERATOR* it )
{
	if ( it->current->key != NULL )
	{
		return HTTP_BOOL_TRUE;
	}
	return HTTP_BOOL_FALSE;
}
struct HTTP_HEADER_FIELD* http_header_field_iterator_next( struct HTTP_HEADER_FIELD_ITERATOR* it )
{
	struct HTTP_HEADER_FIELD* field_tmp;

	field_tmp = it->current;
	it->current = it->current->next;
	return field_tmp;
}
void http_header_field_iterator_free( struct HTTP_HEADER_FIELD_ITERATOR* it )
{
	memset( it, 0, sizeof( struct HTTP_HEADER_FIELD_ITERATOR ) );
}
void http_header_remove_client_field(struct HTTP_HEADER* header, const char* key )
{
	struct HTTP_HEADER_FIELD* field_prev;
	struct HTTP_HEADER_FIELD* field;

	field_prev = &header->additional_client_fields;
	field = &header->additional_client_fields;
	while( field->next != NULL )
	{
		if ( !strcmp( field->key, key ) )
		{
			break;
		}
		field_prev = field;
		field = field->next;
	}
	if ( field->next != NULL )
	{
		// Field found
		field_prev->next = field->next;

		field->next = NULL;
		http_header_fields_free( field );
	}
	return;
}
void http_header_add_client_field( struct HTTP_HEADER* header, const char* key, const char* value, const char* domain, unsigned char state )
{
	struct HTTP_HEADER_FIELD* field;

	field = &header->additional_client_fields;
	while( field->next != NULL )
	{
		if ( !strcmp( field->key, key ) )
		{
			break;
		}
		field = field->next;
	}
	if ( field->next != NULL )
	{
		// Field with same key found, update values
		free( field->value );
		field->value = new_string( value );
	}
	else
	{
		field->key	 = new_string( key );
		field->value = new_string( value );
		field->domain = new_string( domain );
		field->state = state;
		field->next	 = (struct HTTP_HEADER_FIELD*)malloc( sizeof( struct HTTP_HEADER_FIELD ) );
		memset( field->next, 0, sizeof( struct HTTP_HEADER_FIELD ) );
	}
}
void http_header_add_server_field( struct HTTP_HEADER* header, const char* key, const char* value, unsigned char state )
{
	struct HTTP_HEADER_FIELD* field;

	field = &header->additional_server_fields;
	while( field->next != NULL )
	{
		field = field->next;
	}
	field->key	 = new_string( key );
	field->value = new_string( value );
	field->state = state;
	field->next	 = (struct HTTP_HEADER_FIELD*)malloc( sizeof( struct HTTP_HEADER_FIELD ) );
	memset( field->next, 0, sizeof( struct HTTP_HEADER_FIELD ) );
}

void http_header_init( struct HTTP_HEADER** header, HTTP_HEX reset )
{
	char* remote_file;
	char* connection_state;
	char* originalQuery;
	struct HTTP_COOKIE* cookies;
	struct HTTP_HEADER_FIELD clientFields;

	if ( reset & HTTP_HEADER_RESET )
	{
		cookies = (*header)->cookies;
		memcpy( &clientFields, &(*header)->additional_client_fields, sizeof( struct HTTP_HEADER_FIELD ) );

		memset( *header, 0, sizeof( struct HTTP_HEADER ) );

		(*header)->cookies = cookies;
		memcpy( &(*header)->additional_client_fields, &clientFields, sizeof( struct HTTP_HEADER_FIELD ) );

		http_header_cookie_free( (*header)->cookies );
		memset( &(*header)->additional_server_fields, 0, sizeof( struct HTTP_HEADER_FIELD ) );
		return;
	}
	if ( reset & HTTP_HEADER_INIT )
	{
		*header = (struct HTTP_HEADER*)malloc( sizeof( struct HTTP_HEADER ) );
		(*header)->cookies = (struct HTTP_COOKIE*)malloc( sizeof( struct HTTP_COOKIE ) );
		memset( (*header)->cookies, 0, sizeof( struct HTTP_COOKIE ) );

		memset( &(*header)->additional_client_fields, 0, sizeof( struct HTTP_HEADER_FIELD ) );

		http_header_init( header, HTTP_HEADER_RESET );

		/*(*header)->cookies = cookies; */
	}
	if ( reset & HTTP_HEADER_FREE )
	{
		free( (*header)->method );
		free( (*header)->status.version );
		free( (*header)->status.responseText );
		free( (*header)->server );
		free( (*header)->remote_file );
		free( (*header)->arguments );
		free( (*header)->connection_state );
		free( (*header)->transfer_encoding );
		free( (*header)->content_type );
		free( (*header)->location );

		http_header_cookie_free( (*header)->cookies );
		http_header_fields_free( &(*header)->additional_client_fields );
		http_header_fields_free( &(*header)->additional_server_fields );
		http_header_init( header, HTTP_HEADER_RESET );
	}
	if ( reset & HTTP_HEADER_FREE_WITHOUT_PERSISTENT_DATA )
	{
		free( (*header)->method );
		free( (*header)->status.version );
		free( (*header)->status.responseText );
		free( (*header)->server );
		free( (*header)->arguments );
		free( (*header)->transfer_encoding );
		free( (*header)->content_type );
		free( (*header)->location );

		remote_file = (*header)->remote_file;
		connection_state = (*header)->connection_state;
		cookies = (*header)->cookies;
		originalQuery = (*header)->originalQuery;

		(*header)->remote_file = NULL;
		(*header)->connection_state = NULL;
		(*header)->cookies = NULL;
		(*header)->originalQuery = NULL;

		http_header_fields_free( &(*header)->additional_server_fields );
		http_header_init( header, HTTP_HEADER_RESET );

		(*header)->remote_file = remote_file;
		(*header)->connection_state = connection_state;
		(*header)->cookies = cookies;
		(*header)->originalQuery = originalQuery;
	}
	return;
}
void http_header_fields_free( struct HTTP_HEADER_FIELD* field )
{
	if ( field == NULL )
	{
		return;
	}
	http_header_fields_free( field->next );
	free( field->key );
	free( field->value );
	free( field->next );
	memset( field, 0, sizeof( struct HTTP_HEADER_FIELD ) );
}
void http_header_assign_line( struct HTTP* http, const char* line, int size )
{
	int i;
	char* key;
	char* value;

	if ( http->error.errorId != 0 )
	{
		return;
	}
	if ( *line == '\0' )
	{
		return;
	}

	for ( i = 0 ; i < size && *(line+i) != ':' ; i++ );
	key = (char*)malloc( i+1 );
	memcpy( key, line, i );
	memset( key+i, 0, 1 );

	if ( size-i-2+1 <= 0 || i+2 > size )
	{
		free( key );
		return;
	}
	value = (char*)malloc( size-i-2+1 );
	memcpy( value, line+i+2, size-i-2 );
	memset( value+size-i-2, 0, 1 );

	if ( !strcmpi( key, "Server" ) )
	{
		http->header->server = new_string( value );
	}
	if ( !strncmpi( line, "Set-Cookie", 10 ) )
	{
		http_header_cookie_add( http, line );
	}
	if ( !strcmpi( key, "Connection" ) )
	{
		http->header->connection_state = new_string( value );
	}
	if ( !strcmpi( key, "Content-Length" ) )
	{
		http->header->content_length = atoi( value );
	}
	if ( !strcmpi( key, "Content-Type" ) )
	{
		http->header->content_type = new_string( value );
	}
	if ( !strcmpi( key, "Content-Encoding" ) )
	{
		http->header->content_encoding = new_string( value );
	}
	if ( !strcmpi( key, "Transfer-Encoding" ) )
	{
		http->header->transfer_encoding = new_string( value );
	}
	if ( !strcmpi( key, "Location" ) )
	{
		http->header->location = new_string( value );
	}
	if ( !strcmpi( key, "WWW-Authenticate" ) )
	{
		http->header->www_authenticate = new_string( value );
	}

	http_header_add_server_field( http->header, key, value, HTTP_HEADER_FIELD_VOID );
	free( key );
	free( value );
}
void http_header_recv_line( struct HTTP* http, char** line, int* size )
{
	char* tmp;
	char* lineTmp;
	int iBytesRead;
	int readStatus;

	if ( http->error.errorId != 0 )
	{
		return;
	}

	tmp = (char*)malloc( 2 );
	lineTmp = (char*)malloc( HTTP_HEADER_MAX_FIELD_SIZE );
	iBytesRead = 0;
	while ( iBytesRead < HTTP_HEADER_MAX_FIELD_SIZE )
	{
		http_raw_recv( http, &tmp, 1, &readStatus );
		if ( readStatus <= 0 )
		{
			break;
		}
		if ( *tmp == '\r' )
		{
			// Line end
			http_raw_recv( http, &tmp, 1, &readStatus );
			if ( *tmp != '\n' )
			{
				http->error.errorId = HTTP_ERROR_EXPECT_LINEFEED;
				http->error.line = __LINE__;
				http->error.file = __FILE__;
				return;
			}
			break;
		}
		memcpy( lineTmp+iBytesRead, tmp, 1 );
		iBytesRead += 1;
	}
	free( tmp );

	if ( iBytesRead == 0 )
	{
		if ( line != NULL )
		{
			*line = NULL;
		}
		if ( size != NULL )
		{
			*size = 0;
		}
		return;
	}
	if ( line != NULL )
	{
		*line = (char*)malloc( iBytesRead+1 );
		memcpy( *line, lineTmp, iBytesRead );
		memset( *line+iBytesRead, 0, 1 );

		free( lineTmp );
	}
	else
	{
		free( lineTmp );
	}

	if ( size != NULL )
	{
		*size = iBytesRead;
	}
}
void http_header_follow( struct HTTP* http )
{
	char* header;

	if ( http->error.errorId != 0 )
	{
		return;
	}
	if ( http->header->location == NULL )
	{
		return;
	}

	http_sqlite_moved_check( http );
	http_alloc( http, HTTP_FREE_WITHOUT_PERSISTENT_DATA | HTTP_HEADER_FREE_WITHOUT_PERSISTENT_DATA );

	http_parse_link( http );
	http_reconnect( http, http->server, http->port );

	http_prepare_header_static( http, &header );
	__http_send_request_data( http, header );
	free( header );

	http_read_response( http );

	http_handle_response( http );
	return;
}


