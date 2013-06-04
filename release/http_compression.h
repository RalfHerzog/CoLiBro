#ifndef HTTP_COMPRESSION_H_INCLUDED
#define HTTP_COMPRESSION_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "http_wrapper.h"
#include "http_utils.h"

#define windowBits 15
#define ENABLE_ZLIB_GZIP 16+MAX_WBITS
#define ENABLE_ZLIB_DEFLATE -15

unsigned char inflateData ( unsigned char** ucomp, int* usize,
                  const unsigned char* comp, const int csize );
char* base64_string_encode( char* input );

#endif // HTTP_COMPRESSION_H_INCLUDED
