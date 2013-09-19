#ifndef HTTP_HEADER_H_INCLUDED
#define HTTP_HEADER_H_INCLUDED

#include "http_wrapper.h"
#include "http_cookies.h"
#include "http_sqlite.h"

#define HTTP_HEADER_FIELD_VOID 0
#define HTTP_HEADER_FIELD_PERSISTENT 1
#define HTTP_HEADER_FIELD_NON_PERSISTENT 2

#define HTTP_HEADER_MAX_FIELD_SIZE 10024

#define HTTP_HEADER_INIT  0x0100
#define HTTP_HEADER_RESET 0x0200
#define HTTP_HEADER_FREE  0x0400
#define HTTP_HEADER_FREE_WITHOUT_PERSISTENT_DATA 0x0800

#define HTTP_HEADER_STATUS_VERSION_SIZE 8
#define HTTP_HEADER_STATUS_ID_SIZE      3

#ifndef HTTP_HEADER_USER_AGENT_MOBILE
  #define HTTP_HEADER_USER_AGENT_MOBILE  "CoLiBro/1.0 (Other; U; Other; de-de ) Version/1.0 Mobile/1.0 CommandLineBrowser/1.0.1"
#endif

#ifndef HTTP_HEADER_USER_AGENT_DESKTOP
  #if defined _WIN32
    #define HTTP_HEADER_USER_AGENT_DESKTOP "CoLiBro/1.0 (Windows; U; Windows NT 5.1 x86; de-de) Version/1.0 Desktop/1.0 CommandLineBrowser/1.0.1"
  #elif defined _WIN64
    #define HTTP_HEADER_USER_AGENT_DESKTOP "CoLiBro/1.0 (Windows; U; Windows NT 5.1 x86; de-de) Version/1.0 Desktop/1.0 CommandLineBrowser/1.0.1"
  #elif defined __linux
    #define HTTP_HEADER_USER_AGENT_DESKTOP "CoLiBro/1.0 (X11; Linux x86_64; U; de-de) Version/1.0 Desktop/1.0 CommandLineBrowser/1.0.1"
  #else
    #define HTTP_HEADER_USER_AGENT_DESKTOP "CoLiBro/1.0 (Other; Other; U; de-de) Version/1.0 Desktop/1.0 CommandLineBrowser/1.0.1"
  #endif
#endif

#ifndef HTTP_HEADER_USER_AGENT_DESKTOP_MOZILLA
  #define HTTP_HEADER_USER_AGENT_DESKTOP_MOZILLA "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:20.0) Gecko/20100101 Firefox/20.0"
#endif

struct HTTP;

struct HTTP_HEADER_FIELD_ITERATOR{
  struct HTTP_HEADER_FIELD* current;
  struct HTTP* http;
};

struct HTTP_HEADER_FIELD{
  char* key;
  char* value;
  char* domain;
  unsigned char state;
  struct HTTP_HEADER_FIELD* next;
};

struct HTTP_HEADER_STATUS {
  int   responseId;
  char* responseText;
  char* version;
};

struct HTTP_HEADER {
  struct HTTP_HEADER_STATUS status;
  char* method;
  char* remoteFile;
  char* arguments;
  char* server;
  char* connectionState;
  int   contentLength;
  char* contentType;
  char* contentEncoding;  /* eg. Gzip */
  char* transferEncoding; /* chunked */
  char* location;
  char* userAgent;
  char* wwwAutheticate;

  char* originalQuery;

  struct HTTP_COOKIE* cookies;
  struct HTTP_HEADER_FIELD additionalClientFields;
  struct HTTP_HEADER_FIELD additionalServerFields;
};

void http_header_init( struct HTTP_HEADER** header, unsigned int reset );
void http_header_add_client_field( struct HTTP_HEADER* header, const char* key, const char* value, const char* domain, unsigned char state );
void http_header_remove_client_field(struct HTTP_HEADER* header, const char* key );

/** Header field iterator */
void http_header_field_iterator_init( struct HTTP_HEADER_FIELD_ITERATOR* it, struct HTTP* http, void* current );
unsigned char http_header_field_iterator_hasNext( struct HTTP_HEADER_FIELD_ITERATOR* it );
struct HTTP_HEADER_FIELD* http_header_field_iterator_next( struct HTTP_HEADER_FIELD_ITERATOR* it );
void http_header_field_iterator_free( struct HTTP_HEADER_FIELD_ITERATOR* it );

/** Should not be used, only to be visible in extern sources */
extern void http_header_recv_line( struct HTTP* http, char** line, int* size );
extern void http_header_assign_line( struct HTTP* http, const char* line, int size );
extern void http_header_fields_free( struct HTTP_HEADER_FIELD* field );
extern void http_header_follow( struct HTTP* http );

#endif // HTTP_HEADER_H_INCLUDED
