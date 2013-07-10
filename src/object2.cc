#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "define.h"
#include "struct.h"


bool get_trigger    ( char_data*, obj_data* );
bool put_trigger    ( char_data*, obj_data*, obj_data* );
  

/*
 *
 */


obj_data* find_type( thing_array& array, int type )
{
  obj_data* obj;

  for( int i = 0; i < array; i++ )
    if( ( obj = object( array[i] ) ) != NULL
      && obj->pIndexData->item_type == type ) {
      obj->selected = 1;
      obj->shown    = 1;
      return obj;
      }

  return NULL;
}


obj_data* find_vnum( thing_array& array, int vnum )
{
  obj_data* obj;

  for( int i = 0; i < array; i++ )
    if( ( obj = object( array[i] ) ) != NULL
      && obj->pIndexData->vnum == vnum )
      return obj;

  return NULL;
}


/*
 *   OBJECT MANLIPULATION ROUTINES
 */

extern char_data* forcing_char;

/* Smith 15-4-2000 added selective inventory - changed PUIOK 23/4/2000 */
void do_inventory( char_data* ch, char* argument )
{
  char        long_buf  [ MAX_STRING_LENGTH ];
  char             buf  [ MAX_STRING_LENGTH ];
  char          string  [ MAX_STRING_LENGTH ];
  thing_data*    thing;
  obj_data*        obj;
  bool         nothing  = TRUE;
  const char*     name;
  int             wght  = 0;
  int           i, col;

  if( is_confused_pet( ch ) )
    return;

  if( ch->species != NULL
    && !is_set( &ch->species->act_flags, ACT_CAN_CARRY )
    && get_trust( ch ) < LEVEL_APPRENTICE ) {
    send( ch, "You are unable to carry items.\n\r" );
    return;
    }

  *long_buf = '\0';

  for( i = 0; i < ch->contents; i++ ) {
    thing = ch->contents[i];
    if( ( obj = object( thing ) ) != NULL
      && obj->pIndexData->item_type == ITEM_MONEY ) { 
      obj->selected = 0;
      wght += obj->Weight( );
      }
    else 
      thing->selected = thing->number;
    }

  rehash_weight( ch, ch->contents );

  page( ch, "Coins: %d = [%s ]    Weight: %.2f lbs\n\r\n\r",
    get_money( ch ), coin_phrase( ch ), float( wght/100. ) );

  strcpy( string, "Item                          Num  Wgt" );
  page( ch, "%s%s   %s\n\r", bold_v( ch ), string, string );
  strcpy( string, "----                          ---  ---");
  page( ch, "%s   %s%s\n\r", string, string, c_normal( ch ) );
  
  bool use_lifo;  /* PUIOK 27/7/2000 LIFO/FIFO option */
  if( forcing_char == NULL )
    use_lifo = ( ch == NULL || ch->pcdata == NULL
      || !is_set( ch->pcdata->pfile->flags, PLR_FIFO_ORDERING ) );
  else
    use_lifo = forcing_char->pcdata == NULL
      || !is_set( forcing_char->pcdata->pfile->flags, PLR_FIFO_ORDERING );

  int size = ch->contents.size;
  for( col = 0, i = 0; i < size; i++ ) {
    thing = ch->contents[ use_lifo ? ( size - i - 1 ) : i ]; 
    if( thing->shown == 0 )
      continue;
    if( ( obj = object( thing ) ) != NULL
      && ( ( wizard( ch ) == NULL
      || !is_set( ch->pcdata->pfile->flags, PLR_SHOW_NO_SHOW ) )
      && is_set( obj->extra_flags, OFLAG_COOKIE ) ) )
      continue;
    
    /* Smith - 15-4-2000 - selective inventory - mod PUIOK 23/4/2000 */
    if( *argument != '\0' && !is_name( argument, thing->Keywords( ch ) ) )
      continue;
    
    name    = thing->Name( ch );
    nothing = FALSE;

    if( strlen( name ) < 30 ) {
      sprintf( buf, "%-30s%3s%5s%s", name,
        int3( thing->shown ), float3( thing->temp/100. ),
        ++col%2 == 0 ? "\n\r" : "   " );
      page( ch, buf );
      }
    else {
      sprintf( long_buf+strlen( long_buf ), "%-71s%3s%5s\n\r",
        name, int3( thing->shown ), float3( thing->temp/100. ) );
      }
    }

  if( col%2 == 1 )
    page( ch, "\n\r" );

  if( *long_buf != '\0' ) {
    if( col != 0 )
      page( ch, "\n\r" );
    page( ch, long_buf );
    }

  if( nothing ) 
    page( ch, "< empty >\n\r" ); 

  i = ch->get_burden( );

  page( ch, "\n\r  Carried: %6.2f lbs   (%s%s%s)\n\r",
    float( ch->contents.weight/100. ),
    color_scale( ch, i ), burden_name[i], c_normal( ch ) );
  page( ch, "     Worn: %6.2f lbs\n\r",
    float( ch->wearing.weight/100. ) );
  page( ch, "   Number: %6d       ( Max = %d )\n\r",
    ch->contents.number, ch->can_carry_n( ) );
}


/*
 *   JUNK ROUTINE
 */


const char* empty_msg = "You beg and plead to Azrothi, Goddess of Lechery\
 and Treachery,  but only attract a following of off duty mail daemons\
 who taunt you that the goddess has nothing to return you.";

const char* junk_undo_msg = "You abase yourself before Azrothi, Goddess\
 of Lechery and Treachery fervent prayer for your lost items.  Fortunately\
 it seems your whining is noticed.";

thing_data* junk( thing_data* thing, char_data*, thing_data* )
{
  return thing;
}


thing_data* cant_junk( thing_data* thing, char_data*, thing_data* )
{
  obj_data* obj = object( thing );
  
  if( is_set( obj->extra_flags, OFLAG_NO_JUNK ) )
    return (thing_data*) NULL;
  else
    return thing;
}

void execute_junk( event_data* event )
{
  player_data* pc = (player_data*) event->owner;

  extract( pc->junked );
  event->owner->events -= event;

  delete event;
}
  
/* PUIOK 28/2/2000 - added cant_junk check */
void do_junk( char_data* ch, char* argument )
{
  thing_array*  array;
  thing_array   subset  [ 4 ];
  thing_func*     func  [ 4 ]  = { in_use, cursed, cant_junk, junk };
  player_data*      pc         = player( ch );
  event_data*    event;
  obj_data*        obj;

  page_priv( ch, NULL, empty_string );

  if( !strcasecmp( argument, "undo" ) ) {
    if( pc == NULL ) {
      send( ch, "Only player may junk undo.\n\r" );
      return;
      }
    if( is_empty( pc->junked ) ) {
      fsend( ch, empty_msg );
      return;
      }
    fpage( ch, junk_undo_msg );
    page( ch, "\n\r" );

    page_priv( ch, &pc->junked, NULL, NULL,
      "appears in a flash of light",
      "appear in a flash of light" );

    for( int i = pc->junked-1; i >= 0; i-- ) {
      pc->junked[i]->From( pc->junked[i]->number ); 
      pc->junked[i]->To( ch );
      }
    consolidate( pc->junked );

    stop_events( ch, execute_junk );
    return;
    }

  if( ( array = several_things( ch, argument,
    "junk", &ch->contents ) ) == NULL ) 
    return;

  sort_objects( ch, *array, NULL, 4, subset, func );

  page_priv( ch, &subset[0], "are currently using" );
  page_priv( ch, &subset[1], "can't let go of" );
  page_priv( ch, &subset[2], "can't junk" );
  page_publ( ch, &subset[3], "junk" );

  if( !is_empty( subset[3] ) ) {
    if( pc != NULL ) {
      stop_events( ch, execute_junk );
      extract( pc->junked );
      for( int i = 0; i < subset[3]; i++ ) {
        obj = (obj_data*) subset[3][i];
        obj = (obj_data*) obj->From( obj->selected );
        obj->To( &pc->junked );
        }
      event        = new event_data( execute_junk, ch );
      add_queue( event, 2000 );
      }
    else 
      extract( subset[3] );
    }

  delete array;
}


/*
 *   DROP ROUTINES
 */


thing_data* drop( thing_data* thing, char_data* ch, thing_data* )
{
  obj_data* obj;
  
  if( is_set( &ch->status, STAT_DREAMWALKING ) )
    return thing;

  thing = thing->From( thing->selected );
  if( ( obj = object( thing ) ) != NULL ) {
    set_owner( obj, NULL, ch );
    strip_nosell( obj );
    }
  thing->To( ch->array );

  return thing;
}


void do_drop( char_data* ch, char* argument )
{
  thing_array*  array;
  thing_array   subset  [ 3 ];
  thing_func*     func  [ 3 ]  = { in_use, cursed, drop };

  if( newbie_abuse( ch ) )
    return; 

  if( ( array = several_things( ch, argument,
    "drop", &ch->contents ) ) == NULL ) 
    return;
 
  sort_objects( ch, *array, NULL, 3, subset, func );

  page_priv( ch, NULL, empty_string );
  page_priv( ch, &subset[0], "are currently using" );
  page_priv( ch, &subset[1], "can't let go of" );
  page_publ( ch, &subset[2], "drop" );
  
  if( !is_set( &ch->status, STAT_DREAMWALKING ) );
    consolidate( subset[2] );

  delete array;
}


