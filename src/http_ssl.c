#include "http_ssl.h"

void http_ssl_connect( struct HTTP* http )
{
	/** SSL init */
	entropy_init( &http->ssl.entropy );
	http->last_result = ctr_drbg_init( &http->ssl.ctr_drbg, entropy_func, &http->ssl.entropy, (unsigned char*)"HTTP_SSL", 8 );
	if ( http->last_result != 0 )
	{
		/** Entropy init failed */
		http->error.errorId = HTTP_ERROR_SSL_ENTROPY_INIT_FAILED;
		http->error.line = __LINE__;
		http->error.file = __FILE__;
		return;
	}

	memset( &http->ssl.ssl_session, 0, sizeof( ssl_session ) );
	memset( &http->ssl.ssl, 0, sizeof( ssl_context ) );

	http_raw_connect( http );
	if ( http->last_result != 0 )
	{
		/** Connect failed */
		return;
	}

	http->last_result = ssl_init( &http->ssl.ssl );
	if ( http->last_result != 0 )
	{
		/** SSL init failed */
		http->error.errorId = HTTP_ERROR_SSL_INIT_FAILED;
		http->error.line = __LINE__;
		http->error.file = __FILE__;
		return;
	}
	ssl_set_endpoint( &http->ssl.ssl, SSL_IS_CLIENT );
	ssl_set_authmode( &http->ssl.ssl, HTTP_SSL_VERIFY_MODE );

	ssl_set_rng( &http->ssl.ssl, ctr_drbg_random, &http->ssl.ctr_drbg );

	/** Insert debug function here */
	ssl_set_dbg( &http->ssl.ssl, NULL, stdout );
	ssl_set_bio( &http->ssl.ssl, net_recv, &http->socket, net_send, &http->socket );

	ssl_set_session( &http->ssl.ssl, &http->ssl.ssl_session );
}
int http_ssl_recv( struct HTTP* http, void *__buf, size_t __n )
{
	return ssl_read( &(http->ssl.ssl), __buf, __n );
}
int http_ssl_send( struct HTTP* http, const char *__buf, size_t __n )
{
	return ssl_write( &http->ssl.ssl, (const unsigned char*)__buf, __n );
}
void http_ssl_close( struct HTTP* http )
{
	net_close( http->socket );

	ssl_free( &http->ssl.ssl );

	memset( &http->ssl.ctr_drbg, 0, sizeof( ctr_drbg_context ) );
	memset( &http->ssl.entropy, 0, sizeof( entropy_context ) );

	http->header->connection_state = NULL;
}
