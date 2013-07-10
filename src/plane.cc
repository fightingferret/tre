#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "define.h"
#include "struct.h"

bool same_plane( char_data* ch, char_data* victim )
{
  
  if( is_set( &ch->status, STAT_DREAMWALKING )
    != is_set( &victim->status, STAT_DREAMWALKING ) )
    return FALSE;
  
  return TRUE;
}