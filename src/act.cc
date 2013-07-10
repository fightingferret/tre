#include "ctype.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "define.h"
#include "struct.h"


/*
 *   ACT_OBJ -- 15/5/2000
 */

/* text should be an event.  eg. "breaks into pieces." */

void act_obj( obj_data* obj, const char* text )
{
  char         tmp  [ MAX_STRING_LENGTH ];
  char_data*    ch = NULL;
  room_data* room;
  int    position;
  
  char* positions1[] = { "carrying", "wearing", "holding", "wielding" };
  char* positions2[] = { "carried", "worn", "held", "wielded" };
  
  if( ( room = Room( obj->array->where ) ) == NULL )
  {
    if( ( ch = character( obj->array->where ) ) == NULL )
      return;
    
    room = ch->in_room;
    position = obj->array == &ch->wearing;
    
    if( position )
      if( obj->position == WEAR_HELD_R || obj->position == WEAR_HELD_L
        || obj->position == WEAR_LIGHT )
      {
        position++;
        if( obj->pIndexData->item_type == ITEM_WEAPON )
          position++;
      }
        
    if( ch->position > POS_SLEEPING ) {   
      sprintf( tmp, "$p you are %s %s", positions1[ position ], text );
      act( ch, tmp, ch, ch, obj );
    }
    
    if( room != NULL )
    {
      sprintf( tmp, "$p %s by $n %s", positions2[ position ], text );
      act_notchar( tmp, ch, ch, obj );
    }
  }
  else
  {
    sprintf( tmp, "$p %s", text );
    act_room( room, tmp, obj, obj, obj );
  }
}

/*
 *   ACT
 */


void act( char_data* to, const char* text, visible_data* ch,
  visible_data* obj, const char* string1, const char* string2 )
{
  char  tmp  [ 3*MAX_STRING_LENGTH ];
  char  buf  [ 3*MAX_STRING_LENGTH ];

  if( to == NULL || to->link == NULL || *text == '\0' )
    return;

  act_print( buf, text, ch, NULL, obj, NULL, string1, string2, to );   
  convert_to_ansi( to, buf, tmp, sizeof(tmp) );
  send( to, tmp );
}


void act( char_data* to, const char* text, visible_data* ch,
  visible_data* victim, visible_data* obj1, visible_data* obj2 )
{
  char  tmp  [ 3*MAX_STRING_LENGTH ];
  char  buf  [ 3*MAX_STRING_LENGTH ];

  if( to == NULL || to->link == NULL || *text == '\0' )
    return;

  act_print( buf, text, ch, victim, obj1, obj2, NULL, NULL, to );   
  convert_to_ansi( to, buf, tmp, sizeof(tmp) );
  send( to, tmp );
}


/*
 *   ACT_AREA
 */


void act_area( const char* text, char_data* ch, char_data* victim,
  visible_data* obj )
{
  room_data*  room;
  char_data*   rch;

  if( ( room = Room( ch->array->where ) ) == NULL )
    return;

  for( room = room->area->room_first; room != NULL; room = room->next ) 
    if( room != ch->array->where )
      for( int i = 0; i < room->contents; i++ ) 
        if( ( rch = character( room->contents[i] ) ) != NULL
          && rch->position > POS_SLEEPING )
          act( rch, text, ch, victim, obj );
}


/*
 *   ACT_NOTCHAR
 */


/*
void act_notchar( const char* text, char_data* ch, visible_data* obj1,
                  visible_data* obj2 )
*/
void act_notchar( const char* text, char_data* ch, visible_data* victim,
                  visible_data* obj1, visible_data* obj2)
{
  char_data* rch;

  for( int i = 0; i < *ch->array; i++ ) 
    if( ( rch = character( ch->array->list[i] ) ) != NULL
      && rch != ch && rch->position > POS_SLEEPING 
      && rch->Accept_Msg( ch ) )
/*      act( rch, text, ch, obj1, obj2 );  */
        act( rch, text, ch, victim, obj1, obj2);
}


void act_notchar( const char* text, char_data* ch, const char* string1,
  const char* string2 )
{
  char_data* rch;

  for( int i = 0; i < *ch->array; i++ ) 
    if( ( rch = character( ch->array->list[i] ) ) != NULL 
      && rch != ch && rch->position > POS_SLEEPING  
      && rch->Accept_Msg( ch ) )
      act( rch, text, ch, NULL, string1, string2 );   
}


void act_notchar( const char* text, char_data* ch, visible_data* obj,
  const char* string1, const char* string2 )
{
  char_data* rch;

  for( int i = 0; i < *ch->array; i++ ) 
    if( ( rch = character( ch->array->list[i] ) ) != NULL
      && rch != ch && rch->position > POS_SLEEPING  
      && rch->Accept_Msg( ch ) )
      act( rch, text, ch, obj, string1, string2 );   
}


/* 
 *   ACT_ROOM
 */


void act_room( room_data* room, const char* text, const char* string1,
  const char* string2 )
{
  char_data* rch;

  for( int i = 0; i < room->contents; i++ ) 
    if( ( rch = character( room->contents[i] ) ) != NULL
      && rch->position > POS_SLEEPING ) // what about the blind?
      act( rch, text, NULL, NULL, string1, string2 );   
}

/*  PUIOK 25/12/1999, Added */

void act_room( room_data* room, const char* text, 
  visible_data* obj1, visible_data* obj2,  visible_data* obj3 )
{
  char_data* rch;

  for( int i = 0; i < room->contents; i++ ) 
    if( ( rch = character( room->contents[i] ) ) != NULL
      && rch->position > POS_SLEEPING )
      act( rch, text, obj1, obj2, obj3 );
}

/* -- END PUIOK */


/*
void act_room( room_data* room, const char* text, visible_data* ch, 
	       visible_data* obj1 )
{
  char_data* rch;

  for( int i = 0; i < room->contents; i++ ) 
    if( ( rch = character( room->contents[i] ) ) != NULL
      && rch->position > POS_SLEEPING ) // what about the blind?
      act( rch, text, ch, NULL, obj1 );
}
*/


/*
 *   ACT_NEITHER
 */


void act_neither( const char* text, char_data* ch, char_data* victim,
  visible_data* obj1, visible_data* obj2 )
{
  char_data* rch;
 
  for( int i = 0; i < *ch->array; i++ ) 
    if( ( rch = character( ch->array->list[i] ) ) != NULL
      && rch != ch && rch != victim && rch->position > POS_SLEEPING
      && rch->Accept_Msg( ch ) )
      act( rch, text, ch, victim, obj1, obj2 );
}


/*
 *  MAIN PRINT ROUTINE FOR ACT
 */


void act_print( char* out, const char* in, visible_data* ch,
  visible_data* victim, visible_data* obj1, visible_data* obj2,
  const char* string1, const char* string2, char_data* to )
{
  char_data*  pers;
  visible_data*  obj3;
  const char*     sub;
  char*         start;

  start = out;

  for( ; ; ) {
    if( *in != '$' ) {
      *out++ = *in;
      }
    else {
      pers = character( islower( *(++in) ) ? ch : victim );

      switch( toupper( *in ) ) {
       default:
        sub = "[BUG]";
        break;

       case '$':
        sub = "$";
        break;       

       case '1':
        sub = ( ch == NULL ? "[BUG]" : ch->Name( to, ch->selected ) );
        break;

       case '2':
        sub = ( victim == NULL ? "[BUG]" :
          victim->Name( to, victim->selected ) );
        break;

       case '3':
        sub = ( obj1 == NULL ? "[BUG]" : obj1->Name( to, obj1->selected ) );
        break;

       case 'P':
        obj3 = ( *in == 'p' ? obj1 : obj2 );
        sub = ( obj3 == NULL ? "[BUG]" : obj3->Name( to, obj3->selected ) );
        break;

       case 'D' :
       case 'T' :
        sub = ( *in == 't' ? string1 : string2 );
        if( sub == NULL )
          sub = "[BUG]";
        break;

       case 'N':
        sub = ( pers == NULL ? "[BUG]" : pers->Name( to ) );
        break;

       case 'E':
        sub = ( pers == NULL ? "[BUG]" : pers->He_She( to ) );
        break;

       case 'M':
        sub = ( pers == NULL ? "[BUG]" : pers->Him_Her( to ) );
        break;
 
       case 'S':
        sub = ( pers == NULL ? "[BUG]" : pers->His_Her( to ) );
        break;
        }
      strcpy( out, sub );
      out += strlen( sub );
      }
    if( *in++ == '\0' )
      break;
    }

  if ( *start == '@' ) { // allow for one color code at start of line
     *(start+2) = toupper( *(start+2) );
  }
  else { // normal case
     *start = toupper( *start );
  }
  out--;

  if( out != start ) {
    if( *(out-1) == '\r' ) 
      return;
    }

  strcpy( out, "\n\r" );
}



