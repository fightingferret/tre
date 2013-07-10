#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "define.h"
#include "struct.h"


extern char_data* forcing_char;

/*
 * 
 */


obj_data* one_object( char_data* ch, char* argument,
  char* text, thing_array* a1, thing_array* a2, thing_array* a3 )
{
  thing_data*  thing;
  obj_data*      obj;

  if( ( thing = (thing_data*) one_visible( ch, argument, text,
    (visible_array*) a1, (visible_array*) a2,
    (visible_array*) a3, NULL, NULL,
    ( visible_data* ( * ) ( visible_data* ) ) &object ) ) == NULL )
    return NULL;

  if( ( obj = object( thing ) ) != NULL )
    return obj;
  
  if( text != empty_string )
    send( ch, "You can't %s %s.\n\r", text, thing );

  return NULL;
}


char_data* one_character( char_data* ch, char* argument,
  char* text, thing_array* a1, thing_array* a2, thing_array* a3 )
{
  thing_data*   thing;
  char_data*   victim;

  if( *argument == '\0' ) {
    if( text != empty_string )
      send( ch, "%s whom?\n\r", text );
    return NULL;
    }

  if( ( thing = one_thing( ch, argument, text, 
    a1, a2, a3, NULL,
    ( visible_data* ( * ) ( visible_data* ) ) &character ) ) == NULL )
    return NULL;

  if( ( victim = character( thing ) ) != NULL )
    return victim;

  if( text != empty_string )
    send( ch, "You can't %s %s.\n\r", text, thing );

  return NULL;
}


player_data* one_player( char_data* ch, char* argument,
  char* text, thing_array* a1, thing_array* a2, thing_array* a3 )
{
  thing_data*    thing;
  player_data*  victim;

  if( ( thing = one_thing( ch, argument, text, a1, a2, a3, NULL,
    ( visible_data* ( * ) ( visible_data* ) ) &player ) ) == NULL )
    return NULL;

  if( ( victim = player( thing ) ) != NULL )
    return victim;

  if( text != empty_string )
    send( ch, "You can't %s %s.\n\r", text, thing );

  return NULL;
}


/*
 *   ERROR MESSAGES
 */


void not_found( char_data* ch, visible_array** array, int count,
  char* keywords )
{
  if( array[0] == (visible_array*) &ch->contents && array[1] == NULL ) {
    if( *keywords == '\0' ) 
      send( ch, "You aren't carrying anything.\n\r" );
    else
      fsend( ch,
        "You %s carrying %s item%s matching '%s'.",
        count == 0 ? "aren't" : "are only",
        count == 0 ? "any" : number_word( count ),
        count == 1 ? "" : "s",
        keywords );
    }

  else if( array[0] == (visible_array*) &ch->wearing && array[1] == NULL )
{
    if( *keywords == '\0' ) 
      send( ch, "You aren't wearing anything.\n\r" );
    else
      fsend( ch,
        "You %s wearing %s item%s matching '%s'.",
        count == 0 ? "aren't" : "are only",
        count == 0 ? "any" : number_word( count ),
        count == 1 ? "" : "s",
        keywords );
    }

  else if( array[0] == (visible_array*) ch->array && array[1] == NULL ) {
    fsend( ch,
      "The room %s contain%s %s item%s matching '%s'.",
      count == 0 ? "doesn't" : "only",
      count == 0 ? "" : "s",
      count == 0 ? "any" : number_word( count ),
      count == 1 ? "" : "s",
      keywords );
    }

  else if( count == 0 ) {
    if( *keywords == '\0' ) 
      send( ch, "Nothing found.\n\r" );
    else
      fsend( ch, "Nothing found matching '%s'.",
        keywords );
    }

  else {
    fsend( ch, "Only %s item%s found matching '%s'.",
      number_word( count ),
      count == 1 ? "" : "s", keywords ); 
    }

  return;
}


/*
 *   ONE_THING
 */


thing_data* one_thing( char_data* ch, char* argument,
  char* text, thing_array* a1, thing_array* a2, thing_array* a3,
  thing_array* a4, visible_data* ( *checker ) ( visible_data* ) )
{
  return (thing_data*) one_visible( ch, argument, text,
    (visible_array*) a1,
    (visible_array*) a2,
    (visible_array*) a3,
    (visible_array*) a4,
    NULL, checker );
}

/* PUIOK 12/7/2000 FIFO/LIFO option */
visible_data* one_visible( char_data* ch, char* argument,
  char* text, visible_array* a1, visible_array* a2,
  visible_array* a3, visible_array* a4, visible_array* a5,
  visible_data* ( *checker ) ( visible_data* ) )
{
  char               tmp  [ MAX_INPUT_LENGTH ];
  int             number;
  int              count  = 0;
  visible_data*  visible;
  visible_array*   array  [ 5 ]  = { a1, a2, a3, a4, a5 };
  int               size;
  bool          use_lifo;

  if( *argument == '\0' ) {
    if( text != empty_string )
      send( ch, "What do you wish to %s?\n\r", text );
    return NULL;
    }

  if( ( number = smash_argument( tmp, argument ) ) == 0 ) {
    if( text != empty_string )
      send( ch, "Zero times an item is always nothing.\n\r" );
    return NULL;
    }

  if( number < 0 ) {
    if( text != empty_string )
      send( ch, "You may only %s one thing at a time.\n\r", text );
    return NULL;
    }

  if( *tmp == '\0' ) {
    if( text != empty_string )
      send( ch, "You must specify at least one keyword.\n\r" );
    return NULL;
    }
  
  if( forcing_char == NULL )
    use_lifo = ( ch == NULL || ch->pcdata == NULL
      || !is_set( ch->pcdata->pfile->flags, PLR_FIFO_ORDERING ) );
  else
    use_lifo = forcing_char->pcdata == NULL
      || !is_set( forcing_char->pcdata->pfile->flags, PLR_FIFO_ORDERING );

  for( int i = 0; i < 5 && array[i] != NULL; i++ ) {
    size = array[i]->size;
    for( int j = 0; j < size; j++ ) {
      visible = array[i]->list[ use_lifo ? ( size - j - 1 ) : j ];
      if( checker != NULL && checker( visible ) == NULL )
        continue;
      if( !visible->Seen( ch )
        || !subset( tmp, visible->Keywords( ch ) ) 
        || ( count += visible->number ) < number )
        continue;
      visible->selected = 1;
      visible->shown    = 1;
      return visible;
      }
    }
  
  if( checker != NULL )
    return one_visible( ch, argument, text, a1, a2, a3, a4, a5 );

  if( text != empty_string )
    not_found( ch, array, count, tmp );
  
  return NULL;
}


/* 
 *   SEVERAL THINGS
 */


thing_array* several_things( char_data* ch, char* argument,
  char* text, thing_array* a1, thing_array* a2, thing_array* a3 )
{
  return (thing_array*) several_visible( ch, argument, text,
    (visible_array*) a1,
    (visible_array*) a2,
    (visible_array*) a3 );
}

/* PUIOK 12/7/2000 FIFO/LIFO option */
visible_array* several_visible( char_data* ch, char* argument,
  char* text, visible_array* a1, visible_array* a2,
  visible_array* a3, visible_array* a4 )
{
  char                tmp  [ MAX_INPUT_LENGTH ];
  visible_array*   output;
  int              number;
  int               count  = 0;
  visible_data*   visible;
  visible_array*    array  [ 4 ]  = { a1, a2, a3, a4 };
  int                size;
  bool           use_lifo;
   
  if( *argument == '\0' ) {
    if( text != empty_string )
      send( ch, "What do you wish to %s?\n\r", text );
    return NULL;
  }

  if( ( number = smash_argument( tmp, argument ) ) != -1 )
    if( *tmp == '\0' ) {
      if( text != empty_string )
        send( ch, "You must specify at least one keyword.\n\r" );
      return NULL;
    }

  if( ( number = smash_argument( tmp, argument ) ) == 0 ) {
    if( text != empty_string )
      send( ch, "Zero times an item is always nothing.\n\r" );
    return NULL;
  }

  output = new visible_array;
  
  if( forcing_char == NULL )
    use_lifo = ( ch == NULL || ch->pcdata == NULL
      || !is_set( ch->pcdata->pfile->flags, PLR_FIFO_ORDERING ) );
  else
    use_lifo = forcing_char->pcdata == NULL
      || !is_set( forcing_char->pcdata->pfile->flags, PLR_FIFO_ORDERING );

  for( int i = 0; i < 4 && array[i] != NULL; i++ ) {
    size = array[i]->size;
    for( int j = 0; j < size; j++ ) {
      visible = array[i]->list[ use_lifo ? ( size - j - 1 ) : j ];
      if( !visible->Seen( ch )
        || !subset( tmp, visible->Keywords( ch ) ) 
        || ( number == -1 && ch == visible ) ) 
        continue;
      count += visible->number;
      if( number > 0 ) {
        if( count < number ) 
          continue; 
        visible->selected = 1;
        *output += visible;
        goto done;
        }
      *output += visible;
      if( number != -1 && count >= -number ) {
        visible->selected = visible->number-number-count;
        goto done;
        }
      visible->selected = visible->number;
      }
    }

  done:

  if( is_empty( *output ) ) {
    delete output;
    if( text != empty_string )
      not_found( ch, array, count, tmp ); 
    return NULL;
  }

  for( int i = 0; i < *output; i++ ) 
    output->list[i]->shown = output->list[i]->selected;

  return output;
}

/* ADDED -- PUIOK 21/1/2000 */

char* one_extra_desc( extra_array* array, char* keyword )
{
  for( int i = 0; i < *array; i++ )
    if( array->list[i] && array->list[i]->keyword )
       if( !strcasecmp( keyword, array->list[i]->keyword ) )
         return array->list[i]->text;
  
  return NULL;
}

/* -- END PUIOK */
