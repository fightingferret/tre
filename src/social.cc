#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <syslog.h>
#include "define.h"
#include "struct.h"


/* 
 *   MAIN PRINT ROUTINE
 */


void act_social( char_data* to, const char* msg, char_data* ch,
  char_data* victim, obj_data* obj = NULL )
{
  bool heard  = ( *msg == '!' );

  if( heard )
    msg++;

  if( heard || ch->Seen( to )
    || ( victim != NULL && victim->Seen( to ) ) )
    act( to, msg, ch, victim, obj );
}


void act_social( char_data* to, const char* msg, char_data* ch,
  const char* word )
{
  bool heard  = ( *msg == '!' );

  if( heard )
    msg++;

  if( heard || ch->Seen( to ) )
    act( to, msg, ch, NULL, word );
}


void act_social( char_data* to, const char* msg, char_data* ch,
  obj_data* obj )
{
  bool heard  = ( *msg == '!' );

  if( heard )
    msg++;

  if( heard || ch->Seen( to ) )
    act( to, msg, ch, NULL, obj );
}


/*
 *   SOCIAL ACTING ON CHARACTER
 */


void social_no_arg( char_data* ch, social_type* soc1, social_type* soc2 )
{
  char        tmp  [ THREE_LINES ];
  char_data*  rch;

  if( soc1->char_no_arg == empty_string )
    soc1 = soc2;

  convert_to_ansi( ch, soc1->char_no_arg, tmp );
  send( ch, "%s\n\r", tmp );

  for( int i = 0; i < *ch->array; i++ )
    if( ( rch = character( ch->array->list[i] ) ) != NULL
      && rch != ch && rch->position > POS_SLEEPING )
      act_social( rch, soc1->others_no_arg, ch,
        (char_data*) NULL );
}


void social_auto( char_data* ch, social_type* soc1, social_type* soc2 )
{
  char        tmp  [ THREE_LINES ];
  char_data*  rch;

  if( soc1->char_auto == empty_string )
    soc1 = soc2;

  convert_to_ansi( ch, soc1->char_auto, tmp );
  send( ch, "%s\n\r", tmp );

  for( int i = 0; i < *ch->array; i++ )
    if( ( rch = character( ch->array->list[i] ) ) != NULL
      && rch != ch && rch->position > POS_SLEEPING )
      act_social( rch, soc1->others_auto, ch, (char_data*) NULL );
}


void social_victim( char_data* ch, social_type* soc1,
  social_type* soc2, char_data* victim )
{
  char_data* rch;
 
  if( soc1->char_found == empty_string ) {
    if( soc2->char_found == empty_string ) {
      send( ch, "%s and %s do nothing together.\n\r",
        soc1->name, victim );
      return;
      }
    soc1 = soc2;
    }

  act( ch, soc1->char_found, ch, victim );
  act_social( victim, victim->position == POS_SLEEPING ?
    soc1->vict_sleep : soc1->vict_found, ch, victim );

  for( int i = 0; i < *ch->array; i++ )
    if( ( rch = character( ch->array->list[i] ) ) != NULL
      && rch != ch && rch != victim && rch->position > POS_SLEEPING )
      act_social( rch, soc1->others_found, ch, victim );
}


/*
 *   DIRECTION AND SOCIAL
 */


void social_dir( char_data* ch, social_type* soc1, social_type* soc2,
  char* directions )
{
  char_data* rch;

  if( soc1->dir_self == empty_string ) {
    if( soc2->dir_self == empty_string ) {
      send( ch, "%s and a direction does nothing.\n\r", soc1->name );
      return;
      }
    soc1 = soc2;
    }

/*  act( ch, soc1->dir_self, ch, NULL, dir_table[dir].name );  PUIOK 18/2/2000 */
  act( ch, soc1->dir_self, ch, NULL, directions );

  for( int i = 0; i < *ch->array; i++ )
    if( ( rch = character( ch->array->list[i] ) ) != NULL
      && rch != ch && rch->position > POS_SLEEPING )
      act_social( rch, soc1->dir_others, ch, directions );  /* PUIOK 18/2/2000 */
}


/*
 *   OBJECT AND SOCIAL
 */


void social_obj( char_data* ch, social_type* soc1, social_type* soc2,
  obj_data* obj )
{
  char_data* rch;

  if( soc1->obj_self == empty_string ) {
    if( soc2->obj_self == empty_string ) {
      send( ch, "%s and an object does nothing.\n\r", soc2->name );
      return;
      }
    soc1 = soc2;
    }

  act( ch, soc1->obj_self, ch, NULL, obj );

  for( int i = 0; i < *ch->array; i++ )
    if( ( rch = character( ch->array->list[i] ) ) != NULL
      && rch != ch && rch->position > POS_SLEEPING )
      act_social( rch, soc1->obj_others, ch, obj );
}


/*
 *   OBJECT, VICTIM AND SOCIAL
 */


void social_self_obj( char_data* ch, social_type* soc1, social_type* soc2,
  obj_data* obj )
{
  char_data* rch;

  if( soc1->self_obj_self == empty_string ) {
    if( soc2->self_obj_self == empty_string ) {
      fsend( ch, "Mixing %s and %s does nothing interesting.\n\r",
        soc2->name, obj );
      return;
      }
    soc1 = soc2;
    }
  act( ch, soc1->self_obj_self, ch, NULL, obj );
  
  for( int i = 0; i < *ch->array; i++ )
    if( ( rch = character( ch->array->list[i] ) ) != NULL
      && rch != ch && rch->position > POS_SLEEPING )
      act_social( rch, soc1->self_obj_others, ch, obj );
}


void social_ch_obj( char_data* ch, social_type* soc1, social_type* soc2,
  char_data* victim, obj_data* obj )
{
  char_data* rch;

  if( ch == victim ) {
    social_self_obj( ch, soc1, soc2, obj );
    return;
    }

  if( soc1->ch_obj_self == empty_string ) {
    if( soc2->ch_obj_self == empty_string ) {
      fsend( ch, "Mixing %s, %s and %s does nothing interesting.\n\r",
        soc2->name, victim, obj );
      return;
      }
    soc1 = soc2;
    }

  act( ch, soc1->ch_obj_self, ch, victim, obj );

  if( victim->position >= POS_SLEEPING ) {
    act( victim, victim->position == POS_SLEEPING
      ? soc1->ch_obj_sleep : soc1->ch_obj_victim,
      ch, victim, obj );
    }

  for( int i = 0; i < *ch->array; i++ )
    if( ( rch = character( ch->array->list[i] ) ) != NULL
      && rch != ch && rch != victim && rch->position > POS_SLEEPING )
      act_social( rch, soc1->ch_obj_others, ch, victim, obj );
}


social_type* find_social( social_type* table, int max, char* command )
{
  int cmd;

  if( max == 0 )
    return NULL;

  if( ( cmd = search( table, max, command ) ) < 0 )
    cmd = -cmd-1;

  if( cmd == max
    || strncasecmp( command, table[cmd].name, strlen( command ) ) )
    return NULL;

  return &table[cmd];
}


/*
 *   MAIN HANDLER
 */


bool check_social( char_data* ch, char* command, char* argument )
{
  char             arg  [ MAX_INPUT_LENGTH ];
  social_type*    soc1  = NULL;
  social_type*    soc2  = NULL;
  char_data*    victim;
  obj_data*        obj;
  exit_data*      exit;
  thing_data*       t1;
  thing_data*       t2;

  if( ch->shdata->race < MAX_PLYR_RACE )
    soc1 = find_social( social_table[ ch->shdata->race+1 ],
      table_max[ ch->shdata->race+1 ], command );

  soc2 = find_social( social_table[0], 
    table_max[TABLE_SOC_DEFAULT], command );

  if( soc1 == NULL && ( soc1 = soc2 ) == NULL )
    return FALSE;

  if( soc2 == NULL )
    soc2 = soc1;

  if( ch->pcdata != NULL 
    && is_set( ch->pcdata->pfile->flags, PLR_NO_EMOTE ) ) {
    send( ch, "You are anti-social!\n\r" );
    return TRUE;
    }

  if( ch->position < soc1->position ) {
    switch( ch->position ) {
      case POS_DEAD:
        send( ch, "You have died and are unable to move.\n\r" );
        return TRUE;

      case POS_INCAP:
      case POS_MORTAL:
        send( ch, "The bright white light has you distracted.\n\r" );
        return TRUE;

      case POS_STUNNED:
        send( ch, "You are stunned and cannot move.\n\r" );
        return TRUE;
 
      case POS_MEDITATING:
        send( ch, "You are deep in meditation.\n\r" );
        return TRUE;

      case POS_SLEEPING:
        send( "You cannot do that while sleeping.\n\r", ch );
        return TRUE;
 
      case POS_RESTING:
        send( "You must be standing to do that.\n\r", ch );
        return TRUE;
      }
    }
  
  if( soc1->position > POS_FIGHTING && ch->fighting != NULL ) {
    send( "You are unable to do that while fighting.\n\r", ch );
    return TRUE;
  }
  
  if( *argument == '\0' ) {
    social_no_arg( ch, soc1, soc2 );
    return TRUE;
    }

  argument = one_argument( argument, arg );
  
  /* -- PUIOK 18/2/2000 */
  
  for( int i = 0; i < MAX_DOOR; i++ )
    if( !strncasecmp( arg, dir_table[i].name, strlen( arg ) ) ) {
      char directions [ ONE_LINE ];
      
      strcpy( directions, dir_table[i].name );
      while( *argument != '\0' ) {
        int dir = -1;

        argument = one_argument( argument, arg );
        for( int j = 0; j < MAX_DOOR; j++ )
          if( !strncasecmp( arg, dir_table[j].name, strlen( arg ) ) ) {
            dir = j;
            break;
          }
        if( dir != -1 )
        {
          if( strlen( directions ) +
            strlen( dir_table[dir].name ) + 4 >= ONE_LINE )
            break;
          sprintf( directions, "%s%s %s", directions,
            *argument == '\0' ? " and" : ",", dir_table[dir].name );
        }
        else
        {
          send( ch, "No direction found matching '%s'.\n\r", arg );
          return TRUE;
        }
      }
      social_dir( ch, soc1, soc2, directions );
      return TRUE;
    }
  
  /* -- END PUIOK */

  if( ( t1 = one_thing( ch, arg, "social", ch->array, &ch->contents, NULL,
    NULL, ( visible_data* ( * ) ( visible_data* ) ) character ) ) == NULL )
    return TRUE;

  obj    = object( t1 );
  victim = character( t1 );
  
  if( *argument == '\0' ) {
    if( obj != NULL )
      social_obj( ch, soc1, soc2, obj );
    else if( victim == ch )
      social_auto( ch, soc1, soc2 );
    else
      social_victim( ch, soc1, soc2, victim );
    return TRUE;
    }
  
  if( ( t2 = one_thing( ch, argument, "social", &ch->contents, ch->array,
    NULL, NULL, obj == NULL ? ( visible_data* ( * ) ( visible_data* ) ) object
    : ( visible_data* ( * ) ( visible_data* ) ) character ) ) == NULL )
    return TRUE;

  if( obj == NULL )
    obj = object( t2 );
  else
    victim = character( t2 );

  if( obj != NULL && victim != NULL ) 
    social_ch_obj( ch, soc1, soc2, victim, obj );
  else 
    fsend( ch, "Mixing %s, %s, and %s does nothing interesting.",
      soc1->name, t1, t2 );
 
  return TRUE;
}


int get_social( char_data* ch, char* command, char* argument,
  char*& self_message, char*& victim_message, char*& other_message,
  char_data*& vict, obj_data*& obj1, bool message, char* directions,
  thing_array* a1, thing_array* a2, thing_array* a3 )
{
  char             arg  [ MAX_INPUT_LENGTH ];
  social_type*    soc1  = NULL;
  social_type*    soc2  = NULL;
  social_type*  social  = NULL;
  char_data*    victim;
  obj_data*        obj;
  exit_data*      exit;
  thing_data*       t1;
  thing_data*       t2;
  
  self_message   = empty_string;
  victim_message = empty_string;
  other_message  = empty_string;
  vict   = NULL;
  obj1   = NULL;
  
  if( strlen( command ) < 3 )
    return FALSE;
  
  if( ch->shdata->race < MAX_PLYR_RACE )
    soc1 = find_social( social_table[ ch->shdata->race+1 ],
      table_max[ ch->shdata->race+1 ], command );

  soc2 = find_social( social_table[0], 
    table_max[TABLE_SOC_DEFAULT], command );

  if( soc1 == NULL && ( soc1 = soc2 ) == NULL )
    return FALSE;

  if( soc2 == NULL )
    soc2 = soc1;

  if( ch->pcdata != NULL 
    && is_set( ch->pcdata->pfile->flags, PLR_NO_EMOTE ) ) {
    send( ch, "You are anti-social!\n\r" );
    return -1;
    }

  if( *argument == '\0' ) {
    for( social = soc1; social->char_no_arg == empty_string; social = soc2 )
      if( social == soc2 )
        return FALSE;
    
    self_message = social->char_no_arg;
    other_message = social->others_no_arg;
    return TRUE;
  }

  argument = one_argument( argument, arg );

/*    
  for( int i = 0; i < MAX_DOOR; i++ )
    if( !strncasecmp( arg, dir_table[i].name, strlen( arg ) ) ) {
      char directions [ ONE_LINE ];
      
      strcpy( directions, dir_table[i].name );
      while( *argument != '\0' ) {
        int dir = -1;

        argument = one_argument( argument, arg );
        for( int j = 0; j < MAX_DOOR; j++ )
          if( !strncasecmp( arg, dir_table[j].name, strlen( arg ) ) ) {
            dir = j;
            break;
          }
        if( dir != -1 )
        {
          if( strlen( directions ) +
            strlen( dir_table[dir].name ) + 4 >= ONE_LINE )
            break;
          sprintf( directions, "%s%s %s", directions,
            *argument == '\0' ? " and" : ",", dir_table[dir].name );
        }
        else
        {
          send( ch, "No direction found matching '%s'.\n\r", arg );
          return TRUE;
        }
      }
      social_dir( ch, soc1, soc2, directions );
      return TRUE;
    }
*/

  if( ( t1 = one_thing( ch, arg,
    ( char* ) ( message ? "social" : empty_string ), a1, a2, a3 ) ) == NULL )
    if( message )
      return -1;
    else
      return FALSE;

  obj    = object( t1 );
  victim = character( t1 );

  if( *argument == '\0' ) {
    if( obj != NULL )
    {
      for( social = soc1; social->obj_self == empty_string; social = soc2 )
        if( social == soc2 )
          if( message )
          {
            send( ch, "%s and an object does nothing.\n\r", social->name );
            return -1;
          }
          else
            return FALSE;
      
      obj1 = obj;

      self_message = social->obj_self;
      other_message = social->obj_others;
    }
    else if( victim == ch )
    {
      for( social = soc1; social->char_auto == empty_string; social = soc2 )
        if( social == soc2 )
          return FALSE;
      
      self_message = social->char_auto;
      other_message = social->others_auto;      
    }
    else
    {
      for( social = soc1; social->char_found == empty_string; social = soc2 )
        if( social == soc2 )
          if( message )
          {
            send( ch, "%s and %s do nothing together.\n\r", social->name,
              victim );
            return -1;
          }
          else
            return FALSE;
      
      vict = victim;
      
      self_message = social->char_found;
      victim_message = social->vict_found;
      other_message = social->others_found;
    }
    
    return TRUE;
  }
  
  argument = one_argument( argument, arg );
  
  if( *argument != '\0' && !message )
    return FALSE;
  
  if( obj == NULL ) {
    obj = one_object( ch, arg,
      ( char* ) ( message ? "mix this social with" : empty_string ),
      a1, a2, a3 );
  }
  else
  {
    victim = one_character( ch, arg,
      ( char* ) ( message ? "mix this social with" : empty_string ),
      a1, a2, a3 );
  }

  if( obj != NULL && victim != NULL )
  {
    if( ch == victim )
    {
      for( social = soc1;
        social->self_obj_self == empty_string; social = soc2 )
        if( social == soc2 )
          if( message ) {
            fsend( ch, "Mixing %s and %s does nothing interesting.\n\r",
              social->name, obj );
            return -1;
          }
          else
            return FALSE;

      obj1 = obj;
      
      self_message = social->self_obj_self;
      other_message = social->self_obj_others;      
      return TRUE;
    }
    
    for( social = soc1; social->ch_obj_self == empty_string; social = soc2 )
      if( social == soc2 )
        if( message ) {
          fsend( ch, "Mixing %s, %s and %s does nothing interesting.\n\r",
            social->name, victim, obj );
          return -1;
        }
        else
          return FALSE;
    
    obj1 = obj;
    vict = victim;
    
    self_message = social->ch_obj_self;
    victim_message = social->ch_obj_victim;
    other_message = social->ch_obj_others;      
    return TRUE;
  }
  else
  {
    if( message )
      return -1;
    return FALSE;
  }
  
  return TRUE;
}



/* 
 *   DISPLAY SOCIAL LIST
 */


void do_socials( char_data* ch, char* argument )
{
  int                 i;
  int             table  = TABLE_SOC_DEFAULT;

  if( *argument == '\0' ) {
    page_title( ch, "Default Socials" );
    }
  else {
    for( table = 0; ; table++ ) {
      if( table == MAX_PLYR_RACE ) {
        send( ch, "Syntax: Social [race]\n\r" );
        return;
        }
      if( matches( argument, plyr_race_table[table].name ) )
        break;
      }
    page_title( ch, "%s Socials", plyr_race_table[table].name );
    table++;
    }

  for( i = 0; i < table_max[table]; ) {
    page( ch, "%16s ", social_table[table][i].name );
    if( ++i%4 == 0 )
      page( ch, "\n\r" );
    }
  if( i%4 != 0 )
    page( ch, "\n\r" );
}


void do_csocial( char_data* ch, char* )
{
return;
}
