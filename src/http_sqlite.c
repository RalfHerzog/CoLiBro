#include "http_sqlite.h"

void http_sqlite_db_create( struct HTTP* http )
{
	int retval, i;
	char* db_tables[] = { HTTP_SQLITE_DB_TABLE_COOKIES, HTTP_SQLITE_DB_TABLE_BM, HTTP_SQLITE_DB_TABLE_DNS, HTTP_SQLITE_DB_TABLE_REDIRECT };

	if ( http_get_opt( http, HTTP_OPTION_SQLITE_DB_DISABLED ) )
	{
		return;
	}

	for ( i = 0 ; i < ( sizeof( db_tables ) / sizeof( char* ) ) ; i++ )
	{
		retval = sqlite3_exec( http->sqlite_handle, db_tables[i], 0, 0, 0 );
		if ( retval != SQLITE_OK )
		{
			break;
		}
	}
}
void http_sqlite_db_startup( struct HTTP* http )
{
	char* sqlite_query;

	if ( http_get_opt( http, HTTP_OPTION_SQLITE_DB_DISABLED ) )
	{
		return;
	}

	/** Remove cookies not valid anymore */
	sqlite_query = sqlite3_mprintf( HTTP_SQLITE_COOKIE_SELECT_ALL_EXPIRED );
	sqlite3_prepare( http->sqlite_handle, sqlite_query, -1, &http->stmt, NULL );
	sqlite3_free( sqlite_query );

	while ( sqlite3_step( http->stmt ) != SQLITE_DONE )
	{
		// Cookie found here is expired or not valid any more
		sqlite_query = sqlite3_mprintf( HTTP_SQLITE_COOKIE_DELETE, sqlite3_column_text( http->stmt, 0 ),
											sqlite3_column_text( http->stmt, 1 ), sqlite3_column_text( http->stmt, 2 ) );
		sqlite3_exec( http->sqlite_handle, sqlite_query, 0, 0, 0 );
		sqlite3_free( sqlite_query );
	}
	sqlite3_finalize( http->stmt );
	/** End of cookie operation */
}
int http_sqlite_num_rows( struct HTTP* http, const char* query )
{
	int count = 0;
	int sqlite3_result = SQLITE_OK;

	if ( http_get_opt( http, HTTP_OPTION_SQLITE_DB_DISABLED ) )
	{
		return 0;
	}

	sqlite3_result = sqlite3_prepare( http->sqlite_handle, query, -1, &http->stmt, NULL );
	if ( sqlite3_result != SQLITE_OK )
	{
		return 0;
	}

	while ( sqlite3_step( http->stmt ) != SQLITE_DONE )
	{
		count++;
	}
	sqlite3_finalize( http->stmt );
	return count;
}
void http_sqlite_cookie_update( struct HTTP* http, struct HTTP_COOKIE* cookie )
{
	char* sqlite_datetime;
	char* sqlite_query;
	int retval;

	sqlite_datetime = NULL;
	if ( http_header_cookie_validate_expires( cookie->expires ) == HTTP_REG_NOERROR )
	{
		http_header_cookie_sqlite_expires( cookie->expires, &sqlite_datetime );
	}

	if ( http_get_opt( http, HTTP_OPTION_SQLITE_DB_DISABLED ) )
	{
		return;
	}

	// "Select" to check if update or insert
	sqlite_query = sqlite3_mprintf( HTTP_SQLITE_COOKIE_SELECT, cookie->domain, cookie->path, cookie->name );
	retval = http_sqlite_num_rows( http, sqlite_query );
	sqlite3_free( sqlite_query );

	if ( retval > 0 )
	{
		// Update cookie
		sqlite_query = sqlite3_mprintf( HTTP_SQLITE_COOKIE_UPDATE, cookie->value, cookie->domain, cookie->path, cookie->name );
		retval = sqlite3_exec( http->sqlite_handle, sqlite_query, 0, 0, 0 );
		sqlite3_free( sqlite_query );
	}
	else
	{
		// Insert cookie
		sqlite_query = sqlite3_mprintf( HTTP_SQLITE_COOKIE_INSERT, cookie->domain, cookie->path, cookie->name, cookie->value, sqlite_datetime, SAVE_CHAR_RET(cookie->secure), SAVE_CHAR_RET(cookie->http_only) );
		free( sqlite_datetime );

		retval = sqlite3_exec( http->sqlite_handle, sqlite_query, 0, 0, 0 );
		sqlite3_free( sqlite_query );
	}
}
void http_sqlite_moved_add( struct HTTP* http )
{
	char* sqlite_query;
	int retval;

	if ( http_get_opt( http, HTTP_OPTION_SQLITE_DB_DISABLED ) )
	{
		if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
		{
			printf( "\nDISABLED: Add '%s' with link to '%s' to database\n", http->header->originalQuery, http->header->location );
			fflush( stdout );
		}
		return;
	}

	if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
	{
		printf( "\nAdd '%s' with link to '%s' to database...", http->header->originalQuery, http->header->location );
		fflush( stdout );
	}

	// "Select" to check if update or insert
	sqlite_query = sqlite3_mprintf( HTTP_SQLITE_REDIRECT_SELECT, http->header->originalQuery );
	retval = http_sqlite_num_rows( http, sqlite_query );
	sqlite3_free( sqlite_query );

	if ( retval > 0 )
	{
		// Update url
		sqlite_query = sqlite3_mprintf( HTTP_SQLITE_REDIRECT_UPDATE,
											http->header->location, http->header->originalQuery );
		retval = sqlite3_exec( http->sqlite_handle, sqlite_query, 0, 0, 0 );
		sqlite3_free( sqlite_query );
	}
	else
	{
		// Insert url
		sqlite_query = sqlite3_mprintf( HTTP_SQLITE_REDIRECT_INSERT,
											http->header->originalQuery, http->header->location );

		retval = sqlite3_exec( http->sqlite_handle, sqlite_query, 0, 0, 0 );
		sqlite3_free( sqlite_query );
	}

	if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
	{
		printf( "done\n" );
		fflush( stdout );
	}
}
void http_sqlite_moved_check( struct HTTP* http )
{
	char* sqlite_query;
	char found;

	if ( http_get_opt( http, HTTP_OPTION_SQLITE_DB_DISABLED ) )
	{
		if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
		{
			printf( "\nDISABLED: Check if '%s' is moved permanently by the server\n", http->header->originalQuery );
			fflush( stdout );
		}
		return;
	}

	if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
	{
		printf( "\nCheck if '%s' is moved permanently by the server...", http->header->originalQuery );
		fflush( stdout );
	}

	// "Select" to check if update or insert
	sqlite_query = sqlite3_mprintf( HTTP_SQLITE_REDIRECT_SELECT, http->header->originalQuery );
	sqlite3_prepare( http->sqlite_handle, sqlite_query, -1, &http->stmt, NULL );
	sqlite3_free( sqlite_query );

	found = 0;
	if ( sqlite3_step( http->stmt ) != SQLITE_DONE )
	{
		// New url
		found = 1;
		free( http->header->originalQuery );
		http->header->originalQuery = new_string( (const char*)sqlite3_column_text( http->stmt, 1 ) );
	}
	sqlite3_finalize( http->stmt );

	if ( http_get_opt( http, HTTP_OPTION_VERBOSE ) )
	{
		if ( found )
		{
			printf( "yes\nResolved location: %s\n", http->header->originalQuery );
		}
		else
		{
			printf( "no\n" );
		}
		fflush( stdout );
	}
}
