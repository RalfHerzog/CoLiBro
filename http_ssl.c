#include "http_ssl.h"

void http_ssl_connect( struct HTTP* http )
{
  int lastResult;

  /** SSL init */
  entropy_init( &http->ssl.entropy );
  lastResult = ctr_drbg_init( &http->ssl.ctr_drbg, entropy_func, &http->ssl.entropy, (unsigned char*)"HTTP_SSL", 8 );
  if ( lastResult != 0 )
  {
    /** Entropy init failed */
  }

  memset( &http->ssl.ssl_sess, 0, sizeof( ssl_session ) );
  memset( &http->ssl.ssl, 0, sizeof( ssl_context ) );

  http->last_result = net_connect( &http->socket, http->server, http->port );

  lastResult = ssl_init( &http->ssl.ssl );
  if ( lastResult != 0 )
  {
    /** SSL init failed */
  }
  ssl_set_endpoint( &http->ssl.ssl, SSL_IS_CLIENT );
  ssl_set_authmode( &http->ssl.ssl, HTTP_SSL_VERIFY_MODE );

  ssl_set_rng( &http->ssl.ssl, ctr_drbg_random, &http->ssl.ctr_drbg );

  /** Insert debug function here */
  ssl_set_dbg( &http->ssl.ssl, NULL, stdout );
  ssl_set_bio( &http->ssl.ssl, net_recv, &http->socket, net_send, &http->socket );

  ssl_set_ciphersuites( &http->ssl.ssl, ssl_default_ciphersuites );
  ssl_set_session( &http->ssl.ssl, 1, 600, &http->ssl.ssl_sess );
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
  memset( &http->ssl.ssl, 0, sizeof( http->ssl.ssl ) );
}
