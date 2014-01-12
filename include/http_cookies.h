#ifndef HTTP_COOKIES_H_INCLUDED
#define HTTP_COOKIES_H_INCLUDED

#include "http_wrapper.h"
#include "http_sqlite.h"
#include <regex.h>

struct HTTP;
struct HTTP_BOOL;

enum HTTP_COOKIE_FIELD {
	HTTP_COOKIE_NAME,
	HTTP_COOKIE_VALUE,
	HTTP_COOKIE_DOMAIN,
	HTTP_COOKIE_PATH,
	HTTP_COOKIE_EXPIRES,
	HTTP_COOKIE_SECURE,
	HTTP_COOKIE_HTTP_ONLY
};

struct HTTP_COOKIE{
	char* name;
	char* value;
	char* domain;
	char* path;
	char* expires;
	char* secure;
	char* http_only;
	struct HTTP_COOKIE* next;
};

struct HTTP_COOKIE_ITERATOR{
	struct HTTP_COOKIE* current;
	struct HTTP* http;
};

//void http_header_cookie_iterator_init( struct HTTP_COOKIE_ITERATOR* it, struct HTTP_COOKIE* cookies );
//unsigned char http_header_cookie_iterator_hasNext( struct HTTP_COOKIE_ITERATOR* it );
//struct HTTP_COOKIE* http_header_cookie_iterator_next( struct HTTP_COOKIE_ITERATOR* it );
//void http_header_cookie_iterator_free( struct HTTP_COOKIE_ITERATOR* it );

unsigned char http_header_cookie_user_add( struct HTTP* http,
	const char* name, const char* value, const char* domain, const char* path,
	const char* expires, unsigned char secure, unsigned char http_only
);

/** Should not be used, only to be visible in extern sources */
extern void http_header_cookie_add( struct HTTP* http, const char* line );
extern void http_header_cookie_free( struct HTTP_COOKIE* cookie );
extern void http_sqlite_cookie_server( const char* server, char** sqlite_server );
extern void http_sqlite_cookie_path( const char* path, char** sqlite_path );
extern int http_header_cookie_validate_expires( const char* cookie );
extern void http_header_cookie_sqlite_expires_date_convert_month( const char* month_name, char** month_id );
extern void http_header_cookie_sqlite_expires_date( const char* date_str, char** sqlite_date );
extern void http_header_cookie_sqlite_expires( const char* exp_str, char** sqlite_str );
#endif // HTTP_COOKIES_H_INCLUDED
