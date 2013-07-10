#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "define.h"
#include "struct.h"
#include "unistd.h"

/* New Titles/Permissions - Zemus Jan 4- March 10 */
const char* imm_title [] =
 { "Avatar", "Apprentice",  "Builder",    "Designer",  "Architect",
   "Artisan",     "Creator",
   "Masonic",     "Wizard",  
   "Immortal",    "Spirit",     "Angel",      "Demigod", "God",
   "Greater God", "Sub Entity", "Entity" };

const char* permission_name [ MAX_PERMISSION+1 ] =
 { "none", "all_mobs", "all_objects", "all_rooms", "approve",
  "basic", "build_chan", "commands", "echo", "god_chan",
  "goto", "help files", "imm_chan",
  "lists", "load_objects", "misc.tables", "mobs", "noteboard",
  "objects", "players", "quests",
  "reimb_exp", "reimb_equip", "rooms", "shutdown", "site.names",
  "snoop", "spells",
  "socials", "transfer", "unfinished", "write.all", "write.areas",
  "avatar.Chan", "clans", "rtables", "disabled", "force.players",
  "code_chan", "admin", "perm.flags", "all.flags", "news.moderate"
};


/* End Changes */

void   monk_return   ( char_data* );
void   restore       ( char_data*, char_data* );


/*
 *   PERMISSION ROUTINES
 */


bool mortal( char_data* ch, char* command )
{
  if( ch->shdata->level >= LEVEL_APPRENTICE ) 
    return FALSE;

  send( ch,
    "To prevent abuse you are unable to use %s in mortal form.\n\r",
    command );

  return TRUE;
}


bool has_permission( char_data* ch, int* flag, bool msg )
{
  wizard_data* imm;
  int               i;

  if( flag[0] == 0 && flag[1] == 0 )
    return TRUE;

  if( ch == NULL || ( imm = wizard( ch ) ) == NULL )
    return FALSE;

  if( ch->shdata->level == 100 )
    return TRUE;

  for( i = 0; i < 2; i++ )
    if( ( imm->permission[i] & flag[i] ) != 0 )
      return TRUE;

  if( msg ) 
    send( ch, "Permission denied.\n\r." );

  return FALSE;
}


/*
 *   FIND LOCATION
 */


room_data* find_location( char_data* ch, char* argument )
{
  char_data* victim;

  if( is_number( argument ) )
    return get_room_index( atoi( argument ), FALSE );

  if( ( victim = one_character( ch, argument, "goto",
    (thing_array*) &player_list,
    (thing_array*) &mob_list ) ) != NULL )
    return victim->in_room;

  return NULL;
}


/*
 *   TRUST
 */

 
void do_trust( char_data *ch, char *argument )
{
  char           arg  [ MAX_INPUT_LENGTH ];
  char_data*  victim;
  int          level;

  argument = one_argument( argument, arg );

  if( *arg == '\0' || *argument == '\0' || !is_number( argument ) ) {
    send( ch, "Syntax: trust <char> <level>.\n\r" );
    return;
    }
  
  if( ( victim = one_player( ch, arg, "trust",
    (thing_array*) &player_list ) ) == NULL )
    return;

  if( victim->pcdata == NULL ) {
    send( ch, "%s is not a player.\n\r", victim );
    return;
    }
  
  if( ( level = atoi( argument ) ) < 0 || level > MAX_LEVEL ) {
    send( ch, "Level must be 0 (reset) or 1 to 216.\n\r" );
    return;
    }
  
  if( level > get_trust( ch ) ) {
    send( ch, "Limited to your trust.\n\r" );
    return;
    }
  
  if( victim != ch ) {
    if( !has_permission( ch, PERM_PERMFLAGS ) )
    {
      send( ch, "You may not change the trust of others.\n\r" );
      return;
    }
    if( get_trust( victim ) >= get_trust( ch ) )
    {
      send( ch, "You may only change the trust of those with less trust.\n\r" );
      return;
    }
  }
  else if( level < get_trust( ch ) ) {
    send( ch, "To save us all headaches you may not demote yourself.\n\r" );
    return;
  }
  
  victim->pcdata->trust = level;
  
  send( victim, "You are now trusted at level %d.\n\r", level );

  if( ch != victim ) 
    send( ch, "%s is now trusted at level %d.\n\r",
      victim->real_name( ), level );

  return;
}


/*
 *   REIMB FUNCTION
 */


void do_reimburse( char_data* ch, char* argument )
{
  char_data*     victim;
  link_data        link;
  thing_array*    array;
  obj_data*         obj;
  char*             tmp;
  int             flags;
  int               exp;

  if( mortal( ch, "reimburse" ) )
    return;

  if( !get_flags( ch, argument, &flags, "del", "reimburse" ) )
    return;

  if( ( victim = one_player( ch, argument, "reimburse",
    ch->array ) ) == NULL )
    return;

  if( victim == ch ) {
    send( ch, "You can't reimburse yourself.\n\r" );
    return;
    }

  if( flags == 0 ) {
    send( ch, "You must specify what to reimburse, see help.\n\r" );
    return;
    }

  if( is_set( &flags, 0 ) ) {
    exp = exp_for_level( victim )/5;
    send( ch, "You reimburse %s for %d exp and one death.\n\r",
      victim, exp );
    send( victim, "%s reimburses you for %d exp and one death.\n\r",
      ch, exp );

    tmp = static_string( );
    sprintf( tmp, "Reimbursed 1 death, %d exp by %s.",
      exp, ch->real_name( ) );
    player_log( victim, tmp );

    victim->shdata->deaths--;
    add_exp( victim, exp );
    return;
    }

  if( !load_char( &link, victim->descr->name, BACKUP_DIR ) ) {
    send( ch, "No backup of that player found.\n\r" );
    return;
    }

  array = ( is_set( &flags, 1 ) ? &link.player->contents 
    : &link.player->locker );

  if( is_empty( *array ) ) {
    send( ch, "The backup file shows such no equipment.\n\r" );
    }
  else {   
    for( int i = 0; i < *array; i++ ) {
      if( ( obj = object( array->list[i] ) ) == NULL )
        continue;
      obj->From( obj->number );
      obj->To( ch->array );
      consolidate( obj );
      }

    send( ch, "You scan the annals of history ...\n\r" );
    fsend( ch, "Finding the status of %s from earlier times you
 duplicate %s equipment, dropping it to the ground.",
      victim, victim->His_Her( ) );

    send( victim, "You feel %s probing your history ...\n\r", ch );
    send( victim,
      "With a sweep of %s arms %s produces your old equipment.\n\r",
      ch->His_Her( victim ), ch->He_She( victim ) );
    }
  
  link.character->Extract( );
}


/*
 *   ROUTINES TO DEAL WITH MALCONTENTS
 */


void do_deny( char_data *ch, char *argument )
{
  char_data*  victim;
  FILE*           fp;
  int          flags;
  pfile_data*  pfile;

  if( !get_flags( ch, argument, &flags, "ul", "Deny" ) )
    return;
 
  if( is_set( &flags, 0 ) ) {
   
    pfile = find_pfile( argument );
 
    if( pfile == NULL )
      return;
 
    remove_bit( pfile->flags, PLR_DENY );    

    write( find_player( pfile ) );

    send( ch, "Deny flag for %s removed.\n\r", pfile->name );
    return;  
  }

  if( is_set( &flags, 1 ) ) {
    page_centered( ch, "Denied Characters" );

    for( int i = 0; i < max_pfile; i++ ) {
       if( pfile_list[i] != NULL )
         if( is_set( pfile_list[i]->flags, PLR_DENY ) )
           page( ch, "%s\n\r", pfile_list[i]->name );
       }

    return;
  }
  if( ( victim = one_player( ch, argument, "deny",
    (thing_array*) &player_list ) ) == NULL )
    return;

  if( get_trust( victim ) >= get_trust( ch ) ) {
    send( ch, "You cannot deny those of a higher trust.\n\r" );
    return;
    }
  
  set_bit( victim->pcdata->pfile->flags, PLR_DENY );

  send( victim, "Your character is now denyed access.\n\r" );
  send( ch, "You have denied %s.\n\r", victim->descr->name );

  /* Logging Added */
  fp = open_file( FILE_DIR, "iplog.txt", "a" );
  if( fp != NULL )
  {
    fprintf( fp, "%s at %s denied by %s at %s at %s",
      victim->descr->name, victim->link->host, ch->descr->name,
      ch->link->host, (char*) ctime( &current_time ) );
     fclose( fp );
  }

  do_quit( victim, "" );
  return;
}


void do_disconnect( char_data *ch, char *argument )
{
  char_data *victim;

  if( ( victim = one_player( ch, argument, "disconnect",
    (thing_array*) &player_list ) ) == NULL )
    return;

  if( get_trust( victim ) >= get_trust( ch ) ) {
    send( ch, "You can only disconnect those of lower level.\n\r" );
    return;
    }
    
  if( victim->link == NULL ) {
    send( ch, "%s doesn't have a descriptor.\n\r", victim );
    return;
    }

  if( is_set( &victim->status, STAT_CLONED ) )
    purge_clone( player( victim ) );
  else
    close_socket( victim->link );
  send( ch, "Ok.\n\r" );

  return;
}


void do_imprison( char_data *ch, char *argument )
{
  char_data *victim;
     
  if( ( victim = one_player( ch, argument, "imprison",
    (thing_array*) &player_list ) ) == NULL )
    return;

  if( victim->in_room == NULL ) {
    send( ch, "They are in limbo.\n\r" );
    return;
    }

  if( get_trust( victim ) >= get_trust( ch ) ) {
    send( ch, "You can't imprison someone of equal or higher level.\n\r" );
    return;
    }

  send_seen( victim, "%s is mercilessly vaporized.\n\r", victim );

  victim->From( );
  victim->To( get_room_index( ROOM_PRISON, FALSE ) );
  
  if( is_set( &victim->status, STAT_CLONED )
    && victim->link->character == victim )
    do_return( victim, "" );
  
  set_bit( victim->pcdata->pfile->flags, PLR_FREEZE );

  send_seen( victim, "%s joins you in your pennance.\n\r", victim );
  send( victim, "You have been imprisoned!\n\r" );
  show_room( victim, victim->in_room, FALSE, "-n" );
  send( ch, "%s imprisoned.\n\r", victim );

  return;
}


void do_pardon( char_data* ch, char* argument )
{
  /*
  char_data *victim;
  int i;

  if( ( victim = one_player( ch, argument, "pardon",
    (thing_array*) &player_list ) ) == NULL )
    return;

  if( get_trust( victim ) > get_trust( ch ) ) {
    send(
      "You can only pardon those of lower or equal level.\n\r", ch );
    return;
    }
    
  if( victim->species != NULL ) {
    send( ch, "Mobs don't need pardoning!\n\r" );
    return;
    }

  for( i = 0; i < MAX_ENTRY_NATION; i++ )
    victim->pcdata->pfile->reputation[i] = 500;

  if( ch != victim )
    send( victim, "%s has pardoned you.\n\r", ch );
  
  send( ch, "Ok.\n\r" );
  */
}


void do_freeze( char_data *ch, char *argument )
{
  player_data*  victim;
  int            force = -1;
  
  extern bool all_programs_disabled;
  
  if( *argument == '-' || *argument == '+' )
  {
    force = ( *argument == '+' );
    *argument++;
  }
  
  if( is_god( ch ) && exact_match( argument, "code" ) )
  {
    FILE* fp;
    if( all_programs_disabled == force )
    {
      if( force )
        send( ch, "Programs are already FROZEN.\n\r" );
      else
        send( ch, "Programs are not frozen.\n\r" );
      return;
    }
    else
      all_programs_disabled = !all_programs_disabled;
    
    if( all_programs_disabled )
    {
      send( ch, "Programs FROZEN.(does not stop ones already running)\n\r" );
      if( ( fp = fopen( "olp_frozen", "w" ) ) == NULL )
        send( ch, "Could not write file \"olp_frozen\".\n\r" );
      else
        fclose( fp );
    }
    else
    {
      send( ch, "Programs FREEZE removed.\n\r" );
      if( remove( "olp_frozen" ) )
        send( ch, "Could not remove file \"olp_frozen\", maybe it didn't exist?\n\r" );
    }
    return;
  }
  
  if( ( victim = one_player( ch, argument, "pardon",
    (thing_array*) &player_list ) ) == NULL )
    return;

  if( get_trust( ch ) <= get_trust( victim ) ) {
    send( ch, "You failed.\n\r" );
    return;
    }
  
  if( is_set( victim->pcdata->pfile->flags, PLR_FREEZE ) == force )
  {
    if( force )
      send( ch, "It is already set.\n\r" );
    else
      send( ch, "It is not even set.\n\r" );
    return;
  }
  
  if( is_set( &victim->status, STAT_CLONED )
    && victim->link->character == victim )
    do_return( victim, "" );
  
  switch_bit( victim->pcdata->pfile->flags, PLR_FREEZE );

  if( is_set( victim->pcdata->pfile->flags, PLR_FREEZE ) ) {
    send( victim, "You can't do ANYthing!\n\r" );
    send( ch, "FREEZE set.\n\r" );
    }
  else {
    send( victim, "You can play again.\n\r" );
    send( ch, "FREEZE removed.\n\r" );
    }

  write( victim );

  return;
}

void check_olp_freeze( void )
{
  FILE* fp;
  extern bool all_programs_disabled;
  
  if( ( fp = fopen( "olp_frozen", "r" ) ) != NULL ) {
    all_programs_disabled = TRUE;
    fclose( fp );
  }
}

void do_peace( char_data* ch, char* )
{
  char_data *rch;

  if( mortal( ch, "peace" ) )
    return;

  send( ch, "You wave your hand in a gesture for silence.\n\r" );
  send( *ch->array, "%s waves %s hand in a gesture for silence.\n\r",
    ch, ch->His_Her( ) );

  for( int i = 0; i < *ch->array; i++ ) {
    if( ( rch = character( ch->array->list[i] ) ) != NULL ) {   
      rch->fighting = NULL;
      clear( rch->aggressive );
      stop_events( rch, execute_leap );
      disrupt_spell( rch );
      }
    }

  return;
}


/*
 *   BAMFIN/BAMFOUT
 */


inline const char* bamfin( wizard_data* wizard )
{
  return wizard->bamfin != empty_string
    ? wizard->bamfin : "materializes from the ether";
}


inline const char* bamfout( wizard_data* imm )
{
  return imm->bamfout != empty_string
    ? imm->bamfout : "disappears into the ether";
}
  

void do_bamfin( char_data* ch, char* argument )
{
  char    previous  [ MAX_INPUT_LENGTH ];
  wizard_data* imm  = wizard( ch );

  if( imm == NULL )
    return;

  if( *argument == '\0' ) {
    send( ch, "Your bamfin is currently:\n\r" );
    fsend( ch, "%s %s\n\r", ch->descr->name, bamfin( imm ) );
    return;
    }

  strcpy( previous, imm->bamfin );

  free_string( imm->bamfin, MEM_PLAYER );
  if( !strcasecmp( argument, "none" ) )
    imm->bamfin = empty_string;
  else
    imm->bamfin = alloc_string( argument, MEM_PLAYER );

  send( ch, "Bamfin message set to:\n\r" );
  fsend( ch, "%s %s\n\r", ch->descr->name, bamfin( imm ) );
  send( ch, "\n\r[Previous]: %s\n\r", previous );

  return;
}


void do_bamfout( char_data *ch, char *argument )
{
  char    previous  [ MAX_INPUT_LENGTH ];
  wizard_data* imm  = wizard( ch );

  if( imm == NULL )
    return;

  if( *argument == '\0' ) {
    send( ch, "Your bamfout is currently:\n\r" );
    fsend( ch, "%s %s\n\r", ch->descr->name, bamfout( imm ) );
    return;
    }

  strcpy( previous, imm->bamfout );

  free_string( imm->bamfout, MEM_PLAYER );
  if( !strcasecmp( argument, "none" ) )
    imm->bamfout = empty_string;
  else
    imm->bamfout = alloc_string( argument, MEM_PLAYER );

  send( ch, "Bamfout message set to:\n\r" );
  fsend( ch, "%s %s\n\r", ch->descr->name, bamfout( imm ) );
  send( ch, "\n\r[Previous]: %s\n\r", previous );
   
  return;
}


/*
 *   GOTO
 */


void exec_goto( char_data* victim, room_data* room, wizard_data* imm )
{
  fsend_seen( victim, "%s %s\n\r", victim, bamfout( imm ) );

  victim->From( );
  victim->To( room );

  fsend_seen( victim, "%s %s\n\r", victim, bamfin( imm ) );
  show_room( victim, room, TRUE, NULL );

  return;
}


void do_goto( char_data* ch, char* argument )
{
  wizard_data*      imm  = wizard( ch );
  char_data*        rch;
  room_data*         to;
  room_data*       from;

  if( *argument == '\0' ) {
    send( ch, "Usage: goto <location>\n\r" );
    return;
    }
 
  if( ( to = find_location( ch, argument ) ) == NULL ) 
    return;
  
  if( ( from = ch->in_room ) == to ) {
    send( ch, "You are already there!\n\r" );
    return;
    }

  if( is_set( &to->room_flags, RFLAG_OFFICE )
    && !is_god( ch ) && to->vnum != imm->office ) {
    send( ch, "That location belongs to another god.\n\r" );
    return;
    } 

  exec_goto( ch, to, imm );

  for( int i = from->contents-1; i >= 0; i-- ) 
    if( ( rch = character( from->contents[i] ) ) != NULL
      && rch->leader == ch && is_set( &rch->status, STAT_PET ) )
      exec_goto( rch, to, imm );

  return;
}


/*
 *   TRANSFER
 */


void transfer( char_data* ch, char_data* victim, room_data* room )
{
  char              tmp  [ TWO_LINES ];
  content_array*  where  = victim->array;
  char_data*        rch;

  send_seen( victim, "%s disappears in a mushroom cloud.\n\r", victim );
  victim->From( );
  victim->To( room );
  send_seen( victim, "%s arrives from a puff of smoke.\n\r", victim );

  sprintf( tmp, "** %s has transfered you. **\n\r\n\r", ch->Name( victim ) );
  tmp[3] = toupper( tmp[3] );
  send( victim, tmp );
  show_room( victim, room, FALSE, "-n" );

  for( int i = *where-1; i >= 0; i-- ) 
    if( ( rch = character( where->list[i] ) ) != NULL
      && rch->leader == victim && is_set( &rch->status, STAT_PET ) )
      transfer( ch, rch, room );

  return;
}


void do_transfer( char_data* ch, char* argument )
{
  char              arg  [ MAX_INPUT_LENGTH ];
  char_data*     victim;
  player_data*   player;
  room_data*       room;


  argument = one_argument( argument, arg );

  if( *arg == '\0' ) {
    send( ch, "Syntax: transfer <char> [room]\n\r" );
    return;
    }

  if( *argument == '\0' )
    room = ch->in_room;
  else if( ( room = find_location( ch, argument ) ) == NULL ) {
    send( ch, "No such location.\n\r" );
    return;
    }
  
  if( !strcasecmp( arg, "all" ) )
  {
    for( int i = 0; i < player_list; i++ ) {
      player = player_list[i];
      if( player->In_Game( )
        && player->shdata->level < LEVEL_APPRENTICE
        && player->in_room != room )
      transfer( ch, player, room );
    }
    send( ch, "All players transfered.\n\r" );
    return;
  }
  victim = one_character( ch, arg, "transfer", ch->array,
    (thing_array*) &player_list, (thing_array*) &mob_list );
  
  if( victim == NULL )
    return;

  if( victim->in_room == NULL )
    {
      send( ch, "They are in limbo.\n\r" );
    return;
    }

  if( victim == ch )
    {
      send( ch, "You can't transfer yourself!\n\r" );
      return;
    }

  if( !is_mob( victim ) )
  {
    if( victim->pcdata->trust > ch->pcdata->trust )
    {
      send( ch, "You may not transfer them.\n\r" );
      return;
    }
  }

  transfer( ch, victim, room );
 
  send( ch, "Ok.\n\r" );
}


/*
 *   ECHO COMMANDS
 */


void echo( const char* string )
{
  link_data*  link;
  int            n  = strlen( string );

  fprintf( stdout, string );

  for( link = link_list; link != NULL; link = link->next )
    write( link->channel, string, n );

  return;
}


void do_echo( char_data *ch, char *argument )
{
  link_data*  link;
  char         tmp [MAX_STRING_LENGTH];

  if( mortal( ch, "echo" ) )
    return;

  if( *argument == '\0' ) {
    send( ch, "Usage: echo <text>\n\r" );
    return;
  }

  for( link = link_list; link != NULL; link = link->next )
    if( link->connected == CON_PLAYING ) 
    {
      convert_to_ansi( link->player, argument, tmp );
      send( link->character, tmp );
      send( link->character, "\n\r" );
    }
}


void do_recho( char_data* ch, char* argument )
{
  char_data*  rch;
    
  if( mortal( ch, "echo" ) )
    return;

  if( *argument == '\0' ) {
    send( ch, "Syntax: recho <text>\n\r" );
    return;
    }

  for( int i = 0; i < *ch->array; i++ )
    if( ( rch = character( ch->array->list[i] ) ) != NULL ) 
      fsend( rch, argument );
}


void do_beep( char_data* ch, char* argument )
{
  player_data*  victim;

  if( *argument == '\0' ) {
    send( ch, "Usage: beep <player>\n\r" );
    return;
  }

  if( ( victim = one_player( ch, argument, "beep", 
    (thing_array*) &player_list ) ) == NULL )
    return;

  if( victim == ch ) {
    send( ch, "\7You beep yourself.\7\n\r" );
  }
  else {
    send( victim, "\7You are beeped by %s.\7\n\r", ch );
    send( ch, "You beep %s.\n\r", victim );
  }
}


void do_where( char_data* ch, char* argument)
{
/* changed Zemus - Feb21 */

  char            buf  [ MAX_STRING_LENGTH ];
  char_data*   victim;
  link_data*     link;
  bool          found  = FALSE;
  int           flags;
  bool          world;
  int          clones;

  if( !get_flags( ch, argument, &flags, "w", "Where" ) )
    return;
  
  found = ( world = is_set( &flags, 0 ) );
  
  if( world )
  {
    send( "Player         vNum     Room\n\r", ch );
    send( "------         ----     ----\n\r", ch );
  }
  else
    send( "Players near you:\n\r", ch );
  
  for( link = link_list, clones = 0; clones < player_list; )
  {
    if( link == NULL ) {
      victim = player_list[ clones++ ];
      if( !victim->In_Game( ) || victim->link == NULL
        || !is_set( &victim->status, STAT_CLONED ) )
        continue;
    }
    else
    {
      if( link->connected != CON_PLAYING )
      {
        link = link->next;
        continue;
      }
  
      victim = link->player;
      link = link->next;
    }
        
    if( victim == NULL || !victim->Seen( ch ) || !ch->Recognizes( victim )
      || victim->in_room == NULL )
      continue;
        
    if(!world)
    {
      if( victim->in_room->area != ch->in_room->area )
        continue;
    
      sprintf( buf, "%-28s %s\n\r",
        victim->descr->name, victim->in_room->name );
    }
    else
      sprintf( buf, "%-14s %-8d %s\n\r",
        victim->descr->name, victim->in_room->vnum, victim->in_room->name );

    found = TRUE;
    send( buf, ch );
  }

  if( !found )
    send( "None\n\r", ch );
    
  return;
}


void do_purge( char_data* ch, char* argument )
{
  thing_data*    thing;
  char_data*    victim;

  if( mortal( ch, "purge" ) )
    return;

  if( *argument == '\0' ) {
    fsend( *ch->array, 
      "%s utters an ancient rune, engulfing the room in a blinding light.", ch );
    send( ch, "Ok.\n\r" );

    for( int i = *ch->array-1; i >= 0; i-- )
      if( ( victim = character( ch->array->list[i] ) ) == NULL
        || ( victim->pcdata == NULL
        && !is_set( &victim->status, STAT_PET ) ) )
        ch->array->list[i]->Extract( );

    return;
    }

  if( ( thing = one_thing( ch, argument, "purge", ch->array ) ) == NULL )
    return;
  
  if( player( thing ) != NULL ) {

    if( player( thing )->link == NULL && has_permission( ch, PERM_SITE_NAMES ) ) {
       do_quit( player( thing ), "" );
       send( ch, "Done.\n\r" );
       return;
      }

    send( ch, "%s is a player and you can't purge players.\n\r", thing );
    return;
    }

  send( ch, "Ok.\n\r" );
  send( *ch->array, "%s purges %s.\n\r", ch, thing );

  thing->Extract( );
}

/* changed - zemus - april 9 */
void do_advance( char_data* ch, char* argument )
{
  char           arg  [ MAX_INPUT_LENGTH ];
  char_data*  victim;
  int          level;
  int              i;
  int          flags;

  if( !get_flags( ch, argument, &flags, "c", "Advance" ) )
    return;

  if( is_set( &flags, 0 ) ) {
     send( ch, "This doesn't work.\n\r" );
     return;
  }

  argument = one_argument( argument, arg );
  if( *arg == '\0' || *argument == '\0' || !is_number( argument ) ) {
      send( ch, "Usage: advance <char> <level>.\n\r" );
    return;
  }
  
  if( ( victim = one_player( ch, arg, "advance", 
    (thing_array*) &player_list ) ) == NULL ) 
    return;
  
  if( ( level = atoi( argument ) ) <= 0 ) {
    send( ch,
      "Advancing them to a non-positive level does not make sense.\n\r" );
    return;
  }
  
  if( ch != victim ) {
    if( !has_permission( ch, PERM_PERMFLAGS ) )
    {
      send( "You may not advance others.\n\r", ch );
      return;
    }
    if( get_trust( victim ) >= get_trust( ch )
      && get_trust( ch ) != MAX_LEVEL  )
    {
      send( ch, "You may only advance those of lower level.\n\r" );
      return;
    }
  }
  if( level > get_trust( ch ) ) 
  {
    send( "You can't advance higher than your trust.\n\r", ch );
    return;
  }

  if( level < victim->shdata->level )
  {
    victim->shdata->level  = 1;
    victim->exp            = 1000;
    victim->base_hit       = 20;
    victim->base_mana      = 100;
    victim->base_move      = 100;
    
    update_maxes( ch );
    
    victim->hit      = victim->max_hit;
    victim->mana     = victim->max_mana;
    victim->move     = victim->max_move;
    
    for( i = 0; i < MAX_SKILL; i++ )
      victim->shdata->skill[i] = UNLEARNT;
    
    victim->shdata->skill[ LANG_PRIMAL ] = 10;
    
    if( ch->shdata->race >= 0 && ch->shdata->race <= 14 )
      victim->shdata->skill[ LANG_HUMANIC+ch->shdata->race ] = 10;
  }
  
  victim->exp = 0;
  
  for( i = victim->shdata->level ; i < level; i++ ) 
    advance_level( victim, FALSE );
  
  modify_pfile( victim );
  
  if( ch != victim ) {
    send( ch, "You have set %s to level %d.\n\r",
    victim->real_name( ), level );
    send( victim, "%s has set you to level %d.\n\r",
    ch->real_name( ), level );
  }
  else 
    send( ch, "You are now level %d.\n\r", level );
}



/*
 *   RESTORE
 */
 

void do_restore( char_data* ch, char* argument )
{
  char_data* victim;

  if( mortal( ch, "restore" ) ) 
    return;

  if( *argument == '\0' ) {
    send( ch, "Usage: restore <char>\n\r" );
    return;
    }

  if( !strcasecmp( argument, "all" ) ) {
    if( !is_god( ch ) ) {
      send( ch, "You may not restore all.\n\r" );
      return;
      }
    for( int i = 0; i < player_list; i++ ) {
      victim = player_list[i];
      if( victim->In_Game( )
        && victim->shdata->level < LEVEL_APPRENTICE )
        restore( victim, ch );
      }
    send( ch, "All players restored.\n\r" );
    return;
    }
    
  if( ( victim = one_character( ch, argument, "restore",
    ch->array, (thing_array*) &player_list ) ) == NULL )
    return;

  if( !is_god( ch ) && ch != victim ) {
    send( ch, "You can only restore yourself.\n\r" );
    return;
    }

  restore( victim, ch );
  
  if( ch == victim ) 
    send( ch, "You restore yourself.\n\r" );
  else 
    send( ch, "%s restored.\n\r", victim );

  return;
}


void restore( char_data* victim, char_data* ch )
{
  victim->hit  = victim->max_hit;
  victim->mana = victim->max_mana;

  update_max_move( victim );

  victim->move = victim->max_move;

  strip_affect( victim, AFF_POISON );
  strip_affect( victim, AFF_HALLUCINATE );
  strip_affect( victim, AFF_CONFUSED );
  strip_affect( victim, AFF_BLIND );
  strip_affect( victim, AFF_SILENCE );
  strip_affect( victim, AFF_PLAGUE );
  strip_affect( victim, AFF_TOMB_ROT );
  strip_affect( victim, AFF_RABIES );
  strip_affect( victim, AFF_DEATH );

  gain_condition( victim, COND_FULL, 999 );
  gain_condition( victim, COND_THIRST, 999 );
  gain_drunk( victim, -100 );

  update_pos( victim );

  if( ch != victim ) 
    send( victim, "%s has restored you.\n\r", ch );
}


/*
 *   WIZLOCK
 */


void do_wizlock( char_data* ch, char* argument )
{
  if( *argument == '\0' ) {
    send( ch, "Wizlock is %s.\n\r", wizlock ? "on" : "off" );
    return;
    }

  bool    flag  = wizlock;
  char*    tmp  = static_string( );

  if( !toggle( ch, argument, "wizlock", ( int* ) &wizlock, 0 ) ) {
    send( ch, "Syntax: wizlock <on|off>\n\r" );
    return;
    }

  if( wizlock != flag ) {
    sprintf( tmp, "** WIZLOCKED %s by %s **",
      wizlock ? "added" : "removed", ch->Name( ) ); 
    info( tmp, LEVEL_APPRENTICE, tmp, IFLAG_LOGINS, 1, ch );
    }
}


void do_sset( char_data* ch, char* argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  char_data *victim;
  int col = 0;
  int sn;
  int value;
  bool fAll;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  if( *arg1 == '\0' ) {
    send( ch, "Syntax: sset <char> [name] [value]\n\r" );
    return;
    }
 
  if( ( victim = one_player( ch, arg1, "sset", 
    (thing_array*) &player_list ) ) == NULL )
    return;

  if( !has_permission( ch, PERM_PLAYERS ) && ch != victim ) {
    send( ch, "You can only sset yourself.\n\r" );
    return;
    }

  if( ( value = atoi( arg3 ) ) > 10 || value < 0 ) {
    send( ch, "Skill prof. can range from 0 to 10.\n\r" );
    return;
    }

  if( arg2[0] == '\0' ) {
    for( sn = 0; sn < MAX_SKILL; sn++ ) {
      if( victim->shdata->skill[sn] == UNLEARNT )
        continue;
      page( ch, "%17s (%2d)%s", skill_table[sn].name,
        victim->shdata->skill[sn], ++col%3 == 0 ? "\n\r" : "" );
      }
    if( col % 3 != 0 )
      page( ch, "\n\r" );
    return;
    }

  fAll = !strcasecmp( arg2, "all" );

  for( sn = 0; sn < MAX_SKILL; sn++ ) {
    if( fAll ) {
      victim->shdata->skill[sn] = value;
      continue;
      }
    if( !strncasecmp( skill_table[sn].name, arg2, strlen( arg2 ) ) ) {
      victim->shdata->skill[sn] = value;
      send( ch, "%s set on %s to %d.\n\r",
        skill_table[sn].name, victim->descr->name, value );
      return;
      }
    }

  if( fAll ) {
    send( ch, "All skills on %s set to %d.\n\r", victim, value );
    return;
    } 

  send( ch, "Unknown skill.\n\r" );
  return; 
}


extern char_data* forcing_char;

void do_force( char_data* ch, char* argument )
{
  char              arg  [ MAX_STRING_LENGTH ];
  char_data*     victim;
  player_data*   player;
  bool          was_set;

  argument = one_argument( argument, arg );

  if( *arg == '\0' || *argument == '\0' ) {
    send( ch, "Syntax: force <char> <command>\n\r" );
    return;
    }

  if( !strcasecmp( arg, "all" ) ) {
    if( !has_permission( ch, PERM_ADMIN ) ) {
       send( ch, "You may not force all.\n\r" );
       return;
    }
    send( ch, "You force all players to '%s'.\n\r", argument );
    for( int i = 0; i < player_list; i++ ) {
      player = player_list[i];
      if( player->In_Game( ) && get_trust( player ) < get_trust( ch ) ) {
        set_bit( &player->status, STAT_FORCED );
        send( player, "%s forces you to '%s'.\n\r", ch, argument );
        if( !( was_set = forcing_char != NULL ) )
          forcing_char = ch;
        interpret( player, argument );
        if( !was_set )
          forcing_char = NULL;
        remove_bit( &player->status, STAT_FORCED ); 
      }
    }
    return;
  }

  if( ( victim = one_character( ch, arg, "force",
    ch->array, (thing_array*) &player_list ) ) == NULL )
    return;

  if( victim == ch ) {
    send( ch, "Forcing yourself doesn't make sense.\n\r" );
    return;
  }

  if( victim->pcdata != NULL
    && !has_permission( ch, PERM_FORCE_PLAYERS ) ) {
    send( ch, "You do not have permission to force players.\n\r" );
    return;
  }

  if( get_trust( victim ) >= get_trust( ch ) ) {
    send( ch, "You are unable to force them.\n\r" );
    return;
  }

  send( ch, "You force %s to '%s'.\n\r", victim, argument );
  send( victim, "%s forces you to '%s'.\n\r", ch, argument );

  set_bit( &victim->status, STAT_FORCED );
  if( !( was_set = forcing_char != NULL ) )
    forcing_char = ch;
  interpret( victim, argument );
  if( !was_set )
    forcing_char = NULL;
  remove_bit( &victim->status, STAT_FORCED ); 
  
  return;
}


void do_invis( char_data* ch, char* argument )
{
  wizard_data*   imm;
  int            num;

  if( not_player( ch ) )
    return;

  if( ( imm = wizard( ch ) ) == NULL )
    return;

  if( *argument ==  '\0' && imm->wizinvis >= 200 ) {
    if( !is_set( ch->pcdata->pfile->flags, PLR_WIZINVIS ) ) { 
      send_seen( ch, "%s slowly fades into thin air.\n\r", ch );
      send( ch, "You slowly vanish into thin air.\n\r" );
      set_bit( ch->pcdata->pfile->flags, PLR_WIZINVIS );
      }
    else {
      remove_bit( ch->pcdata->pfile->flags, PLR_WIZINVIS );
      send_seen( ch, "%s slowly fades into existence.\n\r", ch );
      send( ch, "You slowly fade back into existence.\n\r" );
      }
    return;
    }

  num = atoi( argument );
  if( num > ch->pcdata->trust || num < 200 ) {
    send( "Choose a number between 200 and your trust.\n\r", ch );
    return;
    }

  send( ch, "Your wizinvis level has been set to %d.\n\r", num );
  imm->wizinvis = num;
}


void do_holylight( char_data* ch, char* )
{
  if( ch->pcdata == NULL )
    return;
    
  switch_bit( ch->pcdata->pfile->flags, PLR_HOLYLIGHT );

  send( ch, "Holy light mode %s.\n\r",
    on_off( ch->pcdata->pfile->flags, PLR_HOLYLIGHT ) );

  return;
}

void do_vaporise( char_data* ch, char* )
{
  if( player( ch ) == NULL )
    return;
  
  switch_bit( &ch->status, STAT_VAPOR );
  
  if( is_set( &ch->status, STAT_VAPOR ) )
    send( ch, "You vaporise.\n\r" );
  else
    send( ch, "Vapor mode off.\n\r" );

  return;
}

/*
 *   SNOOP COMMANDS
 */


void do_snoop( char_data* ch, char* argument )
{

  if( is_mob( ch ) )
    return;
  
  char buf [ MAX_INPUT_LENGTH ];
  
  link_data *link;
  player_data *victim;

  if( *argument == '\0' ) {
    bool fFound = FALSE;

    *buf = '\0';
    for( link = link_list; link != NULL; link = link->next ) 
      if( link->snoop_by != NULL
        && get_trust( ch ) >= get_trust( link->snoop_by->character ) ) {
        fFound = TRUE;
        sprintf( buf + strlen( buf ), "%15s : %s\n\r",
          link->character->real_name( ),
          link->snoop_by->character->real_name( ) );
    }
      
    if( fFound )
      send( buf, ch );
    else
      send( "No active snoops.\n\r", ch );
    return;
  }
  
  if( ( victim = one_player( ch, argument, "snoop", 
    (thing_array*) &player_list ) ) == NULL ) {
    send( "They aren't here.\n\r", ch );
    return;
  }
  
  if( victim->link == NULL )  {
    send( "No descriptor to snoop.\n\r", ch );
    return;
  }

  if( victim == player( ch ) )  {
    send( "Cancelling all snoops.\n\r", ch );
    for( link = link_list; link != NULL; link = link->next ) 
      if( link->snoop_by == ch->link )
        link->snoop_by = NULL;
      return;
  }
  
  if( victim->link->snoop_by != NULL ) {
    send( "That character is already being investigated.\n\r", ch );
    return;
  }

  if( get_trust( victim ) >= get_trust( ch ) ) {
    send( "They are a little out of your league.\n\r", ch );
    return;
  }

  if( ch->link != NULL )
    for( link = ch->link->snoop_by; link != NULL; link = link->snoop_by ) 
      if( link->character == victim || link->player == victim ) {
        send( "No snoop loops.\n\r", ch );
        return;
        }
  
  if( victim->link == ch->link ) {
    send( ch, "Snooping yourself!?  No snoop loops allowed!\n\r" );
    return;
  }
  
  if( is_set( &victim->status, STAT_CLONED ) )
    send( ch, "Warning: %s is a clone, snooping %s.\n\r", victim->descr->name,
      victim->link->player != NULL && victim->link->player->Seen( ch )
      ? victim->link->player->descr->name : "unknown" );

  victim->link->snoop_by = ch->link;
  send( "Ok.\n\r", ch );
  
  return;
}


/*
 *   SHUTDOWN/REBOOT
 */


void shutdown_sequence( char* )
{
  area_data*   area;
  room_data*   room;

  for( area = area_list; area != NULL; area = area->next ) 
    for( room = area->room_first; room != NULL; room = room->next ) 
      if( is_set( &room->room_flags, RFLAG_SAVE_ITEMS ) ) 
        save_room_items( room );

  write_all( );
}


void do_shutdown( char_data* ch, char* argument )
{
  int flags;
  FILE* fp;

  if( !get_flags( ch, argument, &flags, "n", "shutdown" ) )
    return;

  echo( "** Shutdown by %s **\n\r", ch->descr->name );
  system("cp file.ks .killscript");
  fp = open_file( FILE_DIR, "iplog.txt", "a" );
  if( fp != NULL )
    {
     fprintf( fp, "Shutdown by %s at %s", ch->descr->name, (char*) ctime( &current_time ) );     
     fclose( fp );
    }

  if( !is_set( &flags, 0 ) )
    shutdown_sequence( argument );

  exit( 1 );
}


void do_reboot( char_data* ch, char* argument )
{
  int flags;
  FILE* fp;

  /*
  if( !get_flags( ch, argument, &flags, "d", "reboot" ) )
    return;

  if( is_set( &flags, 0 ) ) {
    if( init_daemon( ) ) 
      send( ch, "Host daemon rebooted.\n\r" );
    else 
      send( ch, "Reboot of daemon failed.\n\r" );
    return;
    }
 */
  echo( "** Reboot by %s. **\n\r", ch->descr->name );
  /* Record Reboot in iplog (zemus-march11) */
  fp = open_file( FILE_DIR, "iplog.txt", "a" );
  if( fp != NULL )
    {
     fprintf( fp, "Reboot by %s at %s", ch->descr->name, (char*) ctime( &current_time ) );
     fclose( fp );
    }

  shutdown_sequence( argument );

 /* execlp( "./tfe", "tfe",
    atos( port[0] ), atos( port[1] ), NULL );
*/
  exit( 1 );
}


/*
 *   RENAME
 */


void do_rename( char_data* ch, char* argument )
{
  char           arg  [ MAX_INPUT_LENGTH ];
  char           tmp  [ THREE_LINES ];
  char_data*  victim;
  char*         name;

  in_character = FALSE;

  argument = one_argument( argument, arg );

  if( ( victim = one_player( ch, arg, "rename", 
    (thing_array*) &player_list ) ) == NULL )
    return;

  if( *argument == '\0' ) {
    send( ch, "What do you want to rename them?\n\r" );
    return;
    }  

  if( is_set( &victim->status, STAT_CLONED ) )
  {
    send( ch, "%s is a clone.  Use 'deny' to purge them.\n\r", victim->descr->name );
    return;
  }

  if( victim != ch && get_trust( victim ) >= get_trust( ch ) ) {
    send( ch, "You can only rename those of lower level.\n\r" );
    return;
    } 
/* Rename Self Check -- Modified Zemus */
  if( victim == ch && !is_god( ch ) ) {
    send( ch, "To avoid temptation you are unable to rename yourself.\n\r" );
    return;
    }

  *argument = toupper( *argument );

  if( !check_parse_name( ch->link, argument ) )
    return;

  name = victim->descr->name;

  player_log( victim, "Name changed from %s to %s.",
    name, argument ); 

  rename_file( PLAYER_DIR,      name, PLAYER_DIR,      argument );
  rename_file( PLAYER_PREV_DIR, name, PLAYER_PREV_DIR, argument );
  rename_file( PLAYER_LOG_DIR,  name, PLAYER_LOG_DIR,  argument );
  rename_file( MAIL_DIR,        name, MAIL_DIR,        argument );
  rename_file( EXTRA_DIR,       name, EXTRA_DIR,       argument );
  rename_file( EXTRA_PREV_DIR,  name, EXTRA_PREV_DIR,  argument );
  
  remove_list( pfile_list, max_pfile, victim->pcdata->pfile );

  if( ch == victim ) {
    send( ch, "You rename yourself %s.\n\r", argument );
    }
  else {
    send( ch, "%s renamed %s.\n\r", name, argument );
    send( victim, "You have been renamed %s by %s.\n\r", argument,
      ch->descr->name );
    }

  sprintf( tmp, "%s renamed %s by %s.",
    name, argument, ch->descr->name );
  info( "", LEVEL_APPRENTICE, tmp, IFLAG_WRITES, 2, ch );

  free_string( victim->pcdata->pfile->name, MEM_PFILE );
  free_string( victim->descr->name,         MEM_DESCR );

  victim->pcdata->pfile->name = alloc_string( argument, MEM_PFILE );
  victim->descr->name         = alloc_string( argument, MEM_DESCR );
 
  add_list( pfile_list, max_pfile, victim->pcdata->pfile );
  
  set_playing( victim->link );
  write( victim->link->player );
}


void remote_command( char_data* ch, room_data* at_room, char* command );

void do_at( char_data* ch, char* argument )
{
  char           arg  [ MAX_INPUT_LENGTH ];
  char_data*  victim;
  room_data* at_room = NULL;

  argument = one_argument( argument, arg );

  if( *arg == '\0' || *argument == '\0') {
    send( ch, "Syntax: at <player/room vnum> <command>.\n\r" );
    return;
  }
  
  if( is_number( arg ) )
  {
    if( ( at_room = get_room_index( atoi( arg ), FALSE ) ) == NULL )
      send( ch, "Room not found.\n\r");
  }
  else if( ( victim = one_player( ch, arg, "at",
    (thing_array*) &player_list ) ) != NULL )
    at_room = victim->in_room;
  else
    send( ch, "at Where?\n\r" );
  
  if( at_room == NULL )
      return;
  
  remote_command( ch, at_room, argument );
}

/* SMITH - nested rat check changed - PUIOK 23/4/00 */
void do_rat( char_data* ch, char* argument )
{
  char                   arg  [2][ MAX_INPUT_LENGTH ];
  char*              chkptr;
  room_data*         at_room = NULL;
  int             vnum_from;
  int               vnum_to;
  int                 flags;
  bool                  skip = TRUE;
  bool                unpage = FALSE;
    
  if( !get_flags( ch, argument, &flags, "hp", "Rat" ) )
    return;
  
  argument = one_argument( argument, arg[0] );
  chkptr = argument = one_argument( argument, arg[1] );
  
  if( *chkptr == '_' )
    chkptr++;
  
  if( *arg == '\0' || *chkptr == '\0'
    || !is_number( arg[0] ) || !is_number( arg[1] ) ) {
    send( ch, "Syntax: rat <start vnum> <end vnum> <command>.\n\r" );
    return;
  }
  
  if( ( vnum_to = atoi( arg[1] ) ) < ( vnum_from = atoi( arg[0] ) ) )
  {
    send( ch, "First vnum must be smaller than the second.\n\r" );
    return;
  }
  
  if( vnum_to - vnum_from > 2500 )
  {
    send( ch, "Rat for over 2500 rooms disallowed.\n\r" );
    return;
  }
  
  if( !strncasecmp( chkptr, "rat ", 4 ) ||
    is_set( &ch->status, STAT_REMOTE ) )
  {
    send( ch, "Nested rat disallowed.\n\r" );
    return;
  }
  
  if( is_set( &flags, 1 ) )
  {
    sprintf( arg[0], "@e+=---------- Rat %d %d %s: ----------=+@n\n\r",
      vnum_from, vnum_to, argument );
    
    convert_to_ansi( ch, arg[0], arg[1] );

    if( unpage = !is_set( &ch->status, STAT_PAGE_ALL ) )
      set_bit( &ch->status, STAT_PAGE_ALL );

    page_centered( ch, arg[1] );
  }
  
  for( ; vnum_from <= vnum_to; vnum_from++ )
  {
    at_room = get_room_index( vnum_from, FALSE );
    
    if( at_room == NULL )
    {
      if( is_set( &flags, 0 ) )
        sprintf( arg[0], "@e+=----- Room not found: %d -----=+@n\n\r", vnum_from );
      else
        send( ch, "Room not found: %d\n\r", vnum_from );
    }
    else if( !skip && is_set( &flags, 0 ) )
    {
      sprintf( arg[0], "@e+=---------- Room: %d ----------=+@n\n\r", vnum_from );
    }
    
    if( is_set( &flags, 0 ) && ( at_room == NULL || !skip ) )
    {
      convert_to_ansi( ch, arg[0], arg[1] );
      
      if( ch->link->paged != NULL )
      {
        page( ch, "\n\r" );
        page_centered( ch, arg[1] );
      }
      else
      {
        send( ch, "\n\r" );
        send_centered( ch, arg[1] );
      }
    }

    if( at_room != NULL )
      remote_command( ch, at_room, argument );
    
    if( !ch->Is_Valid( ) )
      return;
    
    if( skip )
      skip = FALSE;
  }
  
  if( unpage )
    remove_bit( &ch->status, STAT_PAGE_ALL );
}

void remote_command( char_data* ch, room_data* at_room, char* command )
{
    room_data* original_room = ch->in_room;
    obj_data*   original_obj = ch->pos_obj;
    bool       dont_remove;
    
    ch->in_room = at_room;
    ch->array   = &at_room->contents;
    
    if( !( dont_remove = is_set( &ch->status, STAT_REMOTE ) ) )
      set_bit( &ch->status, STAT_REMOTE );
    
    interpret( ch, command );
    
    if( ch->position == POS_EXTRACTED )
      return;
    
    if( !dont_remove )
      remove_bit( &ch->status, STAT_REMOTE );
    
    if( ch->pos_obj != NULL )
      ch->pos_obj = NULL;
    
    stop_fight( ch );
    stop_events( ch, execute_drown );
    
    ch->in_room = original_room;
    ch->array   = &original_room->contents;
    
    if( original_obj != ch->pos_obj )
      ch->pos_obj = original_obj;
}

void do_clone( char_data* ch, char* argument )
{
  char            arg [ MAX_INPUT_LENGTH ];
  player_data*    pc;
  pfile_data*  pfile;  
  int          flags;
  int          level;
  
  if( player( ch ) == NULL || ch->link->player->shdata->level <= LEVEL_APPRENTICE )
    return;
  
  if( !get_flags( ch, argument, &flags, "mfihkcsl", "Clone" ) )
    return;
  
  if( is_set( &flags, 6 ) )
  {
    send( ch, "Flags:\n\t-m male character\n\t-f female character\n\t" );
    send( ch, "-i show login info\n\t-h start the character in this room\n\t" );
    send( ch, "-k copy my abilities\n\t-c copy my major stats\n\t-l list current clones\n\t" );
    send( ch, "-s show this info\n\r" );
    return;
  }
  
  if( is_set( &flags, 7 ) )
  {
    sprintf( arg, "%-15s   %-15s   %s\n\r", "Puppet", "Master", "Location" );
    page_underlined( ch, arg );
    for( int i = 0; i < player_list; i++ ) {
      pc = player_list[i];
      if( pc->In_Game( ) && is_set( &pc->status, STAT_CLONED ) ) {
        sprintf( arg, "%-15s   %-15s   %s", pc->descr->name,
          pc->link ? ( ch->Can_See( pc->link->player ) ?
          pc->link->player->descr->name : "Someone" ) : "(link-less)",
          ( pc->array && pc->array->where ) ? pc->array->where->Location( ) : "nowhere" );
        truncate( arg, 78 );
        strcat( arg, "\n\r" );
        page( ch, arg );
      }
    }
    return;
  }
  
  argument = one_argument( argument, arg );
  
  if( *arg == '\0' )
  {
    send( "You have to give a name!\n\r", ch );
    return;
  }
  
  if( *argument != '\0' )
  {
    level = atoi( argument );
    if( level < 1 || level > ch->pcdata->trust )
    {
      send( ch, "Unappropriate level.\n\r" );
      return;
    }
  }
  else
    level = 1;
  
  *arg = toupper( *arg );
  
  if( !check_parse_name( ch->link, arg ) )
    return;
  
  pfile = new pfile_data( arg );
  pc = new player_data( pfile->name );
    
  pc->pcdata->pfile = pfile;
  
  
  if( ( pc->pcdata->terminal = ch->pcdata->terminal ) != TERM_DUMB ) 
    for( int i = 0; i < MAX_COLOR; i++ )
      pc->pcdata->color[i] = ch->pcdata->color[i];

  if( is_set( &flags, 0 ) )
    pc->sex = 1;
  else if( is_set( &flags, 1 ) )
    pc->sex = 2;
  else
    pc->sex = ch->sex;

  pc->pcdata->clss = ch->pcdata->clss;
  pc->shdata->race = ch->shdata->race;
  
  pc->shdata->skill[ LANG_HUMANIC+ch->shdata->race ] = 10;
  pc->pcdata->speaking = LANG_HUMANIC+pc->shdata->race;
  pc->shdata->skill[ LANG_PRIMAL ] = 10;

  pc->shdata->alignment = ch->shdata->alignment;
    
  pc->link = ch->link;
  ch->link->character = pc;
    
  new_player( pc );
  
  ( (player_data*) ch )->switched = pc;
  set_bit( &pc->status, STAT_CLONED );
  
  if( is_set( &flags, 4 ) )
    for( int i = 0; i < MAX_SKILL; i++ )
      pc->shdata->skill[i] = ch->shdata->skill[i];

  pc->shdata->level = level;
  
  if( is_set( &flags, 5 ) )
  {    
    pc->hit    = pc->max_hit    = ch->hit;
    pc->mana   = pc->max_mana   = ch->mana;
    pc->move   = pc->max_move   = ch->move;
    
    pc->gossip_pts              = player( ch ) ? player( ch )->gossip_pts : 50;
    
    pc->shdata->strength        = ch->shdata->strength;
    pc->shdata->intelligence    = ch->shdata->intelligence;
    pc->shdata->wisdom          = ch->shdata->wisdom;
    pc->shdata->dexterity       = ch->shdata->dexterity;
    pc->shdata->constitution    = ch->shdata->constitution;
  }
  
  if( is_set( &flags, 3 ) )
  {
    pc->From( );
    pc->To( ch->in_room );
  }
  
  pc->timer = current_time;
  pc->was_in_room = NULL;

  sprintf( arg, "%s has cloned %self as '%s'",
    ch->descr->name, ch->Him_Her( ), pc->descr->name );

  info( empty_string, LEVEL_ENTITY, arg, IFLAG_LOGINS, 1, ch, NULL, 0, PERM_SITE_NAMES );

  sprintf( arg, "%s created a clone '%s'",
    ch->link->player->descr->name, pc->descr->name );
  
  player_log( ch->link->player, arg );

  /* immortal log logging added */
  sprintf( arg, "*** CLONE created by %s ***", ch->descr->name ); 
  immortal_log( pc, arg, "" );

  if( is_set( &flags, 2 ) )
  {
    sprintf( arg, "%s has connected.", pc->descr->name );
    
    info( arg, LEVEL_ENTITY, arg, IFLAG_LOGINS, 1, pc );
    send_seen( pc, "%s has entered the game.\n\r", pc );
  }
  
  send( pc, "You are know as %s ***.\n\r", pc->Long_Name( pc, 1 ) );
}

void purge_clone( player_data* ch )
{
  if( ch == NULL || !is_set( &ch->status, STAT_CLONED ) )
    return;
  
  if( ch->link != NULL )
  {
    if( ch->link->character == ch )
      do_return( ch, "" );
    ch->link = NULL;
  }
    
  purge( ch );
}

/* idletime - trinity march 20 */

void do_idletime( char_data* ch, char* )
{
  char            tmp  [ THREE_LINES ];
  player_data*     pc;
  int        idletime;

  if( is_mob( ch ) )
    return;

  sprintf( tmp, "%-20s %s\n\r", "Player", "Idle" );
  page_underlined( ch, tmp );
  
  for( int i = 0; i < player_list; i++ ) {
    pc = player_list[ i ];
    
    if( !pc->Seen( ch ) )  /* In_Game( ) check done inside */
      continue;
    
    if( is_set( pc->pcdata->pfile->flags, PLR_IDLE )
      || is_admin( ch ) || wizard( pc ) == NULL )
      idletime = (int) current_time - pc->timer;
    else
      idletime = 0;

    sprintf( tmp, "%-20s %-5d  %s\n\r", pc->descr->name,
      idletime, pc->link == NULL ? "[linkdead]" : "" );
    page( ch, tmp );
  }
}


