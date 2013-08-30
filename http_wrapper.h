#ifndef HTTP_WRAPPER_INCLUDED
#define HTTP_WRAPPER_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <zlib.h>
#include <sqlite3.h>

#if defined _WIN32 || defined _WIN64
  #include <winsock2.h>
#else
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h>
#endif

#include "http_header.h"
#include "http_utils.h"
#include "http_link_info.h"
#include "html.h"
#include "http_ssl.h"
#include "http_sqlite.h"
#include "http_data.h"
#include "getch.h"

#define HTTP_PORT 80
#define HTTP_HEADER_NEWLINE "\r\n\0"
#define HTTP_HEADER_NUM_EOL 2

#define HTTP_LOG_FILE "log.txt"

#define HTTP_VERSION "0.1"

#define HTTP_KEY_BACKSPACE 127
#define HTTP_KEY_ESC 27

#define HTTP_INIT  0x01
#define HTTP_RESET 0x02
#define HTTP_FREE  0x04
#define HTTP_FREE_WITHOUT_PERSISTENT_DATA 0x08
#define HTTP_STATUS_REQUERY 0x01

#define HTTP_SQLITE_DB "colibro.db"

#define HTTP_BOOL_FALSE 0
#define HTTP_BOOL_TRUE  1

typedef unsigned char HTTP_BOOL;
typedef unsigned int  HTTP_HEX;

enum HTTP_OPTION_STATUS{
  /** Internal options */
  HTTP_OPTION_APPLY_ALL=0,
  HTTP_OPTION_HOST_CHANGED,

  /** User options */
  HTTP_OPTION_VERBOSE,
  HTTP_OPTION_RECV_TIMEOUT,
  HTTP_OPTION_SEND_TIMEOUT,
  HTTP_OPTION_CONNECT_TIMEOUT,
  HTTP_OPTION_LOG_ENABLED,
  HTTP_OPTION_LOG_RESPONSE,
  HTTP_OPTION_LOG_OVERWRITE,
  HTTP_OPTION_USER_AGENT_MOBILE,
  HTTP_OPTION_CONNETION_CLOSE_AFTER_TRANSMISSION,
  HTTP_OPTION_DOWNLOAD_FILES,
  HTTP_OPTION_CONTENT_BINARY,
  HTTP_OPTION_DECOMPRESS,
  HTTP_OPTION_SSL_ENABLED,
  HTTP_OPTION_POTOCOL_CHANGED,
  HTTP_OPTION_SQLITE_DB_DISABLED,
  HTTP_OPTION_DOWNLOAD_FOLDER
};

enum HTTP_ERROR_STATUS {
  HTTP_ERROR_SOCKET_ERROR = -1,
  HTTP_ERROR_HOST_MISSING,
  HTTP_ERROR_HOST_LOOKUP_FAILURE,
  HTTP_ERROR_CONNECT_FAILED,
  HTTP_ERROR_RAW_SEND_ERROR,
  HTTP_ERROR_RAW_RECV_ERROR,
  HTTP_ERROR_RECV_ERROR,
  HTTP_ERROR_DOWNLOAD_SAVE_FILE,
  HTTP_ERROR_EXPECT_LINEFEED,
  HTTP_ERROR_CONTENT_SIZE_UNKNOWN,
  HTTP_ERROR_CONNECTION_CLOSED,
  HTTP_ERROR_ONLY_HTTP_SUPPORTED,
  HTTP_ERROR_NO_STATUS_CODE_RECIEVED,
  HTTP_ERROR_NO_POST_DATA_PRESENT,
  HTTP_ERROR_NOT_IMPLEMENTED_YET,
  HTTP_ERROR_UNEXPECTED_RESPONSE,
  HTTP_ERROR_DOWNLOAD_FILE_TOO_BIG
};

struct HTTP_LIST{
  char* data;
  int size;
  struct HTTP_LIST* next;
};

struct HTTP_ERROR{
  int errorId;
  char* file;
  int line;
};

struct HTTP{
  int socket;
  int last_result;
  unsigned short port;
  unsigned long long options;

  char* server;
  char* download_folder;

  struct HTTP_ERROR error;
  struct hostent* hostent;
  struct sockaddr_in addr;
  struct HTTP_HEADER* header;
  struct HTTP_SSL_Connection ssl;

  sqlite3*      sqlite_handle;
  sqlite3_stmt* stmt;

  /** Function pointer */
  void (*connect_func)(struct HTTP*);
  int  (*recv_func)   (struct HTTP*, void*, size_t);
  int  (*send_func)   (struct HTTP*, const char*, size_t);
  void (*close_func)  (struct HTTP*);
};

void http_alloc( struct HTTP* http, HTTP_HEX reset );
void http_init( struct HTTP* http );
void http_set_opt( struct HTTP* http, enum HTTP_OPTION_STATUS option, ... );
HTTP_BOOL http_get_opt( struct HTTP* http, enum HTTP_OPTION_STATUS option );
void http_connect( struct HTTP* http, const char* host, const unsigned short port );
void http_get_page( struct HTTP* http, const char* link, char** content, int* size );
void http_print_errorcode(  struct HTTP* http );
void http_close( struct HTTP* http );
void http_free( struct HTTP* http );

void http_write_memory_dump( struct HTTP* http, FILE* fFile );
void http_log_write( struct HTTP* http, const char* str, unsigned int mode );

void http_list_init( struct HTTP_LIST** list );
struct HTTP_LIST* http_list_last( struct HTTP_LIST* list );
void http_list_free( struct HTTP_LIST* list );

/** Should not be used, only to be visible in extern sources */
extern void http_raw_connect( struct HTTP* http );
extern void http_reconnect( struct HTTP* http, const char* sNewHost, const unsigned short port );
extern void http_raw_recv( struct HTTP* http, char** sData, int iBytesToRead, int* iBytesRead );
extern void http_query( struct HTTP* http );
extern void http_parse_link( struct HTTP* http );
extern void http_read_response( struct HTTP* http );
extern void http_handle_response( struct HTTP* http );

#endif // HTTP_WRAPPER_INCLUDED
