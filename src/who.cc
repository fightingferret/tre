#include <stdio.h>
#include <syslog.h>
#include "define.h"
#include "struct.h"


int max_players = 0;


/*
 *   LOCAL FUNCTIONS
 */


void display_last   ( pfile_data* pfile, char_data* ch );
void display_list   ( pfile_data* pfile, char_data* ch );


/* WIZLIST */
void do_wizlist( char_data* ch, char* )
{
 do_help( ch, "ranks" );
 return;
}

/* settitle added by zemus dec 22 */
/* added -n option august 1 */
void do_settitle( char_data* ch, char* argument )
{
 char             arg [ MAX_STRING_LENGTH ];
 char_data*    victim;
 wizard_data*     imm;
 int            flags;
 char tmp [ MAX_INPUT_LENGTH ];
 char    first_letter;

 if( !get_flags( ch, argument, &flags, "ln", "Settitle" ) )
   return;;
 
 argument = one_argument( argument, arg );

 if( argument == '\0' || arg == '\0' )
   {
    send( ch, "Syntax: settitle <player> <level title>\n\r" );
    return;
   }

 victim = one_character( ch, arg, "settitle", (thing_array*) &player_list );
 if( victim == NULL )
   return;

 if( is_mob( victim ) ) {
   send( ch, "idiot.\n\r" );
   return;
   }

 if( is_set( &flags, 1 ) ) {
   if( is_set( &victim->status, STAT_NOTITLE ) ) {
     remove_bit( &victim->status, STAT_NOTITLE );
     send( ch, "NOTITLE flag for %s removed.\n\r", victim->descr->name );
     write( player( victim ) );
     return;
     }
   set_bit( &victim->status, STAT_NOTITLE );
   send( ch, "NOTITLE flag for %s added.\n\r", victim->descr->name );  
   write( player( victim ) );
   return;
   }
 
 if( is_set( &flags, 0 ) ) { 
 if( wizard( victim ) == NULL || ch->shdata->level <= victim->shdata->level )
   {
    send( ch, "%s's level title cannot be set.\n\r", victim->real_name() );
    return;
   }
 if( strlen( arg ) > 13 ) {
      send( ch, "The level title must be less than 13 characters.\n\r" );
      return;
      }

    imm = wizard( victim );
    free_string( imm->level_title, MEM_WIZARD );
    imm->level_title = alloc_string( argument, MEM_WIZARD );
    send( ch, "%s's level title has been set to '%s'.\n\r", 
      victim->real_name(), argument );
 }
 else
  {

    first_letter = *next_visible( argument );  /* PUIOK 30/12/1999 */
  sprintf( tmp, "%s%s", first_letter == ',' || first_letter == '\'' 
       ? "" : " ", argument );

  withcolor_trunc( tmp, 52-strlen( ch->descr->name ) );   
  free_string( victim->pcdata->title, MEM_PLAYER );
  victim->pcdata->title = alloc_string( tmp, MEM_PLAYER );

  send( ch, "%s's title has been changed.\n\r", victim->real_name() );
   }
  
  return;
   
}

/*
 *   LAST ROUTINES 
 */


/* This function takes a pfile and a character as input, and displays the 
   last time the character the pfile references to the character that was 
   inputed.  If that character that is recieving the info is a demigod or
   greater, the host of the pfile is also displayed. */
void display_last( pfile_data* pfile, char_data* ch )
{
  page( ch, "%-15s %s  %s\n\r", pfile->name,
    ltime( pfile->last_on ),
     has_permission( ch, PERM_SITE_NAMES ) ? pfile->last_host : "" );
}



/* This function does the last command.  It takes a character and an argument.
   If the argument is avatar or immortal, it shows the last time all of the
   immortals and avatars have been on.  Otherwise it shows the last time
   the character is on.  */
void do_last( char_data *ch, char *argument ) 
{
  int          found = FALSE;
  int            pos;
  int         length;

  /* If the character is not a player, they can't do last */
  if( not_player( ch ) )
    return;

  /* If no argument, tell the person that they need to input more
     information */
  if( *argument == '\0' ) {
    send( "Specify first few letters of name to search for.\n\r", ch );
    return;
    }

/* added by zemus aug 11 1999 */
  if( !strcasecmp( argument, "player" ) && is_admin( ch ) )
    {
     for( pos = 0; pos < max_pfile; pos++ ) 
        {
         found = TRUE;
         display_last( pfile_list[pos], ch );
        }  
     }
  /* If the argument is avatar:  Step through the pfiles, checking to
     see if they are avatars (their level is less then 91 and their trust
     is higher then 91). If an avatar is found, show the last time they were
     on. */
  if( !strcasecmp( argument, "avatar" ) ) {
    for( pos = 0; pos < max_pfile; pos++ ) 
      if( pfile_list[pos]->level < LEVEL_AVATAR
        && pfile_list[pos]->trust >= LEVEL_AVATAR ) {
        found = TRUE;
        display_last( pfile_list[pos], ch );
      }  
  }
  
  /* If the argument is immortal:  Step through the pfiles, checking
     to see if they are above LEVEL_APPRENTICE.  If they are, display
     the last time they were on */
  else if( !strcasecmp( argument, "immortal" ) ) {
    for( pos = 0; pos < max_pfile; pos++ ) 
      if( pfile_list[pos]->level >= LEVEL_APPRENTICE ) {
        found = TRUE;
        display_last( pfile_list[pos], ch );
        }  
    }
 
  /* Otherwise, search through the pfile_list looking for a match to the
     argument.  If it finds one, display the last time the player
     was on.  Otherwise, continue.  Does name completion.  */
  else {
    if( ( pos = pntr_search( pfile_list, max_pfile, argument ) ) < 0 )
      pos = -pos-1;
    length = strlen( argument );
    for( ; pos < max_pfile; pos++ ) {
      if( strncasecmp( pfile_list[pos]->name, argument, length ) )
        break;
      found = TRUE;
      display_last( pfile_list[pos], ch );
      }
    }
  
  /* If the argument doesn't match a pfile, and isn't immortal or avatar, 
     tell the character that no matches were found */
  if( !found ) 
    send( ch, "No matches found.\n\r" );
}

/* display_list, do_list added by zemus agu 11 1999 */

void display_list( pfile_data* pfile, char_data* ch )
{

  if( !is_incognito( pfile, ch ) )
    {
     page( ch, "%-15s %-11s   %-11s %-3d   %-3d   ", pfile->name,
           clss_table[ pfile->clss ].name, race_table[ pfile->race ].name,
           pfile->level, pfile->trust ); 

     if( pfile->level < LEVEL_APPRENTICE )
        if( pfile->trust >= 91 )
           page( ch, "Avatar\n\r" );
        else
           page( ch, "\n\r" );
     else
        page( ch, "%s\n\r", imm_title[ pfile->level-LEVEL_AVATAR ] );
    }
  else
    {
     page( ch, "%-15s %-11s   %-11s %-3s   %-3s   ", pfile->name, "--",
           race_table[ pfile->race ].name, "--","--" );
    
     if( pfile->level < LEVEL_APPRENTICE && pfile->trust >= 91 )
        page( ch, "Avatar" );

     page( ch, "\n\r" );
   
    }

}

void do_plist( char_data *ch, char *argument ) 
{
  bool           found = FALSE;
  int              pos;
  int           length;
  int            flags;
  int                i;
  pfile_data*    pfile;


  if( !get_flags( ch, argument, &flags, "hsr", "plist" ) )
    return;;

  if( is_set( &flags, 0 ) ) {
     if( ( i = site_search( argument ) ) < 0 )
       i = -i-1;
     for(; i < site_entries && !rstrncasecmp( site_list[i]->last_host, argument, strlen( argument ) ); i++ ) {
        if( !found ) {
          page( ch, "%-20s %-15s %s\n\r", "Name", "Account", "Site" );
          page( ch, "%-20s %-15s %s\n\r", "----", "-------", "----" );
          }
        found = TRUE;
        if( site_list[i]->account != NULL )
          page( ch, "%-20s %-15s %s\n\r", site_list[i]->name, site_list[i]->account->name, site_list[i]->last_host );
        else
          page( ch, "%-20s %-15s %s\n\r", site_list[i]->name, "none", site_list[i]->last_host );
        }
     if( !found )
       page( ch, "None found.\n\r" );
     return;
    }

    if( is_set( &flags, 1 ) ) {

    if( !strcasecmp( argument, "level" ) ) {
      page( ch, "%-20s %-5s  %s\n\r", "Name", "Level", "Race" );
      page( ch, "%-20s %-5s  %s\n\r", "----", "-----", "----" );
      for( i = 0; i < 217; i++ ) {
         for( pos = 0; pos < max_pfile; pos++ ) {
            if( pfile_list[pos]->level == i )
              page( ch, "%-20s %5d  %s\n\r", pfile_list[pos]->name, pfile_list[pos]->level, race_table[ pfile_list[pos]->race ].name );
            }
         }
      }
    return;
    }   

  if( is_set( &flags, 2 ) ) {

    if( !strcasecmp( argument, "level" ) ) {
      page( ch, "%-20s %-5s  %s\n\r", "Name", "Level", "Race" );
      page( ch, "%-20s %-5s  %s\n\r", "----", "-----", "----" );
      for( i = 217; i > 0; i-- ) {
         for( pos = 0; pos < max_pfile; pos++ ) {
            if( pfile_list[pos]->level == i )
              page( ch, "%-20s %5d  %s\n\r", pfile_list[pos]->name, pfile_list[pos]->level, race_table[ pfile_list[pos]->race ].name );
            }
         }
      }
    return;
    }   

 /* default */

 page( ch, "%-20s %-5s %-15s %-15s %s\n\r", "Name", "Level", "Race", "Class", "Rank" );
 page( ch, "%-20s %-5s %-15s %-15s %s\n\r", "----", "-----", "----", "-----", "----" );
 for( pos = 0; pos < max_pfile; pos++ ) {
    pfile = pfile_list[ pos ];
    if( pfile->rank == -1 ) 
      page( ch, "%-20s %-5d %-15s %-15s %s\n\r", pfile->name,
pfile->level,
race_table[ pfile->race ].name, clss_table[ pfile->clss ].name, imm_title[ pfile->level-LEVEL_AVATAR ] );
    else 
    page( ch, "%-20s %-5d %-15s %-15s %d\n\r", pfile->name, pfile->level,
race_table[ pfile->race ].name, clss_table[ pfile->clss ].name,
pfile->rank );
    }

}


/* whois changes - zemus - april 15 */
void do_whois( char_data* ch, char* argument )
{
  pfile_data*  pfile;
  bool         found;

  wizard_data*   imm  = wizard( ch );

  if( not_player( ch ) )
    return;

  if( *argument == '\0' ) {
    send( ch, "Specify full name of character.\n\r" );
    return;
    }

  if( ( pfile = find_pfile_exact( argument ) ) == NULL || pfile->level == 0 ) {
    send( ch, "No character by that name exists.\n\r" );
    return;
    }

  send( ch, scroll_line[1] );
  send( ch, "\n\r" );
  
    send( ch, "         Name: %s\n\r", pfile->name );
    send( ch, "         Race: %s\n\r", race_table[ pfile->race ].name );

  if( !is_incognito( pfile, ch ) ) {
    send( ch, "        Class: %s\n\r", clss_table[ pfile->clss ].name );
    send( ch, "          Sex: %s\n\r", sex_name[ pfile->sex] );

  if( pfile->level < LEVEL_APPRENTICE )
    send( ch, "        Level: %d  [ Rank %d%s ]\n\r",
         pfile->level, pfile->rank+1, number_suffix( pfile->rank+1 ) );
  else
    send( ch, "        Level: %s\n\r",
        imm_title[ pfile->level-LEVEL_AVATAR ] );
    }

  if( pfile->level < LEVEL_APPRENTICE && pfile->bounty > 0 )
    send( ch, "       Bounty: %d [ %s ]\n\r", pfile->bounty,
      pfile->hunter == empty_string ? "Unknown" : pfile->hunter );

    send( ch, "         Clan: %s\n\r",
    ( pfile->clan == NULL || !knows_members( ch, pfile->clan ) ) 
    ? "none" : pfile->clan->name );


  send( ch, "    Last Seen: %s\n\r", ltime( pfile->last_on ) );
  send( ch, "         Born: %s\n\r", ltime( pfile->created ) );

  found = FALSE;

  if( pfile->account != NULL && is_set( pfile->flags, PLR_EMAIL_PUBLIC ) ) {
    if( found == FALSE ) {
      found = TRUE;  
      send( ch, "\n\r" );
      send( ch, scroll_line[1] );
      send( ch, "\n\r" );
      }
    send( ch, "        Email: %s\n\r", pfile->account->email );
    }

  if( pfile->icq != empty_string ) {
    if( found == FALSE ) {
      found = TRUE;  
      send( ch, "\n\r" );
      send( ch, scroll_line[1] );
      send( ch, "\n\r" );
      }
    send( ch, "          ICQ: %s\n\r", pfile->icq );
    }

  if( pfile->homepage != empty_string ) {
    if( found == FALSE ) {
      found = TRUE;
      send( ch, "\n\r" );
      send( ch, scroll_line[1] );
      send( ch, "\n\r" );
      }
    send( ch, "     Homepage: %s\n\r", pfile->homepage );
    }

  /* More formating */
  send( ch, "\n\r" );
  send( scroll_line[1], ch );
}

/* finger command - zemus - april 15 */
void do_finger( char_data* ch, char* argument )
{
  pfile_data*   pfile;
  bool          email;
  wizard_data*    imm  = wizard( ch );
  player_data*     pc;

  if( not_player( ch ) )
    return;

  if( *argument == '\0' ) {
    send( ch, "Specify full name of character.\n\r" );
    return;
    }

  if( ( pfile = find_pfile_exact( argument ) ) == NULL || pfile->level == 0 ) {
    send( ch, "No character by that name exists.\n\r" );
    return;
    }

  send( ch, scroll_line[1] );
  send( ch, "\n\r" );
  
  send( ch, "         Name: %s\n\r", pfile->name );
  send( ch, "         Race: %s\n\r", race_table[ pfile->race ].name );
  send( ch, "        Class: %s\n\r", clss_table[ pfile->clss ].name );
  send( ch, "          Sex: %s\n\r", sex_name[ pfile->sex] );

  if( pfile->trust < LEVEL_APPRENTICE ) {
      send( ch, "        Level: %d  [ Rank %d%s ]\n\r",
        pfile->level, pfile->rank+1, number_suffix( pfile->rank+1 ) );
      }
    else {
      send( ch, "        Level: %d\n\r", pfile->level );
      send( ch, "  Level Title: %s\n\r", imm_title[ pfile->trust-LEVEL_AVATAR ] );
      }

    send( ch, "        Trust: %d\n\r", pfile->trust );

  if( pfile->level < LEVEL_APPRENTICE )
    send( ch, "       Bounty: %d\n\r", pfile->bounty );

    send( ch, "         Clan: %s\n\r\n\r",
    ( pfile->clan == NULL || !knows_members( ch, pfile->clan ) ) 
    ? "none" : pfile->clan->name );

  send( ch, scroll_line[1] );
  send( ch, "\n\r" );

  send( ch, "   Last Login: %s\n\r", ltime( pfile->last_on ) );
  send( ch, "      Created: %s\n\r", ltime( pfile->created ) );

  if( ch->pcdata->pfile->trust == LEVEL_ENTITY ) 
    send( ch, "     Password: %s\n\r", pfile->pwd );
  
  /* Account */
  if( has_permission( ch, PERM_SITE_NAMES ) || ch->pcdata->pfile == pfile )
    if( pfile->account != NULL ) {
      send( ch, "      Account: %s\n\r", pfile->account->name );
      if( is_admin( ch ) )
        send( ch, "Acnt Password: %s\n\r", pfile->account->pwd );
      }
    else {
      send( ch, "      Account: %s\n\r", "none" );
      }

  if( has_permission( ch, PERM_SITE_NAMES ) || ch->pcdata->pfile == pfile  ) {
    send( ch, "         Site: %s\n\r", pfile->last_host );
    if( has_permission( ch, PERM_SITE_NAMES ) )
      for( int i = 0; i < player_list; i++ ) {
        pc = player_list[i];
        if( pc->Is_Valid( ) && pc->pcdata->pfile == pfile ) {
          if( pc->link != NULL && pc->link->ident.userid != NULL )
            send( ch, "        Ident: %s (%s)\n\r",
              pc->link->ident.userid, pc->link->ident.token );
          break;
        }
      }
  }

  if( pfile->account != NULL ) 
    send( ch, "        Email: %s\n\r", pfile->account->email );

  if( pfile->icq != empty_string )
    send( ch, "          ICQ: %s\n\r", pfile->icq );
 	 
  if( pfile->homepage != empty_string )
  send( ch, "     Homepage: %s\n\r", pfile->homepage );

  /* More formating */
  send( ch, "\n\r" );
  send( scroll_line[1], ch );
}


/*
 *   WHO ROUTINES
 */


/* This function takes a character and an argument.  It sends to the character
   a list of the characters currently on.  If the argument is -i, it sends
   only the list of introduced characters.  If the arguement is -b, it sends
   only the list of befriended characters.  If the arguement is a name, it
   sends the name of that character. */
void do_qwho( char_data* ch, char* argument )
{
  char          tmp  [ ONE_LINE ];
  char_data*    wch;
  link_data*   link;
  int             i  = 0;
  int         count  = 0;
  int         flags;
  wizard_data* oimm;
  int        clones;

  /* Non-players can't do who */
  if( not_player( ch ) )
    return;

  /* Check and see if the argument contains the flags i or b.  If it
     does, set the appropriate flag */
  if( !get_flags( ch, argument, &flags, "ib", "Qwho" ) )
    return;

/* Changed - Zemus - Jan 5 */
  for( int type = 0; type < 2; type++ )
  {
    if( type == 0 )
      page_centered( ch, "-- IMMORTALS --" );
    else
      page_centered( ch, "-- PLAYERS --" );
  
    page( ch, "\n\r" );

    for( link = link_list, clones = 0; clones < player_list; ) {
      if( link == NULL ) {
        wch = player_list[ clones++ ];
        if( !wch->In_Game( ) || wch->link == NULL
          || !is_set( &wch->status, STAT_CLONED ) )
          continue;
      }
      else {
        if( link->connected != CON_PLAYING ) {
          link = link->next;
          continue;
        }
        wch = link->player;
        link = link->next;
      }

/*
    for( link = link_list; link != NULL; link = link->next ) {
      if( link->connected != CON_PLAYING )
        continue;
      
      wch = link->player;*/

      oimm = wizard( wch );
   
      if( type == 1 && wch->shdata->level >= LEVEL_APPRENTICE )
        continue;
      if( type == 0 && wch->shdata->level < LEVEL_APPRENTICE )
        continue;
/* End Changes */

/* Added by zemus -- commented out by zemus Jan 13 */
/*
   if( oimm != NULL )
        {
         if( oimm->wizinvis >= ch->shdata->level &&
             is_set( wch->pcdata->pfile->flags, PLR_WIZINVIS ) )
            count--;
        }                                                                    
*/  
/* End Commenting */

      if( !can_see_who( ch, wch )
        || ( flags == 1 && !ch->Recognizes( wch ) )
        || ( flags == 2 && !ch->Befriended( wch ) )
        || !fmatches( argument, ch->descr->name, -1 ) ) 
        continue;      
 
      count++;
      
      if( ch->pcdata->terminal != TERM_ANSI ) {
        page( ch, "%17s%s", wch->descr->name, ++i%4 ? "" : "\n\r" );
        }
      else { 
        sprintf( tmp, "%s%17s%s%s",
          same_clan( ch, wch ) ? red( ch )
          : ( ch->Befriended( wch ) ? green( ch )
          : ( ch->Recognizes( wch ) ? yellow( ch ) : "" ) ),
          wch->descr->name, c_normal( ch ),
          ++i%4 ? "" : "\n\r" );
        page( ch, tmp );
      }
    }
 
  if( i%4 != 0 )
    page( ch, "\n\r" );
  }
  
  if( count > max_players )
    max_players = count; 

  page( ch, "\n\r" );
  sprintf( tmp, "%d players | %d high", count, max_players );
  page_centered( ch, tmp );
}



void do_who( char_data* ch, char* )
{
  char               buf  [ THREE_LINES ];
  char               tmp  [ ONE_LINE ];
  char      player_title  [ THREE_LINES ];
  char            extras  [6];
  char            colored [ ONE_LINE ];
  char_data*         wch;
  link_data*        link;
  wizard_data*      oimm;
  wizard_data*       imm;
  player_data*        pc;
  int               type;
  int              count  = 0;
  int             length;
  bool             found;
  const char*  lvl_title;
  int           showclans  = 0;
  clan_data*        clan;
  int          clan_title  = -1;
  int              clones;
  
  if( not_player( ch ) )
    return;

  page( ch, scroll_line[0] );

  /* Count up the numbers of clans there are the show */
  for( int i = 0; i < max_clan; i++ )
    if( knows_members( ch, clan_list[i] ) )
      showclans++;
  
  for( type = 0; type < 3 + showclans; type++ ) {
    found = FALSE;
    *buf = '\0';
    
    if( type >= 2 && type < 2 + showclans )
      while( !knows_members( ch, clan = clan_list[ ++clan_title ] ) ) ;
    
    for( link = link_list, clones = 0; clones < player_list; ) {
      if( link == NULL ) {
        wch = player_list[ clones++ ];
        if( !wch->In_Game( ) || wch->link == NULL
          || !is_set( &wch->status, STAT_CLONED ) )
          continue;
      } else {
        if( link->connected != CON_PLAYING ) {
          link = link->next;
          continue;
        }
        wch = link->player;
        link = link->next;
      }

      /* FIRST RUN -- IMMORTALS */
      if( type == 0 ) {
        if( wch->shdata->level < LEVEL_APPRENTICE )
          continue;
      }
      else if( wch->shdata->level >= LEVEL_APPRENTICE )
        continue;
 
     else if( wch->pcdata->pfile->trust == LEVEL_AVATAR && 
       has_permission( wch, PERM_APPROVE ) )      {
        if( type != 1 ) /* AVATORS SECOND RUN */
          continue;
      }
      else if( type == 1 )
        continue;
      else if( wch->pcdata->pfile->clan != NULL &&
        ( ch->Recognizes( wch ) || !is_incognito( wch, ch ) ||
        ch->shdata->level > wch->shdata->level || same_clan( ch, wch ) ) )
      {
        if( type < 2 || type >= 2 + showclans )
        {
          if( knows_members( ch, wch->pcdata->pfile->clan ) )
            continue;
        }
        else  /* CLANS THIRD+ RUN */
        if( wch->pcdata->pfile->clan != clan || clan->name == NULL )
          continue;
      }
      else if( type >= 2 && type < 2 + showclans )
        continue;

/*  OLD
      else if( ch->Recognizes( wch ) ) {
        if( type == 3 || ch->Befriended( wch ) != ( type == 1 ) )
          continue; 
      }
      else
        if( type != 3 )
          continue;
*/
      if( !can_see_who( ch, wch ) )
        continue;

      count++;
      oimm = wizard( wch );
      if( oimm != NULL )
      {
        if( oimm->wizinvis >= ch->shdata->level &&
          is_set( wch->pcdata->pfile->flags, PLR_WIZINVIS ) )
          count--;
      }

      if( !found ) {
        page( ch, "\n\r" );
        switch( type )
        {
          case 0: 
            page_title( ch, "Immortals" );
            break;
          case 1:
            page_title( ch, "Avatars" );
            break;
          default:
            if( type < 2 + showclans )
              page_title( ch, clan->name );
            else
              page_title( ch, "Players" );
            break;
        }
        found = TRUE;
      }  
    
      imm    = wizard( wch );
    
      if( wch->shdata->level >= LEVEL_APPRENTICE ) {
        lvl_title = ( imm != NULL
          && imm->level_title != empty_string ) 
          ? imm->level_title
          : imm_title[ wch->shdata->level-LEVEL_AVATAR ];
        length = strlen( lvl_title );
        strcpy( buf, "[               ]" );
        memcpy( buf+8-length/2, lvl_title, length );
      }
      else {
        if( is_incognito( wch, ch ) ) {
          sprintf( buf, "[    ???   %-4s ]",
            race_table[wch->shdata->race].abbrev );
        }
        else {
          sprintf( buf, "[ %3d %-4s %-4s ]", wch->shdata->level,
            clss_table[wch->pcdata->clss].abbrev,
            race_table[wch->shdata->race].abbrev );
        }
      }
        
      /* -- PUIOK 29/12/1999 -- Color Titles fix */
      {
        char* letter;
        int   invisibles;
        
        letter = wch->pcdata->title;
        for( invisibles = 0; *letter != '\0'; letter++ ) {
          if( *letter == '@' ) {
            letter++;
            
            switch( *letter ) {
              case '\0':
              case '@':
                invisibles++;
                break;
              case 'I': 
                break;
              default:
                invisibles += 2;
                break;
            }
            if( *letter == '\0' )
              break;
          }
        }
        
        strcpy( player_title, wch->pcdata->title );
        if( is_set( &wch->status, STAT_AFK ) /* &&
          PUT FLAG LEVEL CHECKS HERE */ )
        {
          sprintf( tmp, " @R%%s@n%%-%ds@n  %%s\n\r", 
            52-strlen( wch->descr->name ) + invisibles );

          letter = next_visible( player_title, TRUE );
          if( *letter == '\'' || *letter == ',' )
            if( letter > player_title && *--letter == 'C' )
              *letter = 'R';          
        }
        else
          sprintf( tmp, " @C%%s@n%%-%ds@n  %%s\n\r", 
            52-strlen( wch->descr->name ) + invisibles );
      }

      if( is_set( &wch->status, STAT_AFK ) /* &&
        PUT FLAG LEVEL CHECKS HERE */ )
      {
        sprintf( extras, "AFK" );
      }
      else
      {
        if( imm != NULL && imm->wizinvis > 0 && is_set(
          wch->pcdata->pfile->flags, PLR_WIZINVIS ) )
          sprintf( extras, "%d", imm->wizinvis );
        else
          sprintf( extras, "   " );
      }
      
      sprintf( buf+17, tmp, wch->descr->name,
        player_title, extras );

/* Color Titles - Zemus - Dec 28 */
      
      convert_to_ansi( ch, buf, colored );

      page( ch, colored );
    }
  }
  
  if( count > max_players )
    max_players = count; 
    
  page( ch, "%28s[ %d visible | %d high ]\n\r", "", count, max_players );
  page( ch,scroll_line[0] );
}

/* protected who - zemus april 20 */
void w3_protected_who( )
{
/*
 int             idle;
 char             tmp [ MAX_STRING_LENGTH ];
 char            host [ THREE_LINES ];
 player_data*      pc;
 FILE*             fp;

 fp = open_file( FILE_DIR, "who.txt", "w+" );

 fprintf( fp, "%-20s %5s %s\n", "Player", "Idle", "Host" );
 fprintf( fp, "%-20s %5s %s\n", "------", "----", "----" );

 for( int i = 0; i < player_list; i++ ) {
    pc = player_list[i];
    if( !pc->Is_Valid( ) )
      continue;

    if( pc->link == NULL )
      sprintf( host, "[linkdead]" );
    else
      sprintf( host, pc->link->host );

    idle = current_time - pc->timer;
    sprintf( tmp, "%-20s %5d %s", pc->real_name(), idle, host );
    fprintf( fp, "%s\n", tmp );
  }

 fclose( fp );
*/
}

/* w3_who now in html - zemus - april 18 */
void w3_who( )
{
/*
  char_data*            ch;
  link_data*          link;
  FILE*                 fp;
  char                 buf [ THREE_LINES ];  
  int                 count = 0;

  if( ( fp = open_file( W3_DIR, "who.html", "w+" ) ) == NULL )
    return;

  fprintf( fp, "<html>\n" );
  fprintf( fp, "<title>Players on The River's Edge</title>\n" );
  fprintf( fp, "<center><h3>Players</h3></center><br>\n" );

  for( link = link_list; link != NULL; link = link->next ) {
     
     if( link == NULL )
       continue;

     if( link->player == NULL )
       continue;
 
     ch = link->player;
     if( ch->shdata->level >= LEVEL_APPRENTICE )
       continue;

     sprintf( buf, "[ %3d %-4s %-s ] %s", ch->shdata->level,
  clss_table[ ch->pcdata->clss ].abbrev, race_table[ ch->shdata->race ].abbrev,
   ch->descr->name );

     count++;

     fprintf( fp, "%s<br>\n" );

     }

  fprintf( fp, "<center>%d players</center><br>\n", count );
  fprintf( fp, "</html>" );

 fclose( fp );
*/
}  


/*
 *   USERS ROUTINE
 */


bool Wizard_Data :: See_Account( pfile_data* pfile )
{
  if( has_permission( this, PERM_PLAYERS ) || pcdata->pfile == pfile )
    return TRUE;
  
  if( pfile->level >= LEVEL_APPRENTICE )
    return FALSE;

  return has_permission( this, PERM_SITE_NAMES ); 
}


int site_compare( link_data* link1, link_data* link2 )
{
  const char     *s1, *s2;
  const char *ptr1, *ptr2;
  const char *tok1, *tok2;
  bool         bln1, bln2;
  int                 cmp;
  
  if( link1 == link2 )
    return 0;
  
  /* Compare Hostnames */
  
  s1 = ptr1 = link1->host;
  s2 = ptr2 = link2->host;
  
  for( ; *ptr1 != '\0'; *ptr1++ );
  for( ; *ptr2 != '\0'; *ptr2++ );

  for( ; ; ) {
    if( ptr1 <= s1 || ptr2 <= s2 ) {
      if( ptr1 <= s1 && ptr2 <= s2 )
        break;
      return ptr1 <= s1 ? -1 : 1;
      }
      
    for( tok1 = ptr1--; ptr1 >= s1 && *ptr1 != '.'; ptr1-- );
    for( tok2 = ptr2--; ptr2 >= s2 && *ptr2 != '.'; ptr2-- );
    
    cmp = strncmp( ptr1+1, ptr2+1, min( tok1-ptr1-1, tok2-ptr2-1 ) );
    if( cmp != 0 )
      return cmp;
    
    if( tok1-ptr1 != tok2-ptr2 )
      return tok1-ptr1 < tok2-ptr2 ? -1 : 1;
    }
  
  /* Compare Idents */
  
  bln1 = (const char*) ( link1->ident.userid == NULL
    || !strcmp( link1->ident.userid, IDENTERR_NET )
    || !strcmp( link1->ident.userid, IDENTERR_GEN ) );

  bln2 = (const char*) ( link2->ident.userid == NULL
    || !strcmp( link2->ident.userid, IDENTERR_NET )
    || !strcmp( link2->ident.userid, IDENTERR_GEN ) );
  
  if( bln1 || bln2 ) {
    if( !bln1 || !bln2 )
      return bln1 ? -1 : 1;
    }
  else {
    cmp = strcmp( link1->ident.userid, link2->ident.userid );
    if( cmp != 0 )
      return cmp;
    }
  
  /* Compare Names */
  
  bln1 = (const char*) link1->player == NULL;
  bln2 = (const char*) link2->player == NULL;
  
  if( bln1 || bln2 ) {
    if( !bln1 || !bln2 )
      return bln1 ? -1 : 1;
    }
  else {
    cmp = strcmp( link1->player->descr->name,
      link2->player->descr->name );
    if( cmp != 0 )
      return cmp;
    }

  return 0;
}


void display_user( char_data* ch, link_data* link, int flags )
{
  char            tmp  [ THREE_LINES ];
  char           tmp2  [ THREE_LINES ];
  player_data* victim = link->player;
  
  if( !is_god( ch ) && !has_permission( ch, PERM_SITE_NAMES )
    && ( victim == NULL || !can_see_who( ch, victim ) ) )
    return;

  if( is_set( &flags, 0 ) ) {
    if( victim == NULL )
      sprintf( tmp, "[none]\n\r" );
    else
      sprintf( tmp, "%-15s   %s\n\r", victim->descr->name,
        victim->descr->singular );          
    }
  else if( is_set( &flags, 1 ) ) {
    if( victim == NULL )
      sprintf( tmp, "[none]\n\r" );
    else {
      sprintf( tmp,
        "%-18s%4s %4s  %2s %4d   %3d %3d   %5d %5d %5d   %-7d\n\r",
        victim->descr->name,
        clss_table[victim->pcdata->clss].abbrev,
        race_table[victim->shdata->race].abbrev,
        alignment_table[ victim->shdata->alignment ].abbrev,
        victim->shdata->level,
        victim->pcdata->trust, (int) current_time-victim->timer,
        victim->max_hit, victim->max_mana, victim->max_move,
        victim->bank );
      }
    }
  else if( is_set( &flags, 2 ) ) {
    if( victim == NULL )
      sprintf( tmp, "[none]\n\r" );
    else
      sprintf( tmp, "%-15s  %4d  %s\n\r",
        victim->descr->name, (int) current_time-victim->timer, "??" );          
    }
  else if( is_set( &flags, 3 ) ) {
    if( is_admin( ch ) || victim == ch
      || has_permission( ch, PERM_SITE_NAMES ) ) {
      if( link->ident.userid == NULL
        || !strcmp( link->ident.userid, IDENTERR_NET )
        || !strcmp( link->ident.userid, IDENTERR_GEN ) )
        strcpy( tmp2, "" );
       else
      if( strcmp( link->ident.userid, IDENTERR_IDT ) == 0)
        strcpy( tmp2, link->ident.token );
      else
        sprintf( tmp2, "%s (%s)", link->ident.userid, link->ident.token );
        truncate( tmp2, 27 );
        }
    else
      strcpy( tmp2, "(protected)" );
    sprintf( tmp, "%-15s   %-30s   %s\n\r",
      victim == NULL ? "(Logging In)" : victim->descr->name,
      is_admin( ch ) || victim == ch
      || has_permission( ch, PERM_SITE_NAMES )
      || ( victim != NULL && victim->shdata->level < LEVEL_APPRENTICE )
      ? &link->host[ max( 0, strlen( link->host )-30 ) ] :
      "(protected)", tmp2 );
    }
  else if( is_set( &flags, 4 ) ) {
    if( is_admin( ch ) || victim == ch
      || has_permission( ch, PERM_SITE_NAMES ) ) {
      if( link->ident.userid == NULL
        || !strcmp( link->ident.userid, IDENTERR_NET )
        || !strcmp( link->ident.userid, IDENTERR_GEN ) )
        strcpy( tmp2, " -- " );
      else
      if( strcmp( link->ident.userid, IDENTERR_IDT ) == 0)
        strcpy( tmp2, link->ident.token );
      else
        sprintf( tmp2, "%s", link->ident.userid, link->ident.token );
        }
    else
      strcpy( tmp2, "[none]" );
    sprintf( tmp2, "%-12s  %s", tmp2,
      is_admin( ch ) || victim == ch || has_permission( ch, PERM_SITE_NAMES )
      || ( victim != NULL && victim->shdata->level < LEVEL_APPRENTICE )
      ? &link->host[ max( 0, strlen( tmp2 ) +
      strlen( link->host )-(THREE_LINES - 1)) ]
      : "(protected)" );
    sprintf( tmp, "%-15s   %s\n\r",
      victim == NULL ? "[none]" : victim->descr->name, tmp2 );
    }
  else {
    sprintf( tmp, "%-15s   %-30s   %s",
      victim == NULL ? "(Logging In)" : victim->descr->name,
      is_admin( ch ) || victim == ch || has_permission( ch, PERM_SITE_NAMES )
      || ( victim != NULL && victim->shdata->level < LEVEL_APPRENTICE ) 
      ? &link->host[ max( 0, strlen( link->host )-30 ) ] : "(protected)",
      ( victim == NULL || victim->array == NULL )
      ? "(nowhere)" : victim->array->where->Location( ) );
     truncate( tmp, 78 );
    strcat( tmp, "\n\r" );
    }
  page( ch, tmp );
}



void do_users( char_data* ch, char* argument )
{
  char                     tmp  [ THREE_LINES ];
  link_data*              link;
  int                    flags;
  int                   length;
  player_data*          victim;
  Array<link_data*> link_array;
  bool               unswapped;

  if( !get_flags( ch, argument, &flags, "aiwdvs", "Users" ) )
    return;

  length = strlen( argument );

  if( is_set( &flags, 0 ) ) {
    sprintf( tmp, "%-15s   %s\n\r",
      "Name", "Appearance" );
    }
  else if( is_set( &flags, 1 ) ) {
    sprintf( tmp, "%-18s%4s %4s %3s %4s   %3s %3s   %5s %5s %5s   %-7s\n\r",
      "Name", "Clss", "Race", "Ali", "Lvl", "Trs", "Idl",
      "Hits ", "Enrgy", "Move ", "Bank" );
    }
  else if( is_set( &flags, 2 ) ) {
    sprintf( tmp, "%-15s  %4s  %s\n\r", "Player", "Idle", "What?" );
    }
  else if( is_set( &flags, 3 ) ) {
    sprintf( tmp, "%-15s   %-30s   %s\n\r", "Player", "Site", "User" );
    }
  else if( is_set( &flags, 4 ) ) {
    sprintf( tmp, "%-15s   %-12s  %s\n\r", "Player", "User", "Site" );
    }
  else {
    sprintf( tmp, "%-15s   %-30s   %s\n\r",
      "Player", "Site", "Location" );
    }

  page_underlined( ch, tmp );

  if( is_set( &flags, 5 ) ) {
    for( link = link_list; link != NULL; link = link->next )
      link_array += link;
    for( int i = 0, j = link_array.size - 1; i < j; i++, j-- ) {
      for( int k = 0; k < 2; k++ ) {
        unswapped = TRUE;
        for( int l = !k ? i : j - 1; !k ? l < j : l >= i; !k ? l++ : l-- )
          if( site_compare( link_array[l], link_array[l + 1] ) > 0 ) {
            swap( link_array.list[l], link_array.list[l + 1] );
            unswapped = FALSE;
            }
        if( unswapped )
          break;
        }
      }
    for( int i = 0; i < link_array; i++ ) {
      victim = link_array[i]->player;
      if( !strncasecmp( argument,  victim == NULL ?
        "" : victim->descr->name, length ) )
        display_user( ch, link_array[i], flags );
      }
    }
  else {
    for( link = link_list; link != NULL; link = link->next ) {
      victim = link->player;
      if( !strncasecmp( argument,  victim == NULL ?
        "" : victim->descr->name, length ) )
        display_user( ch, link, flags );
      }
    }
}


/*
 *   HOMEPAGE
 */


void do_homepage( char_data* ch, char* argument )
{
  if( is_mob( ch ) ) 
    return;

  if( *argument == '\0' ) {
    send( ch, "What is your homepage address?\n\r" );
    return;
    } 

  if( strlen( argument ) > 60 ) {
    send( ch, "You homepage address must be less than 60 characters.\n\r" );
    return;
    } 

  free_string( ch->pcdata->pfile->homepage, MEM_PFILE );
  if( !strcasecmp( argument, "none" ) )
    ch->pcdata->pfile->homepage = empty_string;
  else
    ch->pcdata->pfile->homepage = alloc_string( argument, MEM_PFILE );

  send( ch, "Your homepage is set to %s.\n\r", ch->pcdata->pfile->homepage );
}  


void do_email( char_data* ch, char* argument ) /* added by Letharin ) 9/1/2000(d/m/y) */
{
/* commented out due to pfile.. at least so i was told..
  if( is_mob( ch ) )
    return;

  if( *argument == '\0' ) {
    send( ch, "What is your E-mail address?\n\r" );
    return;
    } 

  if( strlen( argument ) > 60 ) {
    send( ch, "You E-mail address must be less than 60 characters.\n\r" );
    return;
    } 

  free_string( ch->pcdata->pfile->email, MEM_PFILE );
  ch->pcdata->pfile->email = alloc_string( argument, MEM_PFILE );
*/
}


/* LETHARIN 9/1/2000, changed PUIOK 11/3/2000 - pfile search */
void do_icq( char_data* ch, char* argument )
{
  int         flags;
  pfile_data* pfile;
  
  if( is_mob( ch ) )
    return;

  if( !get_flags( ch, argument, &flags, "s", "Icq" ) )
    return;
  
  if( is_set( &flags, 0 ) )
  {
    if( *argument == '\0' )
    {
      free_string( ch->pcdata->pfile->icq, MEM_PFILE );
      ch->pcdata->pfile->icq = empty_string;
      
      send( ch, "Your ICQ has been set to nothing.\n\r" );
      return;
    }
    
    if( strlen( argument ) > 60 ) {
      send( ch, "Your ICQ must be less than 60 characters.\n\r" );
      return;
    }
    
    free_string( ch->pcdata->pfile->icq, MEM_PFILE );
    ch->pcdata->pfile->icq = alloc_string( argument, MEM_PFILE );
    
    send( ch, "Your ICQ has been set to %s.\n\r", ch->pcdata->pfile->icq );
    return;
  }

  if( *argument == '\0' ) {
    send( ch, "Get the icq number for who?\n\r" );
    return;
  } 
  
  if( ( pfile = find_pfile_exact( argument ) ) == NULL
    || pfile->level == 0 ) {
    send( ch, "No character by that name exists.\n\r" );
    return;
  }
  
  if( pfile->icq == empty_string )
    send( ch, "%s is hasn't specified an icq number.\n\r", pfile->name );
  else
    send( ch, "%s's icq is %s.\n\r", pfile->name, pfile->icq ); 
}

