#ifndef HTTP_DATA_H_INCLUDED
#define HTTP_DATA_H_INCLUDED

#include "http_wrapper.h"
#include "http_utils.h"

int http_post_encode(char **dest, const unsigned char *src, int length);

#endif // HTTP_DATA_H_INCLUDED
