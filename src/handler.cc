#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include "define.h"
#include "struct.h"


/*
 *   LIST GENERATING FUNCTIONS
 */


int get_trust( char_data *ch )
{
  if( ch->pcdata == NULL )
    return LEVEL_APPRENTICE - 2;

  if( ch->pcdata->trust != 0 )
    return ch->pcdata->trust;

  return ch->shdata->level;
}


/*
 *   WEIRD ROUTINES
 */

/* REDONE -- PUIOK 26/1/2000 */

char_data* rand_player( room_data* room, int scale, int lower, int upper )
{
  int           count  = 0;
  char_data*       ch;
  
  if( upper == 0 )
    upper = 300; /* MAX_LEVEL */
  
  if( scale == 2 )
  {
    room_data* room_tmp;
    
    for( room_tmp = room->area->room_first; room_tmp != NULL;
      room_tmp = room_tmp->next )
      for( int i = 0; i < room_tmp->contents; i++ )
        if( ( ch = character( room_tmp->contents[i] ) ) != NULL 
          && ch->pcdata != NULL
          && !not_in_range( ch->shdata->level, lower, upper ) )
            count++;

    if( count == 0 )
      return NULL;
    
    count = number_range( 1, count );
    
    for( room_tmp = room->area->room_first; room_tmp != NULL;
      room_tmp = room_tmp->next )
      for( int i = 0; i < room_tmp->contents; i++ )
        if( ( ch = character( room_tmp->contents[i] ) ) != NULL 
          && ch->pcdata != NULL
          && !not_in_range( ch->shdata->level, lower, upper ) )
            if( --count == 0 )
              return ch;

    return NULL;
  }
  
  if( scale == 3 )
  {
    for( int i = 0; i < player_list; i++ ) {
      ch = (char_data*) player_list[i];
      if( player_list[i]->In_Game( )
      && !not_in_range( ch->shdata->level, lower, upper )  )
        count++;
    }

    if( count == 0 )
      return NULL;
    
    count = number_range( 1, count );

    for( int i = 0; i < player_list; i++ ) {
      ch = (char_data*) player_list[i];
      if( player_list[i]->In_Game( )
      && !not_in_range( ch->shdata->level, lower, upper )  )
        if( --count == 0 )
          return ch;
    }
    return NULL;
  }
  
  for( int i = 0; i < room->contents; i++ )
    if( ( ch = character( room->contents[i] ) ) != NULL
      && ch->pcdata != NULL
      && !not_in_range( ch->shdata->level, lower, upper ) )
        count++;
  
  if( count == 0 )
    return NULL;
  
  count = number_range( 1, count );
  
  for( int i = 0; i < room->contents; i++ )
    if( ( ch = character( room->contents[i] ) ) != NULL
      && ch->pcdata != NULL
      && !not_in_range( ch->shdata->level, lower, upper ) )
        if( --count == 0 )
          return ch;
  return NULL;
} 

/* -- END PUIOK */

char_data* rand_victim( char_data* )
{
  /*
  char_data*  rch;
  int           i = 0;

  for( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    if( rch != ch && ( rch->Seen( ch ) || rch->fighting == ch ) )
      i++;

  if( i == 0 )
    return NULL;

  for( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    if( rch != ch && ( rch->Seen( ch ) || rch->fighting == ch )
      && --i == 0 )
      break;

  return rch;
  */
  return NULL;
} 
 


