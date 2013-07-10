#include <sys/types.h>
#include <stdio.h>
#include "define.h"
#include "struct.h"


bool    obj_compiled   = FALSE;
bool    mob_compiled   = FALSE;
bool    room_compiled  = FALSE;

void    affect_update    ( char_data* );
void    char_update      ( void );
void    drunk_update     ( char_data* );
/*void    light_update     ( char_data* );*/
void    obj_update       ( void );
bool    plague_update    ( char_data* );
void    poison_update    ( char_data* );
void    room_update      ( void );

/* PUIOK 26/1/2000 */
void    mobile_update    ( void );

/*
 *   MAIN UPDATE FUNCTION
 */


void update_handler( void )
{
  static int       pulse_area;
  static int     pulse_mobile;
  static int   pulse_violence;
  static int      pulse_point;
  static int       pulse_room;
  struct timeval        start;  

  gettimeofday( &start, NULL );

  event_update( );
  delete_list( extracted );

  if( --pulse_area <= 0 ) {
    pulse_area = number_range( PULSE_AREA/2, 3*PULSE_AREA/2 );
    area_update( );
    }

  if( --pulse_mobile <= 0 ) {
    pulse_mobile = PULSE_MOBILE;
    time_update( );
    action_update( );
    auction_update( );
    regen_update( );
    mobile_update( );
    }

  if( --pulse_point <= 0 ) {
    pulse_point = number_range( PULSE_TICK/2, 3*PULSE_TICK/2 );
    char_update( );
    obj_update( );
    }

  if( --pulse_room <= 0 ) {
    pulse_room = number_range( PULSE_ROOM/2, 3*PULSE_ROOM/2 );
    room_update( );
    w3_who( );
    w3_protected_who( );
    }

  if( --pulse_violence <= 0 ) {
    pulse_violence = PULSE_VIOLENCE;
    update_queue( );
    }

  pulse_time[ TIME_UPDATE ] = stop_clock( start );  
}


/*
 *   UPDATE CHARACTERS
 */


inline void update( char_data* ch )
{
  /*light_update( ch ); PUIOK 12/3/2000 */
  affect_update( ch );

  if( !plague_update( ch ) )
    poison_update( ch );

  return;
}


inline void update( mob_data* mob )
{
  mprog_data* mprog;

  if( mob->position == POS_EXTRACTED || mob->in_room == NULL )
    return;

  if( mob->timer > 0 && --mob->timer == 0 ) {
     for( mprog = mob->species->mprog; mprog != NULL; mprog = mprog->next ) {
        if( mprog->trigger == MPROG_TRIGGER_TIMER ) {
           var_mob  = mob;
           var_room = mob->in_room;
           execute( mprog );
           return;
        } 
     } 
  } 

  update( (char_data*) mob );

}


inline void update( player_data* player )
{
  int idle;

  if( !player->In_Game( ) )
    return;

  if( player->save_time+600 < current_time
    && !is_set( &player->status, STAT_CLONED ) ) {
    if( is_set( &player->pcdata->message, MSG_AUTOSAVE ) )
      send( player, "Autosaving...\n\r" );
    write( player );
    reference( player, player->contents, -1 );
    }
  
  if( player->link != NULL && player != player->link->character
    && player == player->link->player )
    idle = current_time-player->link->character->timer;
  else
    idle = current_time-player->timer;

  if( player->shdata->level < LEVEL_APPRENTICE 
    && idle > ( player->link == NULL ? 30 : 300 ) ) {
    if( player->was_in_room == NULL && player->in_room != NULL ) {
      if( player->switched != NULL ) 
        do_return( player->switched, "" );
      player->was_in_room = player->in_room;
      send( player, "You are pulled into nothingness.\n\r" );
      send_seen( player, "%s is pulled into nothingness.\n\r", player );
      write( player );
      player->From( );
      player->To( get_room_index( ROOM_LIMBO ) );
      }
    else if( idle > 900 && !is_set( &player->status, STAT_CLONED ) ) {
      forced_quit( player );
      return;
      }
    }

  if( player->was_in_room != NULL )
    return;
    
  if( player->gossip_pts < 1000
    && number_range( 0, player->pcdata->trust >= LEVEL_AVATAR ? 3 : 5 ) == 0 )
    player->gossip_pts++;
  
  player->shdata->fame  = max( --player->shdata->fame, 0 );
  player->whistle       = max( --player->whistle, 0 );
  player->prayer        = min( ++player->prayer, 1000 );

  condition_update( player );

  update( (char_data*) player );
}


void char_update( void )
{   
  struct timeval   start;
  
  gettimeofday( &start, NULL );

  for( int i = 0; i < mob_list; i++ )
    update( mob_list[i] );
   
  for( int i = 0; i < player_list; i++ )
    update( player_list[i] );

  pulse_time[ TIME_CHAR_LOOP ] = stop_clock( start );  

  return;
}


/*
 *   CHARACTER UPDATE SUBROUTINES
 */


void affect_update( char_data* ch )
{
  affect_data*   aff;

  for( int i = ch->affected.size-1; i >= 0; i-- ) {
    aff = ch->affected[i];
    if( aff->leech == NULL ) {
      if( aff->duration-- == 1 && aff->type == AFF_FIRE_SHIELD
        && ch->position > POS_SLEEPING ) {
        send( ch, "The flames around you begin to fade.\n\r" );
        }   
      else if( aff->type == AFF_DEATH ) {
        update_max_hit( ch );
        update_max_move( ch );
        }
      if( aff->duration <= 0 )
        remove_affect( ch, aff );
      }
    }

  return;
}


bool plague_update( char_data* ch )
{
  affect_data*  aff;
  int           con;

  if( !is_set( ch->affected_by, AFF_PLAGUE ) ) 
    return FALSE;

  for( int i = 0; ; i++ ) {
    if( i >= ch->affected ) {
      bug( "%s has the plague with no affect??", ch->real_name( ) );
      return FALSE;
      }
    aff = ch->affected[i];
    if( aff->type == AFF_PLAGUE )
      break; 
    }

  con = ch->Constitution( );
  ch->mod_con--;
  aff->level++;

  update_max_hit( ch );
  update_max_move( ch );

  if( con == 3  ) {
    death( ch, NULL, "the plague" );
    return TRUE;
    }

  if( aff->level < 3 ) {
    send( *ch->array, "%s coughs violently.\n\r", ch );
    if( aff->level == 1 ) 
      send( ch, "You cough violently.\n\r" );
    else
      send( "You cough, your throat burning in pain.\n\rThis is much worse than\
 any cold, you have the plague!\n\r", ch );
    return FALSE;
    }

  send( "The plague continues to destroy your body.\n\r", ch );
  send( *ch->array,
    "%s coughs violently, %s has obviously contracted the plague!\n\r",
    ch, ch->He_She( ) );
 
  return FALSE;
}


void poison_update( char_data* ch )
{

  if( is_set( ch->affected_by, AFF_POISON ) ) {
    send( ch, "Your condition deteriorates from a poison affliction.\n\r" );
    send_seen( ch,
      "%s's condition deteriorates from a poison affliction.\n\r", ch );
    inflict( ch, NULL, 2, "poison" );
    return;
    }

  if( ch->position == POS_INCAP ) 
    inflict( ch, NULL, 1, "[BUG] incap??" );
  else
    if( ch->position == POS_MORTAL ) 
      inflict( ch, NULL, 2, "bleeding to death" );

  return;
}


/*
 *   TIMED ACTIONS
 */


void room_update( void )
{
  action_data*   action;
  area_data*       area;
  char_data*        rch;
  room_data*       room;
  struct timeval   start;
  
  gettimeofday( &start, NULL );

  for( area = area_list; area != NULL; area = area->next ) 
    for( room = area->room_first; room != NULL; room = room->next ) {
      rch = NULL;
      for( int i = 0; rch == NULL && i < room->contents; i++ )
        rch = player( room->contents[i] );
      for( action = room->action; action != NULL; action = action->next ) 
        if( ( ( rch != NULL && action->trigger == TRIGGER_RANDOM )
          || action->trigger == TRIGGER_RANDOM_ALWAYS )
          && number_range( 0, 999 ) < action->value ) {
          var_room = room; 
          execute( action );
	  }
    }  

  pulse_time[ TIME_RNDM_ACODE ] = stop_clock( start );  
     
  return;
}


/*
 *   RESETTING OF AREAS
 */


void area_update( void )
{
  area_data*        area;
  room_data*        room;
  struct timeval   start;
  bool              save  = TRUE;

  gettimeofday( &start, NULL );

  for( int i = 0; i < max_clan; i++ ) 
    if( clan_list[i]->modified )
      save_clans( clan_list[i] );

  for( area = area_list; area != NULL; area = area->next ) {
    if( ++area->age < 15 && ( area->nplayer != 0 || area->age < 5 ) )
      continue;

    for( room = area->room_first; room != NULL; room = room->next )
      if( !player_in_room( room ) ) {
        reset_room( room );
        save_room_items( room );
        }

    area->age = number_range( 0, 3 );

    if( save && area->modified ) {
      save_area( area ); 
      save = FALSE;
      }
    }
  
  shop_update( );

  pulse_time[ TIME_RESET ] = stop_clock( start );

  return;
}



/*
 *  BURNING OBJECT UPDATE 15/5/2000
 */
 
bool burning_update( obj_data* obj )
{
  /*  Handle dampness */
  if( obj->dampness > obj->pIndexData->dampness )
  {
    rust_object( obj, max( 0, obj->dampness - obj->pIndexData->dampness ) );
    
    if( ( obj->dampness -= 2 ) < obj->pIndexData->dampness )
      obj->dampness = obj->pIndexData->dampness;
  }

  if( !obj->burning || is_set( obj->pIndexData->extra_flags, OFLAG_BURNING ) )
    return FALSE;
  
  if( obj->dampness > obj->burnmass )
  {
    if( stop_burning( obj, obj->number ) )
      act_obj( obj, "smolders with the damp." );
    return FALSE;
  }
      
  if( obj->dampness > obj->pIndexData->dampness )
    if( ( obj->dampness -= 12 ) < obj->pIndexData->dampness )
      obj->dampness = obj->pIndexData->dampness;
  
  if( ( obj->burnmass -= 10 ) < 0 )
    obj->burnmass = 0;
  
  if( obj->burnmass >= 11 && obj->burnmass <= 20 )
    act_obj( obj, "burns less brightly." );
  
  if( obj->burnmass <= 20 )
    obj->burnlight = 2*obj->burnlight/3;
  if( obj->burnmass <= 0 )
  {
    if( stop_burning( obj, obj->number ) )
      act_obj( obj, "burns out." );
    
    if( obj->pIndexData->burnto >= 0 ) /* Transform the object */
    {
      obj_clss_data* new_clss;
      obj_data*       new_obj;
      
      if( ( new_clss = get_obj_index( obj->pIndexData->burnto ) ) != NULL
        && ( new_obj = create( new_clss, obj->number ) ) != NULL )
      {
        new_obj->owner = obj->owner;
        replace_obj( obj, new_obj );
        return TRUE;
      }
    }
    else if( obj->pIndexData->burnto == -2 ) /* Purge the object */
    {
      obj->Extract( );
      return TRUE;
    }
    return FALSE;
  }
  
  if( 100*( obj->condition - 50 )/obj->pIndexData->durability >= 60 )
    obj->condition -= (int) ( obj->condition * ( 50 - obj->vs_fire( )/2 )
      / obj->pIndexData->durability );
  return FALSE;
}


/*
 *  OBJECT UPDATE
 */



void obj_update( void )
{   
  char                     tmp  [ TWO_LINES ];
  affect_data*          affect;
  obj_data*                obj;
  obj_data*           new_fire  = NULL;
  obj_clss_data*     transform;
  oprog_data*            oprog;
  room_data*              room;
  char_data*                ch;
  char*                message;
  struct timeval         start;
  bool                    worn;
  
  gettimeofday( &start, NULL );

  for( int i = 0; i < obj_list; i++ ) {
    obj = obj_list[i];
    if( !obj->Is_Valid( ) || obj->array == NULL )
      continue;

    for( oprog = obj->pIndexData->oprog; oprog != NULL;
      oprog = oprog->next ) {
      if( ( oprog->trigger == TRIGGER_RANDOM )
        && number_range( 0, 999 ) < oprog->value ) {
        var_ch     = character( obj->array->where );
        var_room   = Room( obj->array->where );
        var_obj    = obj;
        execute( oprog );
        break;
      }
    }
  }

  for( int j = 0; j < obj_list; j++ ) {
    if( !( obj = obj_list[j] )->Is_Valid( ) )
      continue;

    for( int i = obj->affected - 1; i >= 0; i-- ) {
      affect = obj->affected[i];
      if( affect->duration > 0 && --affect->duration == 0 )
        remove_affect( obj, affect );     
    }

    if ( obj->array == NULL )
      continue;
    
    if( burning_update( obj ) ) {
      j--;
      continue;
    }
    
    ch = NULL;
    if( ( room = Room( obj->array->where ) ) == NULL )
      if( ( ch = character( obj->array->where ) ) != NULL )
        room = ch->in_room;
    
    if( ch != NULL && obj->pIndexData->vnum == OBJ_BALL_OF_LIGHT
      && ( is_set( ch->affected_by, AFF_CONTINUAL_LIGHT ) ) )
      continue;
        
    if( obj->pIndexData->vnum == OBJ_BALL_OF_LIGHT
      && ( room == NULL || ch != NULL )
      && ( ch == NULL || obj->array != &ch->wearing ) )
      continue;
        
    if( obj->timer <= 0 || --obj->timer > 0 ) {
      if( obj->timer == 1 ) {
        if( obj->pIndexData->item_type == ITEM_LIGHT
          || obj->pIndexData->item_type == ITEM_LIGHT_PERM )
          act_obj( obj, "flickers briefly." );
      }
      continue;
    }
    
    for( oprog = obj->pIndexData->oprog; oprog != NULL; oprog = oprog->next )
      if( oprog->trigger == OPROG_TRIGGER_TIMER ) {
        var_ch   = ch;
        var_obj  = obj;
        var_room = room;
        execute( oprog );
        break;
      }
    
    if( oprog != NULL )
      continue;
    
    /* Didn't find an oprog so purge the item the default way */

    if( obj->pIndexData->item_type == ITEM_FIRE ) {
      if( ( transform = get_obj_index( obj->value[1] ) ) != NULL 
        && ( new_fire = create( transform, obj->number  ) ) != NULL ) {
        new_fire->To( room );
        consolidate( new_fire );
      }
      obj->Extract( );
      j--;
      continue;
    }
    
   if( obj->pIndexData->item_type == ITEM_LIGHT
     || obj->pIndexData->item_type == ITEM_LIGHT_PERM ) {
        act_obj( obj, "extinguishes." );
        if( obj->pIndexData->item_type != ITEM_LIGHT_PERM ) {
          obj->Extract( );
          j--;
        }
        continue;
    }
    
    if( room != NULL ) {
      switch( obj->pIndexData->item_type ) {
        case ITEM_GATE:
          message = "$1 shrinks to a point and vanishes."; 
          break;
        
        case ITEM_FOUNTAIN:
          message = "$1 dries up."; 
          break;
        
        case ITEM_CORPSE:
          message = "$1 rots, and is quickly eaten by a grue.";
          break;
        
        case ITEM_FOOD:
          message = "$1 decomposes.";
          break;
  
        default: 
          message = "$1 vanishes.";
          break;
      }
      
      act_room( room, message, obj );
    }
    
    obj->Extract( );
    j--;
  }
  
  pulse_time[ TIME_OBJ_LOOP ] = stop_clock( start );  

  return;
}

/* ADDED -- PUIOK 26/1/2000 */

void mobile_update( void )
{
  mprog_data*          mprog;
  mob_data*              mob;
  int        room_has_player;

  for( int i = 0; i < mob_list; i++ ) {
    mob = mob_list[i];
    if( mob->position == POS_EXTRACTED || mob->in_room == NULL )
      continue;
    
    room_has_player = -1;
    
    for( mprog = mob->species->mprog; mprog != NULL; mprog = mprog->next ) {
      if( mprog->trigger == MPROG_TRIGGER_TIME ) {
        if( mprog->value == weather.minute+100*weather.hour ) {
          var_mob  = mob;
          var_room = mob->in_room;
          execute( mprog );
        }
      }
      else if( mprog->trigger == MPROG_TRIGGER_RANDOM
        || mprog->trigger == MPROG_TRIGGER_RANDOM_ALWAYS ) {
        if( mprog->trigger == MPROG_TRIGGER_RANDOM ) {
          if( room_has_player == -1 )
            room_has_player = player_in_room( mob->in_room );
          if( !room_has_player )
            continue;
        }
        if( number_range( 0, 999 ) < mprog->value ) {
          var_mob    = mob;
          var_room   = mob->in_room; 
          execute( mprog );
          if( !mob->Is_Valid( ) )
            continue;
        }
      }
    }
  }
}
