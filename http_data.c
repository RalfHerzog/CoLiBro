#include "http_data.h"

int http_post_encode(char **dest, const unsigned char *src, int length)
{
  char *d;
  int i;

  d = *dest;
  for( i = 0 ; i < length ; i++ )
  {
    if ( isCharAlphanumeric( *(src+i) ) || *(src+i) == '-' || *(src+i) == '_' || *(src+i) == '.' || *(src+i) == '~' )
    {
      *(d++) = *(src+i);
    }
    else
    {
      sprintf( d, "%%%.2X", *(src+i) );
      d += 3;
    }
  }
  *d = 0;
  return d-*dest;
}
