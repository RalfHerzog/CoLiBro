#include "http_compression.h"

unsigned char inflateZlibData ( unsigned char** ucomp, int* usize,
                  const unsigned char* comp, const int csize, char mode );

unsigned char inflateZlibData ( unsigned char** ucomp, int* usize,
                  const unsigned char* comp, const int csize, char mode )
{
  z_stream strm = {0};
  unsigned char* tmp;
  struct HTTP_LIST list;
  struct HTTP_LIST* list_it;
  unsigned short chunk = 1024;
  int pos, err = 0;

  tmp = (unsigned char*)malloc( chunk+1 );

  strm.zalloc   = Z_NULL;
  strm.zfree    = Z_NULL;
  strm.opaque   = Z_NULL;
  strm.next_in  = (unsigned char*)comp;
  strm.avail_in = csize;
  inflateInit2 ( &strm, mode );

  *usize = 0;
  list_it = &list;
  for ( ; err != Z_STREAM_END ; )
  {
    memset( tmp, 0, chunk );
    strm.next_out   = tmp;
    strm.avail_out  = chunk;

    err = inflate ( &strm, Z_NO_FLUSH );
    if ( err == Z_DATA_ERROR || err == Z_STREAM_ERROR )
    {
      break;
    }
    memset( tmp+chunk, 0, 1 );

    list_it->size   = strlen( (char*)tmp );
    list_it->data   = new_string_num( (char*)tmp, list_it->size );
    *usize += list_it->size;

    list_it->next = malloc( sizeof( struct HTTP_LIST ) );
    memset( list_it->next, 0, sizeof( struct HTTP_LIST ) );

    list_it = list_it->next;
  }
  free( tmp );
  inflateEnd(&strm);

  if ( err == Z_DATA_ERROR || err == Z_STREAM_ERROR )
  {
    *usize = 0;
    return 0;
  }

  *ucomp = (unsigned char*)malloc( *usize+1 );
  pos = 0;

  list_it = &list;
  while( list_it->next != NULL )
  {
    memcpy( (*ucomp+pos), list_it->data, list_it->size );
    pos += list_it->size;

    list_it = list_it->next;
  }
  http_list_free( &list );

  memset( (*ucomp+*usize), 0, 1 );
  return 1;
}

unsigned char inflateData( unsigned char** ucomp, int* usize,
                                const unsigned char* comp, const int csize)
{
  int ret;
  ret = inflateZlibData( ucomp, usize, comp, csize, ENABLE_ZLIB_GZIP );
  if ( ret == 0 )
  {
    return inflateZlibData( ucomp, usize, comp, csize, ENABLE_ZLIB_DEFLATE );
  }
  return ret;
}

char* base64_string_encode( char* input )
{
  int n;
  unsigned char* content;
  size_t size;

  n = my_strlen( input );
  size = (int)( ((n+2-((n+2)%3))/3*4) +1 );
  content = (unsigned char*)malloc( size );
  base64_encode( content, &size, (unsigned char*)input, n );

  return (char*)content;
}
