#include "math.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "define.h"
#include "struct.h"


/*
 *   ADJACANCY SEEP LIGHTING
 */


#define GRID_CENTER     1
#define GRID_NORTH      2
#define GRID_EAST       3
#define GRID_SOUTH      4
#define GRID_WEST       5
#define GRID_UP         6
#define GRID_DOWN       7
#define GRID_NN         8
#define GRID_NE         9
#define GRID_NW        10
#define GRID_EE        11
#define GRID_EN        12
#define GRID_ES        13
#define GRID_SS        14
#define GRID_SE        15
#define GRID_SW        16
#define GRID_WW        17
#define GRID_WN        18
#define GRID_WS        19


/*
 *   SUNLIGHT
 */


const char* light_name( int i )
{
  const char* adj [] = { "Pitch Dark", "Extremely Dark", "Dark",
    "Dimly Lit", "Lit", "Well Lit", "Brightly Lit", "Blinding" };

  i = (int) log( (float) 2+i );
  i = min( i, 7 );

  return adj[i];
}


int sunlight( int time )
{
  int sunrise = 5*60;
  int sunset  = 20*60;

  if( time < sunrise || time > sunset ) 
    return 10;

  double x = 10+500*sin(M_PI*(time-sunrise)/(sunset-sunrise)); 

  return (int) x;
}


/*
 *   ILLUMINATION
 */


int Thing_Data  :: Light( int ) { return 0; }


int Char_Data :: Light( int )
{
  if( Type( ) == MOB_DATA )
    return wearing.light+species->light;
  
  return wearing.light;
}


int Obj_Data :: Light( int n )
{
  int i  = pIndexData->light;

  i *= ( n == -1 ? number : n );

  return i; 
}   


int dim_light( int light, room_data* room )
{
  double dim_light;
  
  dim_light = light / log( room->size + 3 );
  
  switch( room->sector_type ) {
    case SECT_FOREST:
      dim_light *= 6.0/7.0;
      break;
    case SECT_UNDERWATER:
      dim_light *= 7.0/8.0;
      break;
  }
  
  return (int) dim_light;
}


int adj_room_light( room_data* room, int grid_pos, bool& no_sun, bool& no_lit )
{
  if( room == NULL )
    return 0;
  return dim_light( room->Light( grid_pos, no_sun, no_lit ), room );
}


int Room_Data :: Light( int )
{
  bool no_sun = FALSE;
  bool no_lit = FALSE;
  
  return Light( GRID_CENTER, no_sun, no_lit );
}


int Room_Data :: Light( int grid_pos, bool& no_sun, bool& no_lit )
{
  int i = contents.light;
  char_data*     rch;
  thing_data*  thing;
  
  if( !no_sun && !is_set( &room_flags, RFLAG_INDOORS ) ) {
    i += weather.sunlight;
    no_sun = TRUE;
  }
  
  if( !no_lit && is_set( &room_flags, RFLAG_LIT ) ) {
    i += max( 0, 55 - ( !is_set( &room_flags, RFLAG_INDOORS )
      ? weather.sunlight : 0 ) );
    no_lit = TRUE;
  }
  
  for( int j = 0; j < contents; j++ ) {
    thing = contents[j];
    if( ( rch = character( thing ) ) != NULL )
      for( int k = 0; k < rch->wearing; k++ )
        if( rch->wearing[k]->burning )
          i += rch->wearing[k]->burnlight;
    
    if( thing->burning )
      i += thing->burnlight;
  }
  
  room_data* surround_room [ MAX_DOOR ];
  for( int j = 0; j < MAX_DOOR; j++ )
    surround_room[j] = NULL;
  
  for( int j = 0; j < exits; j++ ) {
    if( is_set( &exits[j]->exit_info, EX_CLOSED ) )
      continue;
    if( exits[j]->direction < MAX_DOOR )
      surround_room[exits[j]->direction] = exits[j]->to_room;
  }
    
  switch( grid_pos ) {
    case GRID_CENTER:
      i += adj_room_light( surround_room[DIR_NORTH], GRID_NORTH, no_sun, no_lit );
      i += adj_room_light( surround_room[DIR_EAST], GRID_EAST, no_sun, no_lit );
      i += adj_room_light( surround_room[DIR_SOUTH], GRID_SOUTH, no_sun, no_lit );
      i += adj_room_light( surround_room[DIR_WEST], GRID_WEST, no_sun, no_lit );
      i += adj_room_light( surround_room[DIR_UP], GRID_UP, no_sun, no_lit );
      i += adj_room_light( surround_room[DIR_DOWN], GRID_DOWN, no_sun, no_lit );
      break;
    case GRID_NORTH:
      i += adj_room_light( surround_room[DIR_NORTH], GRID_NN, no_sun, no_lit );
      i += adj_room_light( surround_room[DIR_EAST], GRID_NE, no_sun, no_lit ) / 2;
      i += adj_room_light( surround_room[DIR_WEST], GRID_NW, no_sun, no_lit ) / 2;
      break;
    case GRID_EAST:
      i += adj_room_light( surround_room[DIR_EAST], GRID_EE, no_sun, no_lit );
      i += adj_room_light( surround_room[DIR_NORTH], GRID_EN, no_sun, no_lit ) / 2;
      i += adj_room_light( surround_room[DIR_SOUTH], GRID_ES, no_sun, no_lit ) / 2;
      break;
    case GRID_SOUTH:
      i += adj_room_light( surround_room[DIR_SOUTH], GRID_SS, no_sun, no_lit );
      i += adj_room_light( surround_room[DIR_EAST], GRID_SE, no_sun, no_lit ) / 2;
      i += adj_room_light( surround_room[DIR_WEST], GRID_SW, no_sun, no_lit ) / 2;
      break;
    case GRID_WEST:
      i += adj_room_light( surround_room[DIR_WEST], GRID_WW, no_sun, no_lit );
      i += adj_room_light( surround_room[DIR_NORTH], GRID_WN, no_sun, no_lit ) / 2;
      i += adj_room_light( surround_room[DIR_SOUTH], GRID_WS, no_sun, no_lit ) / 2;
      break;
  }
  
  return i;
} 


/*
 *   SPELLS
 */


bool spell_create_light( char_data* ch, char_data*, void*,
  int level, int, int )
{
  obj_data*     light;
  event_data*   event;

  if( null_caster( ch, SPELL_CREATE_LIGHT ) )
    return TRUE;

  light = create( get_obj_index( OBJ_BALL_OF_LIGHT ) );
  light->timer = max( 12, light->timer * level/10 );
  
  send( ch, "%s appears in your possession.\n\r", light );
  send( *ch->array, "%s appears in %s's hand.\n\r", light, ch );

/*  
  light->position = WEAR_FLOATING;
  light->layer    = 0;
  light->To( &ch->wearing );
*/
  
  set_owner( light, ch, NULL );
  light->To( ch );
  consolidate( light );
  
  /*
  event = new event_data( execute_extinguish, light );
  add_queue( event, 100 );
  */

  return TRUE;    
}


bool spell_continual_light( char_data *ch, char_data *victim, void*,
  int level, int duration, int )
{
  spell_affect( ch, victim, level, duration, SPELL_CONTINUAL_LIGHT,
    AFF_CONTINUAL_LIGHT );

  return TRUE;
}


