#ifndef HTTP_SQLITE_H_INCLUDED
#define HTTP_SQLITE_H_INCLUDED

#include "http_wrapper.h"

struct HTTP;
struct HTTP_COOKIE;

#ifndef HTTP_REG_NOERROR
#define HTTP_REG_NOERROR 0
#endif

#define SAVE_CHAR_RET(a) ( a != NULL ) ? ( *a ) : ( 0 )

/** Database defines */
#define HTTP_SQLITE_DB_TABLE_COOKIES "CREATE TABLE IF NOT EXISTS cookies (\
	Domain		TEXT,\
	Path			TEXT,\
	Name			TEXT,\
	Value			TEXT,\
	Expires		NUMERIC,\
	Secure		NUMERIC,\
	HttpOnly	NUMERIC\
)"
#define HTTP_SQLITE_DB_TABLE_BM "CREATE TABLE IF NOT EXISTS bookmarks (\
	Bookmark TEXT PRIMARY KEY,\
	Website TEXT\
)"
#define HTTP_SQLITE_DB_TABLE_REDIRECT "CREATE TABLE IF NOT EXISTS redirect (\
	Source TEXT,\
	Destination TEXT\
)"
#define HTTP_SQLITE_DB_TABLE_DNS "CREATE TABLE IF NOT EXISTS dns (\
	Url TEXT PRIMARY KEY,\
	IP TEXT\
)"

/** Cookie defines */
#define HTTP_SQLITE_COOKIE_SELECT_ALL_EXPIRED "SELECT * FROM cookies WHERE Expires IS NULL OR Expires < datetime();"

#define HTTP_SQLITE_COOKIE_SELECT "SELECT * FROM cookies \
WHERE Domain='%s' AND Path='%s' AND Name='%s';"

#define HTTP_SQLITE_COOKIE_SELECT_DOMAIN "SELECT * FROM cookies \
WHERE Domain LIKE '%%%s' AND Path LIKE '%s%%' AND ( datetime() < Expires OR Expires='' );"

#define HTTP_SQLITE_COOKIE_INSERT "INSERT INTO cookies \
( Domain, Path, Name, Value, Expires, Secure, HttpOnly ) VALUES \
( '%s', '%s', '%s', '%s', '%s', %i, %i );"

#define HTTP_SQLITE_COOKIE_UPDATE "UPDATE cookies \
SET Value='%s' WHERE Domain='%s' AND Path='%s' AND Name='%s';"

#define HTTP_SQLITE_COOKIE_DELETE "DELETE FROM cookies \
WHERE Domain='%s' AND Path='%s' AND Name='%s';"

/** Redirect defines */
#define HTTP_SQLITE_REDIRECT_SELECT "SELECT * FROM redirect \
WHERE Source='%s';"

#define HTTP_SQLITE_REDIRECT_UPDATE "UPDATE redirect \
SET Destination='%s' WHERE Source='%s';"

#define HTTP_SQLITE_REDIRECT_INSERT "INSERT INTO redirect \
( Source, Destination ) VALUES ( '%s', '%s' );"

int http_sqlite_num_rows( struct HTTP* http, const char* query );
void http_sqlite_db_create( struct HTTP* http );
void http_sqlite_db_startup( struct HTTP* http );
void http_sqlite_cookie_update( struct HTTP* http, struct HTTP_COOKIE* cookie );
void http_sqlite_moved_add( struct HTTP* http );
void http_sqlite_moved_check( struct HTTP* http );

#endif
