#ifndef HTTP_POST_H_INCLUDED
#define HTTP_POST_H_INCLUDED

#include "http_wrapper.h"
#include "http_utils.h"

struct HTTP_POST_FORM_URLENCODED_DATA_ITEM {
  unsigned char* key;
  unsigned char* value;

  struct HTTP_POST_FORM_URLENCODED_DATA_ITEM* next;
};

struct HTTP_POST_FORM_MULTIPART_DATA_ITEM {
  unsigned char* name;
  unsigned char* filename;
  unsigned char* content_type; // 	application/octet-stream by default?

  unsigned char* data;

  struct HTTP_POST_FORM_MULTIPART_DATA_ITEM* next;
};

struct HTTP_POST_FORM_MULTIPART_DATA {
  unsigned char* boundary;
  struct HTTP_POST_FORM_MULTIPART_DATA_ITEM* item_list;
};

unsigned char http_post_form_urlencoded_add( struct HTTP* http, const char* key, const unsigned char* value, unsigned int value_length );
int http_post_encode(char **dest, const unsigned char *src, int length);

#endif // HTTP_POST_H_INCLUDED
