#include "http_data.h"

char* http_post_encode( const char* postData )
{
  unsigned int i, count, length_orig, length_new, old_pos, index_enc, index_orig;
  int result;
  unsigned char j;
  char* encoded_str, *post_encoded;
  char search_list[] = "äÄöÖüÜß";
  char *replace_list[] = { "E4", "C4", "F6", "D6", "FC", "DC", "DF" };
  struct HTTP_LIST* list, *list_it;

  if ( postData == NULL )
  {
    return NULL;
  }

  http_list_init( &list );

  count = 0;
  for ( i = 0 ; *(postData + i) != '\0' ; i++ )
  {
    for ( j = 0 ; j < 7 ; j++ )
    {
      /* Compare 2 bytes */
      result = *(postData+i) - *(search_list+j*2) + *(postData+i+1) - *(search_list+j*2+1);
      if ( !result )
      {
        list_it = http_list_last( list );

        list_it->data = (char*)malloc( 1 );
        memset( list_it->data, j, 1 );

        list_it->size = i;

        http_list_init( &list->next );
        count++;
        break;
      }
    }
  }
  length_orig = i-1;

  // Determinate new post data length
  length_new = length_orig + 2*count;
  post_encoded = (char*)malloc( length_new + 1);

  encoded_str = (char*)malloc( 3 );
  old_pos = index_enc = index_orig = 0;
  list_it = list;
  while( list_it->next != NULL )
  {
    // Index of special char in postData
    index_orig = list_it->size;

    memcpy( post_encoded+index_enc, postData+old_pos, index_orig );
    index_enc += index_orig;

    // Store encoded string
    memset( encoded_str, '%', 1 );
    memcpy( encoded_str+1, replace_list[ (short)*(list_it->data) ], 2 );

    // Copy encoded string
    memcpy( post_encoded+index_enc, encoded_str, 3 );
    index_enc += 3;

    // Skip special char
    old_pos = index_orig + 2;
    list_it = list_it->next;
  }
  memcpy( post_encoded+index_enc, postData+old_pos, length_orig-old_pos );
  index_enc += length_orig-old_pos;
  memset( post_encoded+index_enc, 0, 1 );

  free( encoded_str );
  http_list_free( list );

  return post_encoded;
}
