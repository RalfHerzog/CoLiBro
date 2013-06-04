#ifndef HTTP_UTILS_H_INCLUDED
#define HTTP_UTILS_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../polarssl/include/polarssl/base64.h"
#include "http_compression.h"

char ctolower( char c );
int xtoi( const char* hex );
int strncmpi( const char* str1, const char* str2, int size );
int strcmpi( const char* str1, const char* str2 );
char* stristr( const char* str1, const char* str2 );
char* new_string( const char* text );
char* new_string_num( const char* text, const unsigned int size );
unsigned int my_strlen( const char* str );
char* my_itoa( const unsigned long count );
void* my_min( void* str1, void* str2 );
char* base64_string_encode( char* input );

unsigned char isCharNumeric( char c );
unsigned char isCharAlpha( char c );
unsigned char isCharAlphanumeric( char c );


#endif // HTTP_UTILS_H_INCLUDED
