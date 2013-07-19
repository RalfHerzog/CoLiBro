#include "http_utils.h"

char ctolower( char c )
{
  if ( c >= 'A' && c <= 'Z' )
  {
    return c + 0x20;
  }
  return c;
}
int strcmpi( const char* str1, const char* str2 )
{
  char* str1_tmp, *str2_tmp;
  unsigned int i;

  if ( str1 == NULL || str2 == NULL )
  {
    return 1;
  }

  str1_tmp = new_string( str1 );
  str2_tmp = new_string( str2 );

  for ( i = 0 ; i < strlen( str1_tmp ) ; i++ )
  {
    str1_tmp[i] = ctolower( str1_tmp[i] );
  }
  for ( i = 0 ; i < strlen( str2_tmp ) ; i++ )
  {
    str2_tmp[i] = ctolower( str2_tmp[i] );
  }
  i = strcmp( str1_tmp, str2_tmp );
  free( str1_tmp );
  free( str2_tmp );
  return i;
}
int strncmpi( const char* str1, const char* str2, int size )
{
  char* str1_tmp, *str2_tmp;
  unsigned int i;

  if ( str1 == NULL )
  {
    return -1;
  }
  if ( str2 == NULL )
  {
    return 1;
  }

  str1_tmp = new_string( str1 );
  str2_tmp = new_string( str2 );

  for ( i = 0 ; i < my_strlen( str1_tmp ) ; i++ )
  {
    str1_tmp[i] = ctolower( str1_tmp[i] );
  }
  for ( i = 0 ; i < my_strlen( str2_tmp ) ; i++ )
  {
    str2_tmp[i] = ctolower( str2_tmp[i] );
  }
  i = strncmp( str1_tmp, str2_tmp, size );
  free( str1_tmp );
  free( str2_tmp );
  return i;
}
char* stristr( const char* str1, const char* str2 )
{
  unsigned short i;

  char* str_1;
  char* str_2;
  char* ptr;

  str_1 = new_string( str1 );
  str_2 = new_string( str2 );

  for ( i = 0 ; str_1[i] != '\0' ; i++ )
  {
    if ( str_1[i] >= 'a' && str_1[i] <= 'z' )
    {
      str_1[i] -= 0x20;
    }
  }
  for ( i = 0 ; str_2[i] != '\0' ; i++ )
  {
    if ( str_2[i] >= 'a' && str_2[i] <= 'z' )
    {
      str_2[i] -= 0x20;
    }
  }

  ptr = strstr( str_1, str_2 );
  if ( !ptr )
  {
    return NULL;
  }
  ptr = (char*)str1 + ( ptr - str_1 );
  return ptr;
}
int xtoi( const char* hex )
{
  int result;
  sscanf(hex, "%X", &result);
  return result;
}
char* itox( const unsigned short value )
{
  char* result;

  if ( value == 0 )
  {
    return new_string( "00" );
  }

  result = (char*)malloc( 5 );
  memset( result, 0, 5 );
  sprintf( result, "%X", (unsigned short)value );
  return result;
}
char* new_string( const char* text )
{
  int size;

  if ( text == NULL )
  {
    return NULL;
  }
  size = strlen( text );
  return new_string_num( text, size );
}
char* new_string_num( const char* text, const unsigned int size )
{
  char* ret;

  if ( text == NULL )
  {
    return NULL;
  }
  ret = (char*)malloc( size + 1 );
  memcpy( ret, text, size );
  memset( ret+size, 0, 1 );
  return ret;
}
unsigned int my_strlen( const char* str )
{
  unsigned int i;
  if ( str == NULL )
  {
    return 0;
  }
  for ( i = 0 ; *(str+i) != '\0' ; i++ );
  return i;
}
char* my_itoa( const unsigned long count )
{
  char* tmp;
  tmp = (char*)malloc( 11 );
  memset( tmp, 0, 11 );
  sprintf( tmp, "%lu", count );
  return tmp;
}
void* my_min( void* str1, void* str2 )
{
  if ( str1 == NULL )
  {
    return str2;
  }
  if ( str2 == NULL )
  {
    return str1;
  }
  return (str1<str2)?(str1):(str2);
}
unsigned char isCharNumeric( char c )
{
  if ( c >= '0' && c <= '9' )
  {
    return 1;
  }
  return 0;
}
unsigned char isCharAlpha( char c )
{
  if ( c >= 'a' && c <= 'z' )
  {
    return 1;
  }
  if ( c >= 'A' && c <= 'Z' )
  {
    return 1;
  }
  return 0;
}
unsigned char isCharAlphanumeric( char c )
{
  return ( isCharAlpha( c ) || isCharNumeric( c ) );
}
