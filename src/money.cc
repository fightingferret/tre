#include "ctype.h"
#include "sys/types.h"
#include "stdio.h"
#include "stdlib.h"
#include "define.h"
#include "struct.h"


const int  coin_vnum  [] = { OBJ_COPPER, OBJ_SILVER, OBJ_GOLD, OBJ_PLATINUM }; 
const int   coin_value [] = { 1, 10, 100, 1000 };
const char*  coin_name [] = { "cp", "sp", "gp", "pp" };


/*
 *  VARIOUS MONEY ROUTINES
 */


int monetary_value( obj_data* obj )
{
  int i;

  if( obj->pIndexData->item_type == ITEM_MONEY ) 
    for( i = 0; i < MAX_COIN; i++ )
      if( obj->pIndexData->vnum == coin_vnum[i] ) 
        return obj->selected*coin_value[i];

  return 0;
}


char* coin_phrase( char_data* ch )
{
  int      coins  [ MAX_COIN ];
  obj_data*  obj;

  vzero( coins, MAX_COIN );

  for( int i = 0; i < ch->contents; i++ ) {
    if( ( obj = object( ch->contents[i] ) ) != NULL
      && obj->pIndexData->item_type == ITEM_MONEY ) 
      for( int j = 0; j < MAX_COIN; j++ )
        if( obj->pIndexData->vnum == coin_vnum[j] ) 
          coins[j] += obj->number;
    }

  return coin_phrase( coins );
}


char* coin_phrase( int* num )
{
  static char   buf  [ ONE_LINE ];
  bool         flag  = FALSE;
  int             i;
  int          last;
 
  for( last = 0; last < MAX_COIN; last++ )
    if( num[ last ] != 0 )
      break;

  buf[ 0 ] = '\0';

  for( i = MAX_COIN - 1; i >= last; i-- ) {
    if( num[ i ] == 0 )
      continue;
    sprintf( buf + strlen( buf ), "%s %s%d %s", flag ? "," : "",
      ( i == last && flag ) ? "and " : "", num[i], coin_name[i] );
    flag = TRUE;
    }

  if( !flag ) 
    sprintf( buf, " none" );

  return &buf[0];
}


int get_money( char_data* ch )
{
  obj_data*  obj;
  int        sum  = 0;

  for( int i = 0; i < ch->contents; i++ )
    for( int j = 0; j < 4; j++ )
      if( ( obj = object( ch->contents[i] ) ) != NULL
        && obj->pIndexData->vnum == coin_vnum[j] ) 
        sum += coin_value[j]*obj->number;

  return sum;
}


bool remove_silver( char_data* ch )
{
  obj_data* obj;

  if( ( obj = find_vnum( ch->contents, coin_vnum[1] ) ) != NULL ) {
    obj->Extract( 1 );
    return TRUE;
    }

  return FALSE;
}
 

void add_coins( char_data* ch, int amount, char* message )
{
  obj_data*  obj;
  int        num  [ 4 ];
  int          i;

  for( i = MAX_COIN - 1; i >= 0; i-- ) {
    if( ( num[i] = amount/coin_value[i] ) > 0 ) {
      amount -= num[i]*coin_value[i];
      obj = create( get_obj_index( coin_vnum[i] ), num[i] ); 
      obj->To( ch );
      consolidate( obj );
      }
    }

  if( message != NULL ) 
    send( ch, "%s%s.\n\r", message, coin_phrase( num ) );
}


bool remove_coins( char_data* ch, int amount, char* message ) 
{
  obj_data*            obj;
  obj_data*  last_coin_obj[MAX_COIN];
  obj_data*       coin_obj[MAX_COIN];
  int                coins[MAX_COIN];
  int               number[MAX_COIN];
  int                  pos[MAX_COIN];
  int                  neg[MAX_COIN];
  int                  dum;
  int                    i;
  bool                flag  = FALSE;
  content_array*     where;

  if ( amount <= 0 )
     return TRUE;

  vzero( coin_obj, MAX_COIN );
  vzero( coins,    MAX_COIN );

  for( i = 0; i < ch->contents; i++ ) {
    obj = (obj_data*) ch->contents[i];
    for( int j = 0; j < MAX_COIN; j++ ) {
      if( obj->pIndexData->vnum == coin_vnum[j] ) {
        coin_obj[j] = obj; 
        coins[j] = obj->number;
      }
    }
  }
 
  int j;
  int save_amount = amount;
  for ( j = 0; j < MAX_COIN && amount > 0; j++ ) {
     amount = save_amount;
     vzero( number, MAX_COIN );   
     for( i = j; i >= 0 && amount > 0; i-- ) {
        if ( amount <= coins[i]*coin_value[i] ) {
            number[i] = amount/coin_value[i];
            if ( amount % coin_value[i] != 0 ) {   
	       number[i]++;
            }
            amount -= number[i]*coin_value[i]; 
        }
        else {
           amount -= coins[i]*coin_value[i];
           number[i] = coins[i];
        }
     }
  }

  if( amount > 0 )  
    return FALSE;

  amount = -amount;  
   
  for( ; i >= 0; i-- ) {
    dum = amount/coin_value[i];
    amount -= dum*coin_value[i];
    number[i] -= dum;  
  }
    
  vzero( pos, MAX_COIN );   
  vzero( neg, MAX_COIN );   
  for( i = MAX_COIN - 1; i >= 0; i-- ) {
    if( number[i] > 0 ) {
      coin_obj[i]->Extract( number[i] );
      pos[i] = number[i];
    }
    else if ( number[i] < 0 ) {
      neg[i] = -number[i];
      if( coin_obj[i] == NULL ) {
         obj = create( get_obj_index( coin_vnum[i] ), neg[i] );
         obj->To( ch );
         consolidate( obj );
      }
      else {
        where = coin_obj[i]->array;
        coin_obj[i] =
          (obj_data*) coin_obj[i]->From( coin_obj[i]->number );
        coin_obj[i]->number += neg[i];
        coin_obj[i]->To( where );
      }
      flag = TRUE;
    }
  }

  if( message != NULL ) {
    fsend( ch, "%s%s.\n\r", message, coin_phrase( pos ) );
    if( flag ) 
      send( ch, "You receive%s in change.\n\r", coin_phrase( neg ) );
    }

  return TRUE;
}

/* PUIOK 27/2/2000 - Added split hold */
void do_split( char_data* ch, char* argument )
{
  char message [ TWO_LINES ];
  player_data* pc;
  int amount;
  
  pc = player( ch );

  if( *argument == '\0' )
  {
    if( pc != NULL )
    {
      sprintf( message,
        "You have a current value of %d unsplit coppers.\n\r", pc->split_hold );
      send( ch, message );
      if( pc->split_hold > 0 )
        send( ch, "\n\rUse 'split now' to split or 'split clear' to clear this amount.\n\r" );
    }
    else
      send( ch, "What amount do you wish to split?\n\r" );
    return;
  }
  
  if( !strcmp( argument, "clear" ) )
  {
    if( pc )
    {
      pc->split_hold = 0;
      send( ch, "Split cleared.\n\r" );
    }
    else
      send( ch, "We'll do business when you're in a better form.\n\r" );
    return;
  }

  if( !strcmp( argument, "now" ) )
  {
    if( pc )
    {
      amount = pc->split_hold;
      
      if( get_money( ch ) < amount )
        amount = get_money( ch );
      
      if( amount < 2 )
      {
        send( ch, "It is difficult to split anything less than 2 cp.\n\r" );
        return;
      }
      
      sprintf( message, "You split %d cp.\n\r", amount );
      send( ch, message );
      if( split_money( ch, amount, TRUE ) )
        pc->split_hold = 0;
    }
    else
      send( ch, "You split yourself into two and junk the other.\n\r" );
    return;
  }
/* Shouldn't be needed
  if( !strncmp( argument, "add", 3 ) )
  {
    if( pc )
    {
      argument += 3;
      skip_spaces( argument );
      if( *argument != '\0' && is_number( argument ) )
      {
        amount = atoi( argument );
        pc->split_hold += amount;
        
        sprintf( message,
          "%d cps added to the split hold which is now at %d cps.\n\r",
          amount, pc->split_hold );
        send( ch, message );
      }
      else
        send( ch, "Add how much?\n\r" );
    }
    else
      send( ch, "You split yourself into three and junk two at random.\n\r" );
    return;
  }
*/
  amount = atoi( argument );

  if( amount < 2 ) {
    send( ch, "It is difficult to split anything less than 2 cp.\n\r" );
    return;
    }
 
  if( get_money( ch ) < amount ) {
    send( ch, "You don't have enough coins to split that amount.\n\r" );
    return;
    }

  split_money( ch, amount, TRUE );
}


bool split_money( char_data* ch, int amount, bool msg )
{
  char_data*      gch;
  char_array*   group;
  obj_data*       obj;
  obj_data*  coin_obj  [ MAX_COIN ];
  int      coins_held  [ MAX_COIN ];
  int     coins_split  [ MAX_COIN ];
  int           split;
  int           total;
  int               i;
  int         members  = 1;
  bool       anything  = FALSE;
  bool          found;
  char*        phrase;
  room_data*     room  = ch->in_room;

  if( amount == 0 )
    return FALSE;
  
  group = new char_array;
  
  for( i = 0; i < room->contents; i++ )
    if( ( gch = character( room->contents[i] ) ) != NULL )
      if( gch != ch && is_same_group( gch, ch ) && gch->Seen( ch )
        && !is_set( &gch->status, STAT_PET ) )
        *group += gch;
  
  if( group->size <= 0 )
  {
    if( msg )
      send( ch, "There is noone here to split the coins with.\n\r" );
    return FALSE;
  }
  
  for( i = 0; i < MAX_COIN; i++ )
  {
    coin_obj[i]   = NULL;
    coins_held[i] = 0;
  } 

  for( i = 0; i < ch->contents; i++ )
    if( ( obj = object( ch->contents[i] ) ) != NULL )
      for( int j = 0; j < MAX_COIN; j++ )
        if( obj->pIndexData->vnum == coin_vnum[j] ) 
        {
          coin_obj[j] = obj; 
          coins_held[j] = obj->number;
        }

  split = amount/( group->size + 1);
  
  for( i = 0; i < *group; i++ )
  {
    gch = (*group)[i];
    
    total = 0;  
    found = FALSE;

    for( int j = MAX_COIN - 1; j >= 0; j-- )
    {
      coins_split[j] = min( ( split-total )/coin_value[j], coins_held[j] );
      if( coins_split[j] != 0 )
      {
        total += coin_value[j]*coins_split[j];
        coins_held[j] -= coins_split[j];  
        obj = (obj_data*) coin_obj[j]->From( coins_split[j] );
        set_owner( obj, gch, ch );
        obj->To( &gch->contents );
        consolidate( obj );
        found = TRUE;
        }
      }

    if( found )
    { 
      phrase = coin_phrase( coins_split );
      send( ch, "You give%s to %s.\n\r", phrase, gch );
      send( gch, "%s gives%s to you.\n\r", ch, phrase );
      send( *ch->array, "%s gives%s to %s.\n\r", ch, phrase, gch );
      anything = TRUE;
    }
  }

  if( !anything )
  {
    send( ch, "You lack the correct coins to split that amount.\n\r" );
    return FALSE;
  }
  return TRUE;
}
