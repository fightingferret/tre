#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "define.h"
#include "struct.h"


bool fireball_effect    ( char_data*, char_data*, int );


/*
 *   SMOKE COMMAND
 */


void do_smoke( char_data* ch, char* argument )
{
  obj_data*      pipe;
  obj_data*   tobacco;
  oprog_data*   oprog;

  if( *argument == '\0' ) {
    send( ch, "What do you want to smoke?\n\r" );
    return;
    }

  if( ( pipe = one_object( ch, argument,
    "smoke", &ch->contents ) ) == NULL )
    return;

  if( pipe->pIndexData->item_type == ITEM_TOBACCO ) {
    send( ch, "You need to put that in a pipe to smoke it.\n\r" );
    return;
    }

  if( pipe->pIndexData->item_type != ITEM_PIPE ) {
    send( ch, "That is not an item you can smoke.\n\r" );
    return;
    }

  if( is_empty( pipe->contents ) ) {
    send( ch, "%s contains nothing to smoke.\n\r", pipe );
    return;
    }

  tobacco = object( pipe->contents[0] );

  fsend( ch, "You smoke %s, inhaling the aroma from %s.",
    pipe, tobacco );
  fsend( *ch->array, "%s smokes %s.", ch, pipe );

  for( oprog = tobacco->pIndexData->oprog; oprog != NULL;
    oprog = oprog->next )
    if( oprog->trigger == OPROG_TRIGGER_USE ) {
      var_ch        = ch;
      var_room      = ch->in_room;
      var_obj       = tobacco;
      var_container = pipe;
      if( !execute( oprog ) )
        return;
      if( !pipe->Is_Valid( ) && tobacco->Is_Valid( ) )
        break;
      if( !tobacco->Is_Valid( ) || !pipe->Is_Valid( ) )
        return;
      }

  tobacco->Extract( );
}

bool start_burning( obj_data*& obj, bool msg, int num )
{
  if( obj->burnmass < obj->dampness )
    return FALSE;
  
  if( obj->burning || is_set( obj->pIndexData->extra_flags, OFLAG_BURNING ) )
    return FALSE;

  if( msg ) {
    if( obj->dampness == 0 )
      act_obj( obj, "bursts into flames." );
    else
      act_obj( obj, "starts to burn." );
    }
  
  obj = sunder( obj, num );
  obj->burning = TRUE;

  if( 100*( obj->condition - 50 )/obj->pIndexData->durability >= 60 )
    obj->condition -= (int) ( obj->condition * ( 50 - obj->vs_fire( )/2 )
      / obj->pIndexData->durability );
  consolidate( obj );

  return TRUE;
}

bool stop_burning( obj_data*& obj, int num )
{
  if( is_set( obj->pIndexData->extra_flags, OFLAG_BURNING ) )
    return FALSE;
  
  obj = sunder( obj, num );
  obj->burning = FALSE;
  consolidate( obj );
  
  return TRUE;
}

bool is_burning( obj_data* obj )
{
  if( obj->burning || is_set( obj->extra_flags, OFLAG_BURNING ) )
    return TRUE;
  return FALSE;
}

bool start_fire( char_data* ch, obj_data*& obj )
{
  if( obj->pIndexData->item_type != ITEM_FIRESTARTER )
    return FALSE;
  
  if( obj->dampness > 50 ) {
    send( ch, "%s is too damp to start a fire.\n\r", obj );
    return FALSE;
    }
  
  obj = sunder( obj );    
  if( ( obj->condition -= 10 ) <= 0 ) {
    obj->Extract( );
    return TRUE;
    }
  consolidate( obj );

  return TRUE;
}


/*
 *   IGNITE    need to change to: DO_LIGHT -- 15/5/2000 PUIOK
 */


void do_ignite( char_data* ch, char* argument )
{
  affect_data  affect;
  obj_data*       obj;
  obj_data*    source  = NULL;
  int          found  = FALSE;
  
  if( ( obj = one_object( ch, argument, "light", &ch->contents,
    ch->array, &ch->wearing ) ) == NULL )
    return;

  if( is_burning( obj ) ) {
    send( ch, "%s is already burning.\n\r", obj );
    return;
  }

  for( int i = ch->array->size-1; !found && i >= 0; i-- )
    if( ( source = object( ch->array->list[i] ) ) != NULL
      && ( is_burning( source )
      || source->pIndexData->item_type == ITEM_FIRE ) ) 
      found = TRUE;

  for( int i = ch->contents.size-1; !found && i >= 0; i-- )
    if( ( source = object( ch->contents[i] ) ) != NULL
      && ( is_burning( source )
      || source->pIndexData->item_type == ITEM_FIRE ) )
      found = TRUE + 1;

  for( int i = ch->contents.size-1; !found && i >= 0; i-- )
    if( ( source = object( ch->contents[i] ) ) != NULL
      && source->pIndexData->item_type == ITEM_FIRESTARTER )
      found = TRUE + 2;
  
  if( !found ) {
    if( obj->dampness > obj->burnmass && obj->burnmass > 0 ) {
      fsend( ch, "You have nothing with which to light %s.  Besides, it\
        is unlikely it will catch with the damp.\n\r", obj );
      }
    else
      fsend( ch, "You have nothing with which to light %s.\n\r", obj );
    return;
  }
  
  source->selected = 1;
  source->shown    = 1;
  
  if( obj->burnmass <= 0 ) {
    fsend( ch, "%s doesn't seem to catch light.\n\r", obj );
    return;
  }
  
  if( obj->dampness > obj->burnmass ) {
    fsend( ch, "%s is too damp to catch light.\n\r", obj );
    return;
  }
  
  if( found == 3 ) {
    if( !start_fire( ch, source ) )
      return;
    send( ch, "You use %s to light %s.\n\r", source, obj );
    send( *ch->array, "%s fiddles with %s.\n\r", ch, source );
  }
  else {
    send( ch, "You light %s with %s.\n\r", obj, source );
    send( *ch->array, "%s lights %s with %s.\n\r", ch, obj, source );
  }
    
  start_burning( obj, TRUE );
}


thing_data* notburning( thing_data* thing, char_data*, thing_data* )
{
  obj_data* obj = object( thing );

  if( !is_burning( obj ) )
    return NULL;

  return obj;
}


thing_data* cantextinguish( thing_data* thing, char_data*, thing_data* t2 )
{
  obj_data* obj1 = object( thing );
  obj_data* obj2 = obj1;
  thing_array*  array  = (thing_array*) t2;
  
  if( !stop_burning( obj1, obj1->selected ) )
    return NULL;
  if( array != NULL )
    *array += obj1;
  
  return obj2;
}


void do_smother( char_data* ch, char* argument )
{
  thing_array* array;
  obj_data*      obj;
  
  if( *argument == '\0' ) {
    send( ch, "Smother what?\n\r" );
    return;
  }
  
  if( ( array = several_things( ch, argument, empty_string,
    &ch->contents ) ) == NULL ) {
      if( ( obj = one_object( ch, argument, empty_string,
        &ch->wearing ) ) == NULL ) {
          send( ch, "Nothing found matching '%s'.\n\r", argument );
          return;
        }
    array = new thing_array;
    *array += obj;
  }
  
  thing_array    subset  [ 3 ];
  thing_func*      func  [ 3 ]  = { notburning, cantextinguish, NULL };
  thing_array    extinguish_array;
  
  sort_objects( ch, *array, (thing_data*) &extinguish_array, 3, subset, func );

  msg_type = MSG_STANDARD;

  page_priv( ch, NULL, empty_string );
  page_priv( ch, &subset[0], NULL, NULL,
    "is not even burning", "are not even burning" );
  page_priv( ch, &subset[1], "cannot put out the flames on" );
  for( int i = 0; i < extinguish_array; i++ )
    extinguish_array[ i ]->burning = TRUE;
  page_publ( ch, &extinguish_array, "smother" );
  for( int i = 0; i < extinguish_array; i++ )
    extinguish_array[ i ]->burning = FALSE;

  delete array;
}


/*
void do_ignite( char_data* ch, char* argument )
{
  affect_data  affect;
  obj_data*       obj;
  obj_data*    source  = NULL;
  bool          found  = FALSE;
 
  if( ch->shdata->race == RACE_TROLL ) {
    send( ch, "Due to the natural attraction of flame and troll flesh and the associated\n\rchildhood nightmares burning things is not one of your allowed hobbies.\n\r" );


    return;
    }

  if( ( obj = one_object( ch, argument, "ignite",
    &ch->wearing, &ch->contents, ch->array ) ) == NULL )
    return;

  if( is_set( obj->extra_flags, OFLAG_BURNING ) ) {
    send( ch, "%s is already burning.\n\r", obj );
    return;
    }

  for( int i = 0; !found && i < *ch->array; i++ )
    if( ( source = object( ch->array->list[i] ) ) != NULL
      && is_set( source->extra_flags, OFLAG_BURNING ) ) 
      found = TRUE;

  for( int i = 0; !found && i < ch->contents; i++ )
    if( ( source = object( ch->contents[i] ) ) != NULL
      && is_set( source->extra_flags, OFLAG_BURNING ) ) 
      found = TRUE;

  if( !found ) {
    send( ch, "You have nothing with which to ignite %s.\n\r", obj );
    return;
    }

  if( obj->vs_fire( ) > 90 ) {
    send( ch, "Depressingly %s doesn't seem inclined to burn.\n\r", obj );
    return;
    }
  
  send( ch, "You ignite %s using %s.\n\r", obj, source );
  send( *ch->array, "%s ignites %s using %s.\n\r", ch, obj, source );
  
  affect.type      = AFF_BURNING;
  affect.duration  = 1;
  affect.level     = 1;
  affect.leech     = NULL;

  add_affect( obj, &affect );
}
*/

/*
 *   FIRE DAMAGE ROUTINES
 */


index_data fire_index [] = 
{
  { "singes",                "singe",                  3 },
  { "scorches",              "scorch",                 7 },
  { "toasts",                "toast",                 15 },
  { "cooks",                 "cook",                  30 },
  { "fries",                 "fry",                   50 },
  { "SEARS",                 "SEAR",                  75 },
  { "CHARS",                 "CHAR",                 100 },
  { "* IMMOLATES *",         "* IMMOLATE *",         140 },
  { "* VAPORIZES *",         "* VAPORIZE *",         200 },
  { "** INCINERATES **",     "** INCINERATE **",     300 },  
  { "** CREMATES **",        "** CREMATE **",        400 },
  { "*** DISINTEGRATES ***", "*** DISINTEGRATE ***",  -1 }
};


bool damage_fire( char_data* victim, char_data* ch, int damage,
  const char* string, bool plural )
{
  damage *= 100-victim->Save_Fire( );
  damage /= 100;

  dam_message( victim, ch, damage, string,
    lookup( fire_index, damage, plural ) );
  
  return inflict( victim, ch, damage, "fire" );
}


int obj_data :: vs_fire( )
{
  int save  = 100;
  int    i;

  for( i = 0; i < MAX_MATERIAL; i++ ) 
    if( is_set( &pIndexData->materials, i ) )
      save = min( save, material_table[i].save_fire );

  if( pIndexData->item_type != ITEM_ARMOR 
    || pIndexData->item_type != ITEM_WEAPON ) 
    return save;

  return save+value[0]*(100-save)/(value[0]+2);
}


/* 
 *   FIRE BASED SPELLS
 */


bool spell_resist_fire( char_data* ch, char_data* victim, void*,
  int level, int duration, int )
{
  spell_affect( ch, victim, level, duration, SPELL_RESIST_FIRE,
    AFF_RESIST_FIRE );
  
  return TRUE;
}


bool spell_fire_shield( char_data* ch, char_data* victim, void*,
  int level, int duration, int )
{
  if( is_submerged( victim ) ) {
    fsend_all( victim->in_room, "The water around %s bubbles briefly.\n\r",
      victim );
    return TRUE;
    }

  spell_affect( ch, victim, level, duration, SPELL_FIRE_SHIELD,
    AFF_FIRE_SHIELD );

  return TRUE;
}


bool spell_ignite_weapon( char_data* ch, char_data*, void* vo,
  int level, int, int )
{
  affect_data  affect;
  obj_data*       obj  = (obj_data*) vo;

  if( null_caster( ch, SPELL_IGNITE_WEAPON ) )
    return TRUE;

  if( is_set( &obj->pIndexData->materials, MAT_WOOD ) ) {
    fsend( ch,
      "%s you are carrying bursts into flames which quickly consume it.",
      obj );
    fsend( *ch->array,
      "%s %s is carrying bursts into flames which quickly consume it.",
      obj, ch );
    obj->Extract( 1 );
    return TRUE;
    }

  affect.type      = AFF_FLAMING;
  affect.duration  = level;
  affect.level     = level;
  affect.leech     = NULL;

  add_affect( obj, &affect );

  return TRUE;
}

 
/*
 *   DAMAGE SPELLS
 */


bool spell_burning_hands( char_data* ch, char_data* victim, void*,
  int level, int, int )
{
  if( null_caster( ch, SPELL_BURNING_HANDS ) ) 
    return TRUE;

  damage_fire( victim, ch, spell_damage( SPELL_BURNING_HANDS, level ),
    "*the burst of flame" );

  return TRUE; 
}


bool spell_flame_strike( char_data* ch, char_data* victim, void*,
  int level, int, int )
{
  if( null_caster( ch, SPELL_FLAME_STRIKE ) ) 
    return TRUE;

  damage_fire( victim, ch, spell_damage( SPELL_FLAME_STRIKE, level ),
    "*An incandescent spear of flame" );

  return TRUE;
}



bool spell_conflagration( char_data* ch, char_data* victim, void*,
  int level, int, int )
{
  if( null_caster( ch, SPELL_CONFLAGRATION ) ) 
    return TRUE;

  damage_fire( victim, ch, spell_damage( SPELL_CONFLAGRATION, level ),
    "*A raging inferno" );

  return TRUE;
}


/*
 *   FIREBALL
 */


bool spell_fireball( char_data* ch, char_data* victim, void*,
  int level, int, int )
{
  if( null_caster( ch, SPELL_FIREBALL ) )
    return TRUE; 

  if( fireball_effect( ch, victim, level ) )
    return TRUE;

  if( victim->in_room != ch->in_room )
    return TRUE;

  if( victim->mount != NULL ) 
    fireball_effect( ch, victim->mount, level );

  if( victim->rider != NULL )
    fireball_effect( ch, victim->rider, level );

  return TRUE;
}


bool fireball_effect( char_data *ch, char_data *victim, int level )
{
  room_data*     dest;
  int               i;

  if( damage_fire( victim, ch, spell_damage( SPELL_FIREBALL,level ),
    "*The raging fireball" ) )
    return TRUE;

  /*
  if( victim->mount != NULL )
    if( number_range( 0, 12 ) > victim->get_skill( SKILL_RIDING ) ) {
      send( "The blast throws you from your mount!\n\r", victim );
      fsend_seen( victim, "%s is thrown from his mount by the blast.",
        victim );
      victim->mount->rider = NULL;
      victim->mount = NULL;
    return FALSE;
    }   

  i = number_range( 0, 20 );

  if( number_range( 0, SIZE_HORSE ) > victim->Size( )
    && i < 6 && victim->Can_Move( i ) ) {
    send( victim, "The blast throws you from the room!\n\r" );
    fsend_seen( victim, "The blast throws %s from the room!", victim );
    dest = victim->in_room->exit[i]->to_room;
    char_from_room( victim );
    char_to_room( victim, dest );
    send( "\n\r", victim );
    do_look( victim, "");
    }
  */

  return FALSE;
}
















