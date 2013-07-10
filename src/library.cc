#include "sys/types.h"
#include "stdio.h"
#include "stdlib.h"
#include "define.h"
#include "struct.h"


const char* arg_type_name[] = { "none", "any", "string", "integer",
  "character", "object", "room", "direction", "clan", "skill", "rflag",
  "stat", "class", "religion", "race", "thing" };


/*
 *   FUNCTION TABLE
 */

#define nn NONE
#define st STRING
#define ch CHARACTER
#define ob OBJECT
#define in INTEGER
#define rm ROOM
#define sk SKILL
#define sa STAT
#define di DIRECTION
#define rf RFLAG
#define na NATION
#define cl CLASS
#define re RELIGION
#define rc RACE
#define th THING

const cfunc_type cfunc_list [] =
{
  { "acode",           &code_acode,         nn, { rm, in, nn, nn } },
  { "act_area",        &code_act_area,      nn, { st, ch, ob, ch } },
  { "act_bark",        &code_act_bark,      nn, { st, ch, in, nn } },
  { "act_room",        &code_act_room,      nn, { st, ch, ob, ch } },
  { "act_notchar",     &code_act_notchar,   nn, { st, ch, ob, ch } },
  { "act_notvict",     &code_act_notchar,   nn, { st, ch, ob, ch } },
  { "act_tochar",      &code_act_tochar,    nn, { st, ch, ob, ch } },
  { "act_neither",     &code_act_neither,   nn, { st, ch, ob, ch } },
  { "add_path",        &code_add_path,      in, { ch, ch, nn, nn } },
  { "atoi",            &code_atoi,          in, { st, nn, nn, nn } },
  { "assign_quest",    &code_assign_quest,  nn, { ch, in, nn, nn } },
  { "attack",          &code_attack,        in, { ch, ch, in, st } },
  { "attack_acid",     &code_attack_acid,   in, { ch, ch, in, st } },
  { "attack_cold",     &code_attack_cold,   in, { ch, ch, in, st } },
  { "attack_fire",     &code_attack_fire,   in, { ch, ch, in, st } },
  { "attack_shock",    &code_attack_shock,  in, { ch, ch, in, st } },
  { "attack_room",     &code_attack_room,   nn, { ch, in, st, nn } },
  { "attack_weapon",   &code_attack_weapon, in, { ch, ch, in, st } },
  { "can_fly",         &code_can_fly,       in, { ch, nn, nn, nn } },
  { "can_move",        &code_can_move,      in, { ch, st, nn, nn } },
  { "cast_spell",      &code_cast_spell,    nn, { sk, ch, ch, nn } },
  { "cat_string",      &code_cat_string,    st, { st, st, st, st } },
  { "check_cond",      &code_check_cond,    in, { st, ch, ch, ob } },
  { "char_in_room",    &code_char_in_room,  in, { ch, rm, nn, nn } },
  { "cflag",           &code_cflag,         in, { in, ch, nn, nn } },
  { "class",           &code_class,         cl, { ch, nn, nn, nn } },
  { "close",           &code_close,         nn, { rm, di, nn, nn } },
  { "coin_value",      &code_coin_value,    in, { ob, nn, nn, nn } },
  { "cond_act",        &code_cond_act,      nn, { st, ch, ch, ob } },
  { "dam_message",     &code_dam_message,   nn, { ch, in, st, nn } },
  { "dice",            &code_dice,          in, { in, in, in, nn } },
  { "disarm",          &code_disarm,        in, { ch, in, nn, nn } },
  { "do_spell",        &code_do_spell,      nn, { sk, ch, ch, nn } },
  { "doing_quest",     &code_doing_quest,   in, { ch, in, nn, nn } },
  { "done_quest",      &code_done_quest,    in, { ch, in, nn, nn } },
  { "drain_exp",       &code_drain_exp,     nn, { ch, in, nn, nn } },
  { "drain_stat",      &code_drain_stat,    nn, { ch, sa, in, in } },
  { "find_char",       &code_find_char,     ch, { st, ch, nn, nn } },
  { "find_hp",         &code_find_hp,       in, { ch, nn, nn, nn } },
  { "find_maxhp",      &code_find_maxhp,    in, { ch, nn, nn, nn } },
  { "find_maxmp",      &code_find_maxmp,    in, { ch, nn, nn, nn } }, 
  { "find_maxmv",      &code_find_maxmv,    in, { ch, nn, nn, nn } }, 
  { "find_mob",        &code_find_mob,      ch, { st, ch, nn, nn } },
  // added Trinity 4-9-00
  { "find_mount",      &code_find_mount,    ch, { ch, nn, nn, nn } },
  // end of addition
  { "find_mp",         &code_find_mp,       in, { ch, nn, nn, nn } },
  { "find_player",     &code_find_player,   ch, { st, ch, nn, nn } },
  { "find_room",       &code_find_room,     rm, { in, nn, nn, nn } },
  { "find_skill",      &code_find_skill,    in, { ch, sk, nn, nn } },
  { "find_stat",       &code_find_stat,     in, { ch, sa, nn, nn } },
  { "find_vnum",       &code_find_vnum,     in, { rm, nn, nn, nn } }, 
  { "get_leader",      &code_get_leader,    ch, { ch, nn, nn, nn } },
  { "get_room",        &code_get_room,      rm, { ch, nn, nn, nn } },
  { "get_name",        &code_get_name,      st, { ch, nn, nn, nn } },
  { "get_time",        &code_get_time,      in, { nn, nn, nn, nn } },
  { "get_victim",      &code_get_victim,    ch, { ch, st, st, nn } },
  { "get_vnum_room",   &code_get_vnum_room, rm, { in, nn, nn, nn } },
  { "has_obj",         &code_has_obj,       ob, { in, ch, nn, nn } },
  { "has_mflag",       &code_has_mflag,     in, { in, in, ch, nn } },
  { "has_quest",       &code_has_quest,     in, { ch, in, nn, nn } },
  { "heal",            &code_heal,          nn, { ch, in, nn, nn } },
  { "inflict",         &code_inflict,       in, { ch, ch, in, st } },
  { "inflict_acid",    &code_inflict_acid,  in, { ch, ch, in, st } },
  { "inflict_cold",    &code_inflict_cold,  in, { ch, ch, in, st } },
  { "inflict_fire",    &code_inflict_fire,  in, { ch, ch, in, st } },
  { "inflict_shock",   &code_inflict_shock, in, { ch, ch, in, st } },
  { "int_string",      &code_int_string,    st, { in, st, st, nn } },
  { "interpret",       &code_interpret,     nn, { ch, st, ch, nn } },
  { "is_exhausted",    &code_is_exhausted,  in, { ch, in, nn, nn } },
  { "is_fighting",     &code_is_fighting,   in, { ch, nn, nn, nn } },
  { "is_follower",     &code_is_follower,   in, { ch, nn, nn, nn } },
  { "is_locked",       &code_is_locked,     in, { rm, di, nn, nn } },
  { "is_mounted",      &code_is_mounted,    in, { ch, nn, nn, nn } },
  { "is_name",         &code_is_name,       in, { st, st, nn, nn } },
  { "is_objtype",      &code_is_objtype,    in, { st, ob, nn, nn } },
  { "is_open",         &code_is_open,       in, { rm, di, nn, nn } },
  { "is_player",       &code_is_player,     in, { ch, nn, nn, nn } },
  { "is_resting",      &code_is_resting,    in, { ch, nn, nn, nn } },
  { "is_searching",    &code_is_searching,  in, { ch, nn, nn, nn } },
  { "is_silenced",     &code_is_silenced,   in, { ch, nn, nn, nn } },
  { "is_time_range",   &code_is_time_range, in, { in, in, in, nn } },
  { "junk_mob",        &code_junk_mob,      nn, { ch, nn, nn, nn } },
  { "junk_obj",        &code_junk_obj,      nn, { ob, in, nn, nn } },
  { "lock",            &code_lock,          nn, { rm, di, nn, nn } },
  { "mpcode",          &code_mpcode,        nn, { ch, in, nn, nn } },
  { "mload",           &code_mload,         ch, { in, rm, nn, nn } },
  { "mob_in_room",     &code_mob_in_room,   ch, { in, rm, nn, nn } },
  { "modify_mana",     &code_modify_mana,   in, { ch, in, nn, nn } },
  { "modify_move",     &code_modify_move,   in, { ch, in, nn, nn } },
  { "num_in_room",     &code_num_in_room,   in, { rm, nn, nn, nn } },
  { "num_mob",         &code_num_mob,       in, { in, rm, nn, nn } },
  { "obj_name",        &code_obj_name,      st, { ob, nn, nn, nn } },
  { "obj_in_room",     &code_obj_in_room,   ob, { in, rm, nn, nn } },
  { "obj_to_room",     &code_obj_to_room,   nn, { ob, rm, nn, nn } },
  { "obj_to_char",     &code_obj_to_char,   nn, { ob, ch, nn, nn } },
  { "obj_value",       &code_obj_value,     in, { ob, in, nn, nn } },
  { "ofind",           &code_ofind,         in, { ob, nn, nn, nn } },
  { "oload",           &code_oload,         ob, { in, nn, nn, nn } },
  { "opcode",          &code_opcode,        nn, { ob, in, nn, nn } },
  { "open",            &code_open,          nn, { rm, di, nn, nn } },
  { "plague",          &code_plague,        nn, { ch, nn, nn, nn } },
  { "players_area",    &code_players_area,  in, { rm, nn, nn, nn } },
  { "players_room",    &code_players_room,  in, { rm, nn, nn, nn } },
  { "poison",          &code_poison,        nn, { ch, nn, nn, nn } },
  { "race",            &code_race,          rc, { ch, nn, nn, nn } },
  { "raise_exp",       &code_raise_exp,     nn, { ch, in, nn, nn } },
  { "random",          &code_random,        in, { in, in, nn, nn } },
  { "rand_char",       &code_rand_char,     ch, { rm, nn, nn, nn } },
  { "rand_player",     &code_rand_player,   ch, { rm, in, in, in } },
  { "remove_cflag",    &code_remove_cflag,  nn, { in, ch, nn, nn } },
  { "remove_rflag",    &code_remove_rflag,  nn, { rf, rm, nn, nn } },
  { "reputation",      &code_reputation,    in, { ch, na, nn, nn } },
  { "rflag",           &code_rflag,         in, { rf, rm, nn, nn } },
  { "sacrifice",       &code_sacrifice,     nn, { ob, ch, nn, nn } },
  { "send_to_area",    &code_send_to_area,  nn, { st, rm, nn, nn } },
  { "send_to_char",    &code_send_to_char,  nn, { st, ch, nn, nn } },
  { "send_to_room",    &code_send_to_room,  nn, { st, rm, nn, nn } },
  { "set_cflag",       &code_set_cflag,     nn, { in, ch, nn, nn } },
  { "set_hp",          &code_set_hp,        nn, { ch, in, nn, nn } },
  { "set_mp",          &code_set_mp,        nn, { ch, in, nn, nn } },
  { "set_mv",          &code_set_mv,        nn, { ch, in, nn, nn } },
  { "set_religion",    &code_set_religion,  nn, { ch, re, nn, nn } },
  { "set_rflag",       &code_set_rflag,     nn, { rf, rm, nn, nn } },
  { "show",            &code_show,          nn, { ch, rm, in, nn } }, 
  { "show_room",       &code_show_room,     nn, { ch, rm, st, nn } }, 
  { "size",            &code_size,          in, { ch, nn, nn, nn } }, 
  { "summon",          &code_summon,        nn, { ch, ch, nn, nn } },
  { "tell",            &code_tell,          nn, { ch, ch, st, nn } },
  { "transfer",        &code_transfer,      nn, { ch, rm, nn, nn } },
  { "transfer_all",    &code_transfer_all,  nn, { rm, rm, nn, nn } },
  { "unlock",          &code_unlock,        nn, { rm, di, nn, nn } },
  { "update_quest",    &code_update_quest,  nn, { ch, in, nn, nn } },
  { "wait",            &code_wait,          nn, { in, nn, nn, nn } },
  { "weight",          &code_weight,        in, { th, nn, nn, nn } },
  { "with_coins",      &code_with_coins,    in, { in, ch, st, in } },
  { "",                NULL,                nn, { nn, nn, nn, nn } }
};

#undef nn
#undef st
#undef ch
#undef ob
#undef in
#undef rm
#undef sk
#undef st
#undef di
#undef rf
#undef na
#undef re
#undef th


/*
 *   POSITION ROUTINES
 */

   
void* code_is_mounted( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  return (void*) ( ch->mount != NULL );
}
//Added Trinity 4-9-00
void* code_find_mount( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
	 code_bug( "Find_mount: NULL character" );
		return NULL;
  }
  else if ( ch->mount == NULL ) {
	 code_bug( "Find_mount: NULL mount" );
		return NULL;
  }
  return ch->mount;
}
//End of addition (Trinity)
void* code_is_resting( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Is_Resting: NULL character." );
    return FALSE;
    }

  return (void*) ( ch->position == POS_RESTING );
}


void* code_is_fighting( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Is_Fighting: NULL character." );
    return (void*) FALSE;
    }

/*  return (void*) ( ch->position == POS_FIGHTING ); */
  return (void*) ( ch->fighting != NULL );
}


void* code_is_player( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Is_Player: NULL character." );
    return (void*) FALSE;
    }

  return (void*) ( ch->species == NULL );
}


/*
 *   ROOM ROUTINES
 */


void* code_rflag( void** argument )
{
  int         flag  = (int)        argument[0];
  room_data*  room  = (room_data*) argument[1]; 

  if( room == NULL ) {
    code_bug( "Code_rflag: NULL room" );
    return (void*) FALSE;
    }

  return (void*) is_set( &room->room_flags, flag );
}


void* code_set_rflag( void** argument )
{
  int         flag  = (int)        argument[0];
  room_data*  room  = (room_data*) argument[1]; 

  if( room == NULL ) {
    code_bug( "Code_rflag: NULL room" );
    return NULL;
    }

  set_bit( &room->room_flags, flag );

  return NULL;
}


void* code_remove_rflag( void** argument )
{
  int         flag  = (int)        argument[0];
  room_data*  room  = (room_data*) argument[1]; 

  if( room == NULL ) {
    code_bug( "Code_rflag: NULL room" );
    return NULL;
    }

  remove_bit( &room->room_flags, flag );

  return NULL;
}


void* code_get_room( void** argument )
{
  char_data* ch = (char_data*) argument[0];

  if( ch == NULL ) {
	 code_bug( "Get_room: NULL character" );
    return NULL;
	 }

  return ch->in_room;
}


/*
 *   CHARACTER ROUTINES
 */


void* code_size( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Size: Null argument" );
    return NULL;
    }

  return (void*) ch->Size( );
}


void* code_weight( void** argument )
{
  thing_data*  thing  = (char_data*) argument[0];

  if( thing == NULL ) {
    code_bug( "Weight: Null argument" );
    return NULL;
    }

  return (void*) thing->Weight( );
}


void* code_race( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Race: Null character." );
    return NULL;
    }

  return (void*) ch->shdata->race;
}


void* code_class( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Class: Null character." );
    return NULL;
    }

  if( ch->species != NULL ) {
    code_bug( "Class: Non-Player character." );
    return NULL;
    }
   
  return (void*) ch->pcdata->clss;
}


/*
 *   CFLAG ROUTINES
 */


void* code_cflag( void** argument )
{
  int         flag  = (int)        argument[0];
  char_data*    ch  = (char_data*) argument[1]; 

  if( ch == NULL ) {
    code_bug( "Code_Cflag: NULL character." );
    return (void*) FALSE;
    }

  if( flag < 0 || flag >= 32*MAX_CFLAG ) {
    code_bug( "Code_Cflag: flag out of range." );
    return (void*) FALSE;
    }

  if( ch->pcdata == NULL )
    return (void*) FALSE;

  return (void*) is_set( ch->pcdata->cflags, flag );
}


void* code_set_cflag( void** argument )
{
  int        flag  = (int)        argument[0];
  char_data*   ch  = (char_data*) argument[1]; 

  if( ch == NULL ) {
    code_bug( "Code_Set_Cflag: NULL character." );
    return NULL;
    }

  if( flag < 0 || flag >= 32*MAX_CFLAG ) {
    code_bug( "Code_Set_Cflag: flag out of range." );
    return NULL;
    }

  if( ch->pcdata != NULL )
    set_bit( ch->pcdata->cflags, flag );

  return NULL;
}


void* code_remove_cflag( void** argument )
{
  int flag      = (int)        argument[0];
  char_data* ch = (char_data*) argument[1]; 

  if( ch == NULL ) {
    code_bug( "Code_Remove_Cflag: NULL character." );
    return NULL;
    }

  if( flag < 0 || flag >= 32*MAX_CFLAG ) {
    code_bug( "Code_Remove_Cflag: flag out of range." );
    return NULL;
    }

  if( ch->pcdata != NULL )
    remove_bit( ch->pcdata->cflags, flag );

  return NULL;
}


/*
 *   CHARACTER STATUS ROUTINES
 */

void* code_find_maxhp( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  if( ch == NULL )
    {
     code_bug( "find_maxhp: NULL char" );
     return (void*) FALSE;
    }
 
  return (void*) ch->max_hit;
}

void* code_find_maxmp( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  if( ch == NULL )
    {
     code_bug( "find_maxmp: NULL char" );
     return (void*) FALSE;
    }
 
  return (void*) ch->max_mana;
}

void* code_set_mp( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  int value = (int) argument[1];
  if( ch == NULL )
    {
     code_bug( "set_mp: NULL char" );
     return (void*) FALSE;
    }
  ch->mana = value;
  return NULL;
}

void* code_set_hp( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  int value = (int) argument[1];
  if( ch == NULL )
    {
     code_bug( "set_hp: NULL char" );
     return (void*) FALSE;
    }

  ch->hit = value;
  return NULL;
}

void* code_find_mp( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  if( ch == NULL )
    {
     code_bug( "find_mp: NULL char" );
     return (void*) FALSE;
    }
  
  return (void*) ch->mana;
}

void* code_find_hp( void** argument )
{
  char_data* ch  = (char_data*) argument[0];

  if( ch == NULL )
    {
     code_bug( "find_hp: NULL char" );
     return (void*) FALSE;
    }
 
  return (void*) ch->hit;
}

void* code_can_fly( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Can_Fly: NULL Character." );
    return (void*) FALSE;
    }

  return (void*) ch->can_fly( );
}


void* code_is_silenced( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Is_Silenced: NULL Character." );
    return (void*) FALSE;
    }

  return (void*) is_set( ch->affected_by, AFF_SILENCE );
}


/*
 *   UNCLASSIFIED 
 */


void* code_interpret( void** argument )
{
  char_data* ch     = (char_data*) argument[0];
  char* string      = (char*)      argument[1];
  char_data* victim = (char_data*) argument[2];

  char tmp [ MAX_INPUT_LENGTH ];
    
  if( ch == NULL )
    return NULL;
  
  if( is_set( &ch->status, STAT_SOFTFORCED ) )
    return NULL;
  
  set_bit( &ch->status, STAT_SOFTFORCED );

  if( victim == NULL ) 
    interpret( ch, string );
  else {
    sprintf( tmp, "%s %s", string, victim->descr->name );
    interpret( ch, tmp );
  }

  remove_bit( &ch->status, STAT_SOFTFORCED );

  return NULL;
}   


void* code_send_to_char( void** argument )
{
  char      *string = (char*)      argument[0];
  char_data *ch     = (char_data*) argument[1];

  send( string, ch );
 
  return NULL;
}


void* code_send_to_room( void** argument )
{
  char*      string = (char*)      argument[0];
  room_data*   room   = (room_data*) argument[1];

  if( room != NULL )
    act_room( room, string );
 
  return NULL;
}


void* code_send_to_area( void** argument )
{
  char      *string = (char*)      argument[0];
  room_data *room   = (room_data*) argument[1];

  if( room != NULL )
    send_to_area( string, room->area ); 
 
  return NULL;
}


void* code_act_room( void** argument )
{
  char*       string  = (char*)      argument[0];
  char_data*      ch  = (char_data*) argument[1];
  obj_data*      obj  = (obj_data*)  argument[2];
  char_data*  victim  = (char_data*) argument[3];  

  if( ch != NULL && ch->in_room != NULL )
    act_room( ch->in_room, string, ch, victim, obj ); /*  PUIOK 25/12/99 */

/*  act_room( ch->in_room, string );*/

  return NULL;
}


void* code_act_neither( void** argument )
{
  char*       string  = (char*)      argument[0];
  char_data*      ch  = (char_data*) argument[1];
  obj_data*      obj  = (obj_data*)  argument[2];
  char_data*  victim  = (char_data*) argument[3];  
  
  act_neither( string, ch, victim, obj );

  return NULL;
}


void* code_act_tochar( void** argument )
{
  char*      string  = (char*)      argument[0];
  char_data*     ch  = (char_data*) argument[1];
  obj_data*     obj  = (obj_data*)  argument[2];
  char_data* victim  = (char_data*) argument[3];  
  
  act( ch, string, ch, victim, obj );

  return NULL;
}


void* code_act_area( void** argument )
{
  char*       string  = (char*)      argument[0];
  char_data*      ch  = (char_data*) argument[1];
  obj_data*      obj  = (obj_data*)  argument[2];
  char_data*  victim  = (char_data*) argument[3];  
  
  if( ch == NULL ) {
    code_bug( "Act_Area: Character = null pointer." );
    return NULL;
    }

  act_area( string, ch, victim, obj );

  return NULL;
}


void* code_act_notchar( void** argument )
{
  char*      string  = (char*)      argument[0];
  char_data*     ch  = (char_data*) argument[1];
  obj_data*     obj  = (obj_data*)  argument[2];
  char_data* victim  = (char_data*) argument[3];  
  
  if( ch != NULL )
    act_notchar( string, ch, victim, obj );

  return NULL;
}


void* code_act_notvict( void** argument )
{
  char      *string = (char*)      argument[0];
  char_data *ch     = (char_data*) argument[1];
  obj_data  *obj    = (obj_data*)  argument[2];
  char_data *victim = (char_data*) argument[3];  
  
  act_notchar( string, ch, victim, obj );
  return NULL;
}


void* code_junk_mob( void** argument )
{
  char_data* ch = (char_data*) argument[0];

  if( ch == NULL )
    return NULL;
 
  if( !IS_NPC( ch ) ) {
    code_bug( "Junk_mob: character is a player??" );
    return NULL;
    } 
  
  ch->Extract( );

  return NULL;
}


void* code_drain_stat( void** argument )
{
  char_data* ch  = (char_data*) argument[0];
  int         i  = (int)        argument[1]; 
  int         j  = (int)        argument[2];
  int         m  = (int)        argument[3];

  int loc[] = { APPLY_STR, APPLY_INT, APPLY_WIS, APPLY_DEX, APPLY_CON }; 
  affect_data affect;
 
  if( ch == NULL || i < 0 || i > 4 || j <= 0 || m < -1 || m > 1 )
    return NULL;
 
  affect.type      = AFF_NONE;
  affect.location  = loc[i];
  affect.modifier  = m;
  affect.duration  = j;
  affect.level     = 5;
  affect.leech     = NULL;

  add_affect( ch, &affect );
  return NULL;
}  


void* code_find_stat( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];
  int          i  = (int)        argument[1]; 

  if( ch == NULL ) 
    return NULL;

  if( i < 0 || i > 8 ) {
    code_bug( "Find_Stat: Impossible field." );
    return NULL;
    } 

  if( ch->species != NULL ) {
    int value[] = { ch->Strength( ), ch->Intelligence( ),
      ch->Wisdom( ), ch->Dexterity( ), ch->Constitution( ),
      ch->shdata->level, 0, 0, ch->shdata->alignment };
    return (void*) value[i];
    }
  else { 
    int value[] = { ch->Strength( ), ch->Intelligence( ),
      ch->Wisdom( ), ch->Dexterity( ), ch->Constitution( ),
      ch->shdata->level, ch->pcdata->piety,
      ch->pcdata->clss, ch->shdata->alignment };
    return (void*) value[i];
    }
}


void* code_random( void** argument )
{
  int  n1  = (int) argument[0];
  int  n2  = (int) argument[1];

  return (void*) ( number_range( n1,n2 ) );
}


void* code_dice( void** argument )
{
  int  n1  = (int) argument[0];
  int  n2  = (int) argument[1];
  int  n3  = (int) argument[2];

  return (void*) ( roll_dice( n1,n2 )+n3 );
}


void* code_find_skill( void** argument )
{
  char_data*    ch  = (char_data*) argument[0];
  int        skill  = (int)        argument[1];

  if( ch == NULL ) {
    code_bug( "Find_Skill: NULL character." );
    return NULL;
    }

  return (void*) ch->get_skill( skill );
}


void* code_summon( void** argument )
{
  char_data*      ch  = (char_data*) argument[0];
  char_data*  victim  = (char_data*) argument[1];

  if( ch != NULL )
    summon_help( ch, victim );

  return NULL;
}


void* code_reputation( void** argument )
{
  char_data*     ch  = (char_data*) argument[0];
  int        nation  = (int)        argument[1];

  if( ch == NULL ) {
    code_bug( "Reputation: Null character." );
    return NULL;
    }

  return (void*) 50;
}


void* code_find_room( void** argument )
{
  int         vnum   = (int) argument[0];
  room_data*  room;

  if( ( room = get_room_index( vnum, FALSE ) ) == NULL ) 
    code_bug( "Find_Room: Non-existent room." );
 
  return room;
}

/* find_vnum added zemus august 1 */
void* code_find_vnum( void** argument )
{
  room_data*  room  = (room_data*) argument[0];
  int         vnum;

  if( room == NULL )
    code_bug( "Find_vnum: Null Room." );
    
  return (void*) room->vnum;
}



void* code_cast_spell( void** argument )
{
  int          spell  = (int)        argument[0];
  char_data*      ch  = (char_data*) argument[1];
  char_data*  victim  = (char_data*) argument[2];

  if( ch == NULL || spell < SPELL_FIRST || spell >= WEAPON_UNARMED ) 
    return NULL;

  switch( spell_table[ spell-SPELL_FIRST ].type ) {
   case STYPE_SELF_ONLY :
   case STYPE_PEACEFUL :
    if( victim == NULL )
      victim = ch;
    break;

   case STYPE_OFFENSIVE :
    if( victim == NULL )
      return NULL;
    break;

   default :
    return NULL;
    }

  /*
  stop_active( ch );

  event = new cast_event( ch, victim );
  
  event->vo      = victim;
  event->spell   = *spell-SPELL_FIRST;
  event->prepare = FALSE;
  event->wait    = spell_table[*spell-SPELL_FIRST].prepare-1;
  ch->active     = event;

  has_reagents( ch, event );
  execute_cast( event );
  */

  return NULL;  
}


void* code_do_spell( void** argument )
{
  int          spell  = (int)        argument[0];
  char_data*      ch  = (char_data*) argument[1];
  char_data*  victim  = (char_data*) argument[2];

  if( ch == NULL )
    return NULL;

  if( victim == NULL )
    victim = ch;

  if( spell >= SPELL_FIRST && spell < WEAPON_UNARMED ) {
    if( !is_set( &spell_table[spell-SPELL_FIRST].usable_flag,
      STYPE_PROC_CAST ) ) {
      code_bug( "Do_Spell: Unsupported spell." );
      bug( "-- Spell = %s", spell_table[spell-SPELL_FIRST].name );
      return NULL;
      }
    ( spell_table[spell-SPELL_FIRST].function )
      ( ch, victim, NULL, 10, -1, STYPE_PROC_CAST );
    }

  return NULL;
}


void* code_num_in_room( void** argument )
{
  room_data*  room  = (room_data*) argument[0];
  int          num  = 0;

  if( room != NULL )
    for( int i = 0; i < room->contents; i++ )
      num += ( character( room->contents[i] ) != NULL );

  return (void*) num;
}  


void* code_players_area( void** argument )
{
  room_data*  room  = (room_data*) argument[0];

  if( room == NULL ) {
    code_bug( "Players_Area: NULL room." );
    return NULL;
    }

  return (void*) room->area->nplayer;
}  


void* code_players_room( void** argument )
{
  room_data*  room  = (room_data*) argument[0];
  int          num  = 0;  

  if( room != NULL )
    for( int i = 0; i < room->contents; i++ )
      num += ( player( room->contents[i] ) != NULL );

  return (void*) num;
}  


void* code_num_mob( void** argument )
{
  int         vnum  = (int)        argument[0];
  room_data*  room  = (room_data*) argument[1];
  mob_data*    rch;
  int          num  = 0;  

  if( room != NULL )
    for( int i = 0; i < room->contents; i++ )
      if( ( rch = mob( room->contents[i] ) ) != NULL
        && rch->species->vnum == vnum )
        num++;

  return (void*) num;
}  


void* code_transfer( void** argument )
{
  char_data*    ch  = (char_data*) argument[0];
  room_data*  room  = (room_data*) argument[1];

  if( room == NULL ) {
    send( "A script attempts to send you to a non-existent room??\n\r", ch );
    return NULL;
    }

  if( ch == NULL )
    return NULL;

  if( ch->rider != NULL )
    ch = ch->rider;

  ch->From( );
  ch->To( room );

  if( ch->mount != NULL ) {
    ch->mount->From( );
    ch->mount->To( room );
    }

  send( "\n\r", ch );
  do_look( ch, "" );

  return NULL;
}


void* code_transfer_all( void** argument )
{
  room_data*       from  = (room_data*) argument[0];
  room_data*         to  = (room_data*) argument[1];
  char_data*        rch;
  thing_array      list;

  if( to == NULL )
    return NULL;

  copy( list, from->contents );

  // Zeth, transfer all players, not characters.
  for( int i = 0; i < list; i++ )
    if( ( rch = player( list[i] ) ) != NULL ) {
      rch->From( );
      rch->To( to );
      }

  for( int i = 0; i < list; i++ )
    if( ( rch = character( list[i] ) ) != NULL ) {
      send( "\n\r", rch );
      show_room( rch, to, FALSE, "-n" );
      }

  return NULL;
}


void* code_mload( void** argument )
{
  int               vnum  = (int)        argument[0];
  room_data*        room  = (room_data*) argument[1];
  mob_data*          mob;
  species_data*  species;

  if( room == NULL || ( species = get_species( vnum ) ) == NULL ) {
    code_bug( "Mload: non-existent species or null room." );
    return NULL;
    }

  mob = create_mobile( species );
  mreset_mob( mob );
 
  mob->To( room );
  
  return mob;
}


void* code_rand_char( void** argument )
{
  room_data*  room  = (room_data*) argument[0];
  char_data*   rch;

  if( room == NULL )
    return NULL;

  rch = random_pers( room );
  
  return rch;
}

/* NEWER CODE AT THE BOTTOM
void* code_rand_player( void** argument )
{
  room_data*  room  = (room_data*) argument[0];
  char_data*   rch;

  if( room == NULL )
    return NULL;

  rch = rand_player( room );
  
  return rch;
}
*/
   
void* code_obj_in_room( void** argument )
{
  int         vnum  = (int)       argument[0];
  room_data*  room  = (room_data*) argument[1];

  if( room == NULL ) {
    code_bug( "Obj_in_room: NULL room." );
    return NULL;
    }

  return find_vnum( room->contents, vnum );
}


void* code_mob_in_room( void** argument )
{
  int         vnum  = (int)        argument[0];
  room_data*  room  = (room_data*) argument[1];
  char_data*   rch;

  if( room == NULL ) {
    code_bug( "Mob_in_Room: Null Room??" );
    return NULL;
    }

  for( int i = 0; i < room->contents; i++ )
    if( ( rch = mob( room->contents[i] ) ) != NULL
      && rch->species->vnum == vnum )
      return rch; 
  
  return NULL;
}


void* code_has_obj( void** argument )
{
  int        vnum  = (int)        argument[0];
  char_data*   ch  = (char_data*) argument[1];

  if( ch == NULL ) {
    code_bug( "Has_Obj: NULL character." );
    return NULL;
    }
 
  return find_vnum( ch->contents, vnum );
}


void* code_obj_to_room( void** argument )
{
  obj_data*    obj  = (obj_data*)  argument[0];
  room_data*  room  = (room_data*) argument[1];

  if( obj == NULL || room == NULL )
    return NULL;

  obj->To( room );
  consolidate( obj );
  
  return NULL;
}


void* code_obj_to_char( void** argument )
{
  obj_data*   obj  = (obj_data*)  argument[0];
  char_data*   ch  = (char_data*) argument[1];

  if( obj == NULL || ch == NULL )
    return NULL;
  
  set_owner( obj, ch, NULL );
  obj->To( ch );
  
  consolidate( obj );
  
  return obj;
}

   
void* code_coin_value( void** argument )
{
  obj_data*  obj  = (obj_data*) argument[0];
  int          i;

  if( obj == NULL || obj->pIndexData->item_type != ITEM_MONEY )  
    return NULL;

  for( i = 0; i < MAX_COIN; i++ ) 
    if( obj->pIndexData->vnum == coin_vnum[i] ) 
      return (void*) ( coin_value[i]*obj->number );

  return NULL;;
}


void* code_plague( void** argument )
{
  char_data*        ch  = (char_data*) argument[0];
  affect_data   affect;
 
  if( ch == NULL || ch->save_vs_poison( 5 ) )
    return NULL;

  if( var_mob != NULL && var_mob->species != NULL
    && !is_set( ch->affected_by, AFF_PLAGUE ) )
    var_mob->species->special += 20;

  affect.type      = AFF_PLAGUE;
  affect.duration  = 11;
  affect.level     = 0;
  affect.leech     = NULL;

  add_affect( ch, &affect );

  return NULL;
}


void* code_poison( void** argument )
{
  char_data*        ch  = (char_data*) argument[0];
  affect_data   affect;

  if( ch == NULL || ch->save_vs_poison( 5 ) )
    return NULL;

  if( var_mob != NULL && var_mob->species != NULL 
    && !is_set( ch->affected_by, AFF_POISON ) ) 
    var_mob->species->special += 10;

  affect.type      = AFF_POISON;
  affect.duration  = 10;
  affect.level     = 8;
  affect.leech     = NULL;

  add_affect( ch, &affect );

  return NULL;
}


void* code_heal( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  int i         = (int)        argument[1];

  if( ch != NULL ) {
    ch->hit += i;
    ch->hit = min( ch->hit, ch->max_hit );
    }

  return NULL;
}


void* code_modify_mana( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];
  int          i  = (int)        argument[1];

  if( ch == NULL || ch->mana+i < 0 ) 
    return (void*) FALSE;

  ch->mana += i;
  ch->mana = min( ch->mana, ch->max_mana );

  return (void*) TRUE;
}

void* code_raise_exp( void** argument )
{
  char_data *ch = (char_data*) argument[0];
  int i         = (int) argument[1];

 if( ch != NULL && ch->species == NULL )
   add_exp( ch, i, "You gain %d experience points.\n\r" );

 return NULL;
}

void* code_drain_exp( void** argument )
{
  char_data *ch = (char_data*) argument[0];
  int i         = (int)       argument[1];

  if( ch != NULL && ch->species == NULL ) 
    add_exp( ch, -i, "You lose %d experience points.\n\r" );

  return NULL;
}


/*
 *   FUNCTION CALLS
 */


void* code_acode( void** argument )
{
  room_data *room = (room_data*) argument[0];
  int i           = (int)        argument[1];

  action_data* action; 
  int               j  = 1;

  if( room == NULL ) {
    code_bug( "Code_acode: NULL room." );
    return NULL;
    }

  for( action = room->action; action != NULL; action = action->next ) 
    if( j++ == i ) {
      push( );
      execute( action );
      pop( );
      break;
      }

  if( action == NULL ) 
    code_bug( "Code_acode: NULL action." );

  return NULL;
}


void* code_mpcode( void** argument )
{
  char_data* mob = (char_data*) argument[0];
  int i          = (int)       argument[1];

  mprog_data* mprog;
  int             j  = 1;

  if( mob == NULL || mob->species == NULL ) {
    code_bug( "Code_mpcode: NULL mob or mob is a player." );
    return NULL;
    }

  for( mprog = mob->species->mprog; mprog != NULL; mprog = mprog->next ) 
    if( j++ == i ) {
      push( );
      execute( mprog );
      pop( );
      break;
      }

  if( mprog == NULL ) 
    code_bug( "Code_mpcode: NULL mprog." );

  return NULL;
}


void* code_opcode( void** argument )
{
  obj_data *obj = (obj_data*) argument[0];
  int i         = (int)       argument[1];

  oprog_data* oprog;
  int             j  = 1;

  if( obj == NULL ) {
    code_bug( "Opcode: NULL obj." );
    return NULL;
    }
 
  for( oprog = obj->pIndexData->oprog; oprog != NULL; oprog = oprog->next ) 
    if( j++ == i ) {
      push( );
      execute( oprog );
      pop( );
      break;
      }

  if( oprog == NULL )
    code_bug( "Opcode: NULL oprog." );

  return NULL;
}


void* code_wait( void** argument )
{
  int i = (int) argument[0];

  queue_data* queue = new queue_data;

  end_prog   = TRUE;
  queue_prog = TRUE;
  
  queue->time = i;

  queue->room = var_room;
  queue->mob  = var_mob;
  queue->ch   = var_ch;
  queue->obj  = var_obj;
  queue->i    = var_i;

  queue->arg  = curr_arg;
  queue->next = queue_list;
  queue_list  = queue;

  return NULL;
}


void* code_is_searching( void** argument )
{
  char_data*   ch  = (char_data*) argument[0];

  return (void*) ( ch != NULL && ch->pcdata != NULL
    && is_set( ch->pcdata->pfile->flags, PLR_SEARCHING ) );
}


void* code_is_follower( void** argument )
{
  char_data*   ch  = (char_data*) argument[0];

  return (void*) ( ch == NULL || is_set( &ch->status, STAT_FOLLOWER ) );
}
  

void* code_char_in_room( void** argument )
{
  char_data*    ch  = (char_data*) argument[0];
  room_data*  room  = (room_data*) argument[1]; 

  return (void*) ( ch != NULL && ch->in_room == room );
}


void* code_is_name( void** argument )
{
  char*       str  = (char*) argument[0];
  char*  namelist  = (char*) argument[1];

  if( str == NULL || namelist == NULL )
    return (void*) FALSE;

  return (void*) is_name( str, namelist );
}


void* code_set_religion( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];
  int          i  = (int)        argument[1];

  if( ch == NULL || ch->pcdata == NULL )
    return NULL;

  if( i < 0 || i >= MAX_RELIGION ) {
    code_bug( "Code_set_religion: religion value out of range." );
    return NULL;
    } 

  ch->pcdata->religion = i;

  return NULL;
}  


void* code_tell( void** argument )
{
  char_data*      ch  = (char_data*) argument[0];
  char_data*  victim  = (char_data*) argument[1];
  char*       string  = (char*)      argument[2];

  char      tmp  [ 3*MAX_STRING_LENGTH ];

  if( victim == NULL || ch == NULL || victim->pcdata == NULL )
    return NULL;

  act_print( tmp, string, victim, ch, NULL, NULL, NULL, NULL, victim );
  process_tell( ch, victim, tmp );

  return NULL;
}


void* code_obj_name( void** argument )
{
  obj_data* obj = (obj_data*) argument[0];

  if( obj == NULL )
    return NULL;

  return obj->singular;
} 


void* code_disarm( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];
  int          j  = (int)        argument[1];

  if( ch == NULL ) {
    code_bug( "Disarm: NULL character or level." );
    return NULL;
    }

  return (void*) ( ch->check_skill( SKILL_UNTRAP )
    && number_range( 0, 20 ) > j );
}


/*
 *   MOVEMENT ROUTINES
 */


void* code_modify_move( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];
  int          i  = (int)        argument[1];

  if( ch == NULL || ch->move+i < 0 ) 
    return (void*) FALSE;

  ch->move += i;
  ch->move  = min( ch->move, ch->max_move );
 
  return (void*) TRUE;
}


void* code_is_exhausted( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];
  int          i  = (int)        argument[1];

  if( ch == NULL )
    return (void*) TRUE;
  
  if( ch->move-i < 0 ) {
    send( ch, "You are too exhausted.\n\r" );
    return (void*) TRUE;
    }

  ch->move -= i;
 
  return (void*) FALSE;
}

/*                           *
 *   MISC OBJECT ROUTINES    *
 *                           *
 * Added by Zemus Dec 21     */

void* code_is_objtype( void** argument )
{
  char*       string  = (char*)      argument[0];
  obj_data*      obj  = (obj_data*)  argument[1];

  if( !strcasecmp( item_type_name[ obj->pIndexData->item_type ], string ) )
    { 
     return (void*) TRUE;
    }

  return (void*) FALSE;

}

void* code_sacrifice( void** argument )
{
  obj_data*      obj  = (obj_data*)  argument[0];
  char_data*      ch  = (char_data*) argument[1];
  player_data*    pc;

  if( ( pc = player( ch ) ) != NULL )
    { 
     pc->reputation.gold += obj->Cost( );

     if( obj->pIndexData->item_type == ITEM_CORPSE )
        pc->reputation.blood++;
    
     if( is_set( obj->extra_flags, OFLAG_MAGIC ) )
        pc->reputation.magic += 1+obj->Cost( )/1000;
    }

  obj->Extract( 1 );

  return NULL;
}

/* OFIND - ZEMUS ( Jan 29 ) */
void* code_ofind( void** argument )
{
 obj_data*      obj  = (obj_data*)  argument[0];
 
}

/*
 *    PUIOK  ---   25/12/1999
 *
 *   
 *    ADDED FUNCTIONS:
 *        void cond_act( string, ch, ch, obj );            -- conditional act
 *        (bool) int check_cond( string, ch, ch, obj );    -- stat compare/checker
 *        ch find_player( string );                        -- finds a player by name
 *        string get_name( ch );                           -- returns the name of a player
 *        string int_string( string, int, string );        -- converts int to string
 *        string cat_string( string, string, string, string ); -- concatnates strings
 *        (bool) int is_time_range( int, int, int )        -- returns wether the time is within range
 *
 *    
 *    POSSIBLE ERRORS: incorrect reference to pointers/structs, '0' < '9'?
 *              mixed up ch/vict and other targets
 */

/*
 *        --- ALL COMMENTS BELOW ARE OUTDATED ---
 */

/*
library.h

  cfunc code_cond_act;
  cfunc code_check_cond;

*/
/*
library.cc
const class cfunc_type cfunc_list []

  { "cond_act",        &code_cond_act,      nn, { st, ch, ch, ob } },
  { "check_cond",      &code_check_cond,    in, { st, ch, ch, ob } },

*/


void conditional_act( char* text, char_data* ch, char_data* vict, obj_data* obj );
bool check_conditional( char* text, char_data* ch, char_data* vict, obj_data* obj );
int to_value(char flag, char_data* target, int value);
int to_value(char flag, obj_data* target, int value);

/*

The first field, the string, takes three parts separated with a ';'(semicolon):
  Where to send the message, ONE of:
    'c'    ch;
    'v'    vict
    'r'    room the ch is in
    'R'    room the vict is in
    'a'    area the ch is in
    'A'    area the vict is in
    'g'    globally
    'G'    same as above
    
    In addition to ONE of the above, the same code as above can be used,
      as many as neccessary, with a '!' in front to mean, 'not for'.
    So "R!c!v" means the room the vict is in but not the ch or vict.

  The obj field was only added for completion sakes, it can't be the target
    of a message.
  Don't do anything stupid like "v!v", you WILL get unexpected results.

  The second part is a conditional text.  See check_conditional(...) below.
    Some note:
      The 'v' used in the condition is the player that may see the message.
      The 'c' is the same ch.
      'o' can be used for the object passed.
  
  The third part is the actual message;
  
cond_act( "where;condition;message", ch, vict, obj)

    DO NOT separate the where/condition/message section with spaces.

Examples:
    "r!v;Lv>Lc;Everyone in ch's room except vict if they're higher level than ch"
    =P =P Can't be bothered/


**The purpose of this is not so a message can be sent to multiple targets.
There is only one final target so do not do "Ar" and think the message
will be sent to the victim's area and the ch's room.  You may get unexpected
results


"Pv>4" should be added in the condition for 'rch->position > POS_SLEEPING',
  a standard in all acts

*/

void* code_cond_act( void** argument )
{
  char*      string  = (char*)      argument[0];
  char_data*     ch  = (char_data*) argument[1];
  char_data* victim  = (char_data*) argument[2];  
  obj_data*     obj  = (obj_data*)  argument[3];

  conditional_act( string, ch, victim, obj );
  
  return NULL;
}


void conditional_act( char* text, char_data* ch, char_data* vict, obj_data* obj )
{
  bool  reverse = 0, immortal_global = 0;
  char_data*    tch = NULL;
  bool  not_c = 0, not_v = 0, not_r = 0, not_a = 0;
  bool  not_A = 0, not_R = 0;
  int    to_max = 0;
  char  buffer;
  char  condition [ TWO_LINES ] = "";
  
  for( buffer = text[0]; buffer != '\0'; buffer = (++text)[0] )
  {
    if(buffer == ';')
      break;
    if(buffer == '!')
      reverse = 1;
    else {
      switch(buffer)
      {
        case 'r': not_r = reverse;
          if(ch == NULL) {
            code_bug( "Cond_act: Can't have 'r' without ch" );
            return;
          }
            if(!to_max) to_max = 1;
            break;
        case 'R': not_R = reverse;
          if(vict == NULL) {
            code_bug( "Cond_act: Can't have 'R' without ch2" );
            return;
          }
            if(!to_max) to_max = 1;
            break;
        case 'a': not_a = reverse;
           if(ch == NULL) {
            code_bug( "Cond_act: Can't have 'a' without ch" );
            return;
          }
            if(to_max < 2) to_max = 2;
            break;
        case 'A': not_A = reverse;
           if(vict == NULL) {
            code_bug( "Cond_act: Can't have 'A' without ch2" );
            return;
          }
            if(to_max < 2) to_max = 2;
            break;
        case 'g': to_max = 3;
          if(ch == NULL) {
            code_bug( "Cond_act: Can't have 'g' without ch" );
            return;
          }
          if(ch->in_room->area->status != AREA_OPEN)
            to_max = 2;
          else
          if(ch->in_room->area->status == AREA_IMMORTAL)
            immortal_global = TRUE;
          break;
        case 'G': to_max = 3;
            if(vict == NULL) {
            code_bug( "Cond_act: Can't have 'G' without ch2" );
            return;
          }
          if(ch->in_room->area->status != AREA_OPEN)
            to_max = 2;
          else
          if(ch->in_room->area->status == AREA_IMMORTAL)
            immortal_global = TRUE;
            break;
        case 'c': not_c = reverse;
            if(ch == NULL) {
            code_bug( "Cond_act: Can't have 'c' without ch" );
            return;
          }
            break;
        case 'v': not_v = reverse;
            if(ch == NULL) {
            code_bug( "Cond_act: Can't have 'v' without ch2" );
            return;
          }
            break;
        case ' ':
        case '=': break;
        default:
          code_bug( "Cond_act: Invalid location" );
          return;
            break;
      }
      
      if( !reverse ) {
        if( (buffer >= 'A' && buffer <= 'Z') || buffer == 'v' )
          tch = vict;
        else if( buffer >= 'a' && buffer <= 'z' )
          tch = ch;
      }
      else reverse = 0;
    }
  }
  
  if(tch == NULL) {
    code_bug( "Cond_act: no target!?" );
    return;
  }
  
  if(buffer == '\0') {
    code_bug( "Cond_act: no message!?" );
    return;
  } else
    ++text;
  
  
  for( buffer = text[0]; buffer != '\0' && buffer != ';'; buffer = (++text)[0] )
  {
    if(buffer != ' ')
      sprintf( condition, "%s%c", condition, buffer);
  }

  if(buffer == '\0') {
    code_bug( "Cond_act: no message!?" );
    return;
  } else
    ++text;
  
  if(to_max == 3)
  {
    char_data*   rch;

    for( int i = 0; i < player_list; i++ ) {
      rch = player_list[i];
      if( !rch->In_Game( ) )
        continue;
      if(!(immortal_global && rch->in_room->area->status == AREA_IMMORTAL))
      if(!( not_a && rch->in_room->area == ch->in_room->area ))
      if(!( not_A && rch->in_room->area == vict->in_room->area ))
      if(!( not_r && rch->in_room == ch->in_room ))
      if(!( not_R && rch->in_room== vict->in_room ))
      if(!( not_c && rch == ch ))
      if(!( not_v && rch == vict ))
        if(check_conditional( condition, ch, rch, obj ) )
          act( rch, text, ch, vict, obj );
    }
  }
  else
  if(to_max == 2)
  {
    room_data*  room;
    char_data*   rch;
    
    if( ( room = Room( tch->in_room ) ) == NULL )
      return;
    
    for( room = room->area->room_first; room != NULL; room = room->next ) 
      if(!( not_r && room == ch->in_room ))
      if(!( not_R && room == vict->in_room ))
        for( int i = 0; i < room->contents; i++ ) 
          if( ( rch = character( room->contents[i] ) ) != NULL )
            if(!( not_c && rch == ch ))
            if(!( not_v && rch == vict ))
              if(check_conditional( condition, ch, rch, obj ) )
                act( rch, text, ch, vict, obj );
  }
  else
  if(to_max == 1)
  {
    room_data*  room;
    char_data*   rch;

    if( ( room = Room( tch->in_room ) ) == NULL )
      return;
    
    for( int i = 0; i < room->contents; i++ ) 
      if( ( rch = character( room->contents[i] ) ) != NULL )
        if(!( not_c && rch == ch ))
        if(!( not_v && rch == vict ))
          if(check_conditional( condition, ch, rch, obj ) )
            act( rch, text, ch, vict, obj );
  }
  else
  if(to_max == 0)
    if(check_conditional( condition, ch, tch, obj ) )
      act( tch, text, ch, vict, obj );
  
  return;
}


/*
   Please refer below
*/

void* code_check_cond( void** argument )
{
  char*       string  = (char*)      argument[0];
  char_data*      ch  = (char_data*) argument[1];
  char_data*  victim  = (char_data*) argument[2];
  obj_data*      obj  = (obj_data*)  argument[3];
  
  return (void*) check_conditional( string, ch, victim, obj);
}

/*
  
  Expects a condition text "", char, victim, and obj
  
  char, victim, obj    may be NULL
  
  The default outcome is false in case of an error so use accordingly.
  
  condition text is comprised of:
      A possible UPPER case char signifying a flag (see to_value(...) below)
      A possible lower case char for signifying target (one of 'c', 'v', 'o'
        for char, victim, obj respectively) --must have if a flag is specified.
      
      A possible integer value, used in some cases.
      
      The above three combination is compared to another combination alike with
        the following possible operands:
          >    greater than
          <    less than
          =    equal to
          !    not equal to  -- DON'T use it for truth toggling
                    (don't think there's a need so won't add support)
          ]    greater or equal to
          [    less or equal to
      
      And you have yourself an operation.  You can whack multiple operations
        together with '&' or '|' for AND or OR.
              ###It might be backwards precidented..  **CHECK** (F&T|T)
  
  DON'T use spaces!
  
  Examples:
    
  
  
  
  Remember: If the operation is invalid, the default outcome is false.
*/

bool check_conditional( char* text, char_data* ch, char_data* vict, obj_data* obj )
{
  char  r_f = 0, r_t = 0, l_f = 0, l_t = 0;
  int    rvalue = -1, lvalue = -1, singular_t = 0;
  char  buffer;
  char  operand = '\0';
  
  if(text[0] == '\0')
    return TRUE;
  
  for( buffer = text[0]; buffer != '\0'; buffer = (++text)[0] )
  {
    if( buffer >= 'A' && buffer <= 'Z' )
      { !operand ? r_f = buffer : l_f = buffer; }
    else
    if(buffer == 'c' || buffer == 'v' || buffer == 'o')
      { !operand ? r_t = buffer : l_t = buffer; }
    else
    if( buffer >= '0' && buffer <= '9' )
    {
      if(!operand) {
        if(rvalue == -1 )
          rvalue = 0;
        rvalue = rvalue * 10 + (buffer - '0');
      }
      else {
        if(lvalue == -1 )
          lvalue = 0;
        lvalue = lvalue * 10 + (buffer - '0');
      }
    }
    else
    if(buffer == '<' || buffer == '>' || buffer == '=' ||
      buffer == '!' || buffer == ']' || buffer == '[')
      operand = buffer;
    else
    if(buffer == '&' || buffer == '|')
      break;
  }
  
  if(r_f == 'S')
  {
    if(!r_t || r_t == 'o')
      rvalue = -1;
    else {
      if(!l_t)
        if(ch != NULL)
          rvalue = ch->Can_See();
        else {
          code_bug( "Cond_check: NULL things cannot see!! ch/ch2 NULL when needed" );
          return FALSE;
        }
      else
      if(r_t == 'o')
        rvalue = 0;
      else
      {
        if(l_t == 'o' && obj != NULL)
        {
          rvalue = obj->Seen(r_t == 'c' ? ch : vict);
        }
        else if((l_t == 'c' && ch != NULL) || (l_t == 'v' && vict != NULL))
        {        
          rvalue = (l_t == 'c' ? ch : vict)->Seen(r_t == 'c' ? ch : vict);
        }
        else {
          code_bug( "Cond_check: Can see compares NULL -- ch/ch2/obj NULL when needed" );
          return FALSE;
        }
      }

      if(operand == '!')
        rvalue = !rvalue;
    }
  }
  else {
    if(r_t != 0) {
      if(r_t == 'o')
        rvalue = to_value(r_f, obj, rvalue);
      else
        rvalue = to_value(r_f, r_t == 'c' ? ch : vict, rvalue);
    }
    else if( r_f )
      goto notarget;
    if(rvalue == -1) singular_t = -1; else singular_t = 1;
    
    if(l_t != 0 && singular_t != -1) {
      if(l_t == 'o')
        lvalue = to_value(l_f, obj, lvalue);
      else
        lvalue = to_value(l_f, l_t == 'c' ? ch : vict, lvalue);
    }
    else if( l_f )
      goto notarget;
    if(lvalue == -1) singular_t = -1; else singular_t += 2;
    
    if(singular_t == 0)
    {
      if(rvalue != -1)
        singular_t = 1;
      if(lvalue != -1)
        singular_t += 2;
      
      if(singular_t == 0)
        rvalue = operand == '!' ? 0 : 1;
    }
    
    if(singular_t == 3)
      switch(operand)
      {
        case '=': rvalue = rvalue == lvalue;
            break;
        case '!': rvalue = rvalue != lvalue;
            break;
        case '<': rvalue = rvalue < lvalue;
            break;
        case '>': rvalue = rvalue > lvalue;
            break;
        case ']': rvalue = rvalue >= lvalue;
            break;
        case '[': rvalue = rvalue <= lvalue;
            break;
      }
    else if(singular_t == 1)
      rvalue = ( operand == '!' ? !rvalue : rvalue );
    else if(singular_t == 2)
      rvalue = ( operand == '!' ? !lvalue : lvalue );
    else if(singular_t == -1)
      rvalue = -1;  /* Could put code_bug() but there's already one in to_value() */
  }
    
  if(buffer == '&')
    return rvalue > 0 && check_conditional(++text, ch, vict, obj);
  else
  if(buffer == '|')
    return rvalue > 0 || check_conditional(++text, ch, vict, obj);
  else
    return rvalue > 0;
  
  return FALSE;
  
  {
    notarget:
  
    char tmp[50] = "Check_cond: No target:- r_f='";
  
    sprintf( tmp, "%s%c%s%c%s", tmp, r_f, "' l_f='", l_f, "'" );
    code_bug( tmp );
    return FALSE;
   }
}

/*
  
  -1 is a reserved error code, if a non-error value of negative one needs
  to be returned, please change check_conditional(...) appropriately.
  
  Char
    Flags       Value               Description
    C      1:hp,2:e,3:mv    current stat
    E      percent of      maximum energy
    H      percent of      maximum health
    L      n/a          level
    K      skillnumber      skilllevel
    P      n/a          position
    S      n/a          Sc=v c sees v  (can be Sc=Sv)
                    Sc!v v doesn't see v
                    Sc>Sv DOES'T mean c sees more than v
                    (don't use anything other than '=' or '!')
    X      n/a          sex
  Objects
    nothing yet
 */


int to_value(char flag, char_data* target, int value)
{
  if( target == NULL ) {
      code_bug( "Check_cond: NULL target" );
      return -1;
  }
  
  switch(flag)
  {
    case 'C': switch(value) {
            case -1:
            case 1:  return target->hit;
            case 2: return target->mana;
            case 3: return target->move;
            default: return -1;
        }
    case 'E': if(value < 0) value = 100;
        return (int) ((double) target->max_mana * (value / 100.00));
    case 'H': if(value < 0) value = 100;
        return (int) ((double) target->max_hit * (value / 100.00));
    case 'L': return target->shdata->level;
    case 'K': return target->get_skill( value );
    case 'P': return target->position;
/*    case 'S': return value;    reserved, dont' use  */
    case 'X': return target->sex;
    case 0: if(target->species == NULL) /*NEED TO FIX*/
          return ((player_data*) target)->pcdata->pfile->ident;
        else return 0;
        break;
    default:
      code_bug( "To_value: invalid option" );
      return -1;
      break;
  }

  return -1;
}

int to_value(char flag, obj_data* target, int value)
{
  #pragma unused (value)

  if( target == NULL ) {
      code_bug( "Check_cond: NULL target" );
      return -1;
  }

  switch(flag)
  {
    
    default:
      code_bug( "To_value: invalid option" );
      return -1;
      break;  
  }
  
  return -1;
}

void* code_find_player( void** argument )
{
  char*      string = (char*)      argument[0];
  char_data* ch     = (char_data*) argument[1];

  return (void*) one_character( ch, string, empty_string,
    (thing_array*) &player_list );
}

void* code_get_name( void** argument )
{
  char_data *ch     = (char_data*) argument[0];

  if( ch == NULL )
    return (void*) empty_string;
  
  return (void*) ( ch->species == NULL ? ch->descr->name : ch->descr->singular );
}

void* code_int_string( void** argument )
{
  int          i  = (int)        argument[0];
  char*   string1 = (char*)      argument[1];
  char*   string2 = (char*)      argument[2];

  char      tmp [ MAX_STRING_LENGTH ];

  if( sprintf( tmp, "%s%d%s", string1 ? string1 : "", i, string2 ? string2 : "" ) < 0)
    strcpy( tmp, "Error" );
  
  return (void *) tmp;
}

void* code_cat_string( void** argument )
{
  char*   string1 = (char*)      argument[0];
  char*   string2 = (char*)      argument[1];
  char*   string3 = (char*)      argument[2];
  char*   string4 = (char*)      argument[3];

  char      tmp [ MAX_STRING_LENGTH ];

  if( sprintf( tmp, "%s%s%s%s", string1 == NULL ? "" : string1,
    string2 == NULL ? "" : string2, string3 == NULL ? "" : string3,
    string4 == NULL ? "" : string4 ) < 0)
    strcpy( tmp, "Error" );
  
  return (void *) tmp;
}

void* code_is_time_range( void** argument )
{
  int        lower  = (int)        argument[0];
  int        upper  = (int)        argument[1];
  int         time  = (int)        argument[2];
  int minute = -1;
  
  if( time == 0 )
    time = weather.hour * 100 + weather.minute;
  else
  if( time < 100 )
    time *= 100;
  
  if( lower < 100 )
    lower *= 100;
  if( upper < 100 )
    upper *= 100;
  
  if( lower > upper )
  {
    if( time >= lower || time <= upper )
      return (void*) TRUE;
  }
  else
  if( time >= lower && time <= upper )
    return (void*) TRUE;
  
  return (void*) FALSE;
}

void* code_get_time( void** )
{
  return (void*) weather.hour;
}

void hear_bark( char_data*, char_data*, char*, int, int );

void* code_act_bark( void** argument )
{
  char*       string  = (char*)      argument[0];
  char_data*      ch  = (char_data*) argument[1];
  int          range  = (int)        argument[2];

  if( ch != NULL && ch->in_room != NULL )
    exec_range( ch, range, hear_bark, string );

  return NULL;
}

void hear_bark( char_data* victim, char_data* ch, char* message,
  int dir, int )
{
  if( victim->position < POS_RESTING )
    return;

  act( victim, message, ch, NULL, dir_table[dir].where );
}

void* code_with_coins( void** argument )
{
  int         amount  = (int)        argument[0];
  char_data*      ch  = (char_data*) argument[1];
  char*      message  = (char*)      argument[2];
  int         remove  = (int)        argument[3];

  if( ch == NULL )
    return (void*) TRUE;
  
  if(amount < 0)
  {
    amount *= -1;
    remove = TRUE;
  }
  
  if(remove)
    return (void*) remove_coins( ch, amount, message );
  else
    add_coins( ch, amount, message );
  
  return (void*) TRUE;
}

void* code_find_mob( void** argument )
{
  char*        string = (char*)      argument[0];
  char_data*      ch  = (char_data*) argument[1];

  return (void*) one_character( ch, string, empty_string,
    (thing_array*) &mob_list );
  
  return NULL;
}

void* code_show_room( void** argument )
{
  char_data*     ch   = (char_data*) argument[0];
  room_data*   room   = (room_data*) argument[1];
  char*    no_shows   = (char*)      argument[2];

  char*    defaults   = "-nsrhe";
  
  if( ch == NULL || room == NULL )
    return NULL;

  if( no_shows == NULL )
    no_shows = defaults;
  
  show_room( ch, room, FALSE, no_shows );
  
  return NULL;
}

void* code_get_vnum_room( void** argument )
{
  int           vnum  = (int) argument[0];
  
  return (void*) get_room_index( vnum, FALSE );
}


void* code_find_char( void** argument )
{
  char*        string = (char*)      argument[0];
  char_data*      ch  = (char_data*) argument[1];
    
  return (void*) one_character( ch, string, empty_string,
    (thing_array*) &player_list, (thing_array*) &mob_list );

  return NULL;
}

void* code_atoi( void** argument )
{
  char*      string   = (char*)  argument[0];
  
  return (void*) atoi( string );
}

void* code_can_move( void** argument )
{
  char_data*      ch  = (char_data*) argument[0];
  char*       string  = (char*)      argument[1];

  exit_data*    exit;
  
  if( ch == NULL || ch->in_room == NULL )
    return (void*) 0;

  if( ( exit = (exit_data*) one_thing( ch, string, empty_string, 
    (thing_array*) &ch->in_room->exits ) ) == NULL )
    return (void*) 0;
  
  if( ch->Can_Move( exit ) )
    return (void*) exit->to_room->vnum;
  
  return (void*) 0;
}

void* code_get_victim( void** argument )
{
  char_data*      ch  = (char_data*) argument[0];
  char*        string = (char*)      argument[1];
  char*           msg = (char*)      argument[2];
  
  if( msg == NULL )
    msg = empty_string;
    
  return (void*) get_victim( ch, string, msg );
}

void* code_set_mv( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  int value = (int) argument[1];
  if( ch == NULL )
  {
    code_bug( "set_mv: NULL char" );
    return (void*) FALSE;
  }
  ch->move = value;
  return NULL;
}


void* code_find_maxmv( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  if( ch == NULL )
  {
    code_bug( "find_maxmv: NULL char" );
    return (void*) FALSE;
  }
 
  return (void*) ch->max_move;
}

void* code_rand_player( void** argument )
{
  room_data*  room  = (room_data*) argument[0];
  int        scale  = (int)        argument[1];
  int        lower  = (int)        argument[2];
  int        upper  = (int)        argument[3];
  
  if( scale == 0 )
    scale = 1;
  
  return rand_player( room, scale, lower, upper );
}

extern path_data* make_path( room_data* start );

void* code_add_path( void** argument )
{
  char_data*        ch  = (char_data*) argument[0];
  char_data*  summoner  = (char_data*) argument[1];

  path_data*    path;
  event_data*  event;
  
  if( ch == NULL || summoner == NULL )
    return (void*) FALSE;
    
  if( ( path = make_path( ch->in_room ) ) == NULL )
    return (void*) FALSE;

  stop_events( ch, execute_path );
  stop_events( ch, execute_wander );

  event          = new event_data( execute_path, ch );
  event->pointer = (void*) path;  
  path->summoner = summoner; 

  add_queue( event, 50 );
  return (void*) TRUE;
}



/*
  --  END PUIOK
*/

/* has_mflag - zemus - april 14 */
void* code_has_mflag( void** argument )
{
  int             type  = (int) argument[0];
  int             flag  = (int) argument[1];
  char_data*        ch  = (char_data*) argument[2];

  if( ch == NULL ) {
    beetle( "has_mflag: null char" );
    return (void*) FALSE;
    }

  if( is_mob( ch ) ) {
   switch( type ) {

    case 0:  
      if( is_set( &ch->species->act_flags, flag ) )
        return (void*) TRUE;
      break;
 
    case 1: 
      beetle( "has_mflag: affect flag checks do not work" );
/*      if( is_set( &ch->species->affected_by, flag ) )
        return (void*) TRUE;
*/ 
     break;

    case 2:
      if( is_set( &ch->species->wear_part, flag ) )
        return (void*) TRUE;
      break;

    default:
      beetle( "has_mflag: invalid type" );
      break;

    }

    }
  else {
    }

 return (void*) FALSE;

}

void* code_get_leader( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  return (void*) ( ch == NULL ? NULL : ch->leader );
}
