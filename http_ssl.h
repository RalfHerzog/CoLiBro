#ifndef HTTP_SSL_H_INCLUDED
#define HTTP_SSL_H_INCLUDED

#define HTTP_SSL_PORT 443
#define HTTP_SSL_VERIFY_MODE SSL_VERIFY_NONE

//#include "polarssl/include/polarssl/config.h"
//#include "polarssl/include/polarssl/net.h"
//#include "polarssl/include/polarssl/ssl.h"
//#include "polarssl/include/polarssl/entropy.h"
//#include "polarssl/include/polarssl/ctr_drbg.h"
//#include "polarssl/include/polarssl/error.h"

#include <polarssl/config.h>
#include <polarssl/net.h>
#include <polarssl/ssl.h>
#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>
#include <polarssl/error.h>

struct HTTP_SSL_Connection {
  entropy_context   entropy;
  ctr_drbg_context  ctr_drbg;
  ssl_context       ssl;
  ssl_session       ssl_sess;
};

#include "http_wrapper.h"

//void http_ssl_connect( struct HTTP* http, const char* host, const unsigned short port );

void http_ssl_connect( struct HTTP* http );
int http_ssl_recv( struct HTTP* http, void *__buf, size_t __n );
int http_ssl_send( struct HTTP* http, const char *__buf, size_t __n );
void http_ssl_close( struct HTTP* http );

#endif // HTTP_SSL_H_INCLUDED
