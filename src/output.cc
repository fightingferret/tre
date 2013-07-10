#include "ctype.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "define.h"
#include "struct.h"


const char* He_She          [] = { "it",  "he",  "she" };
const char* Him_Her         [] = { "it",  "him", "her" };
const char* His_Her         [] = { "its", "his", "her" };


const char* scroll_line[] =
{
   "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\r",
   ">>>>==--------====--<>--====--------==<<<<\n\r",
   "+=--------------------------------------------------------------------------=+\n\r"
};




/*
 *   LOW LEVEL ROUTINES
 */


void send( char_data* ch, const char* text )
{
  if( ch == NULL || ch->link == NULL )
    return;

  if( is_set( &ch->status, STAT_PAGE_ALL ) )
  {
    page( ch, text );
    return;
  }
  
  if( ch->link->character != ch ) /* PUIOK 28/12/1999 */
    switched_send( ch, text );
  else
    send( ch->link, text );

  return;
}

void send( link_data* link, const char* message )
{
  text_data* text;

  if( link != NULL ) {
    text = new text_data( message );
    append( link->send, text );
    if( link->snoop_by != NULL )
      send( link->snoop_by, message );
    }

  return;
}

/*  ADDED -- PUIOK 28/12/1999 */

void switched_send( char_data* ch, const char* text )
{
  player_data*  pc = ch->link->player;
  int         term;
  int        color;
  
  if( pc == NULL )
    return;

  term   = pc->pcdata->terminal;
  color  = pc->pcdata->color[ COLOR_MISCSET ];
  
  char tmp[ MAX_STRING_LENGTH ];
  
  if( term != TERM_DUMB )
    sprintf(tmp, "%s| %s |%s  %s",
      term_table[term].codes( color ), ch->Seen_Name( pc ),
      term_table[term].codes( pc->pcdata->color[ COLOR_DEFAULT ] ), text );
  else
    sprintf(tmp, "| %s |  %s", ch->Seen_Name( pc ), text );
  
  send( ch->link, tmp );
}

/* -- END PUIOK */


/*
 *   ACCEPT MESSAGE?
 */


bool Player_Data :: Accept_Msg( char_data* ch )
{
  if( msg_type == MSG_STANDARD )
    return TRUE;

  int level = level_setting( &pcdata->mess_settings, msg_type );

  if( level == 3 )
    return TRUE;

  if( level == 0 ) 
    return FALSE;

  if( Befriended( ch ) )
    return TRUE;

  if( level == 1 )
    return FALSE;

  return is_same_group( this, ch );
}


bool Mob_Data :: Accept_Msg( char_data* )
{
  return FALSE;
}


/* 
 *   PAGER ROUTINES
 */


/* This function takes a character and a text string as input.  It parses
   the string creating a new text_data item that is appended to the 
   end of of the character's link->paged list for output */
void page( char_data* ch, const char* txt )
{
  char          tmp  [ MAX_STRING_LENGTH ];
  text_data*   send;
  link_data*   link;
  char*        line;

  if( ch == NULL || ( link = ch->link ) == NULL )
    return;
  
  for( ; ; ) {
    for( line = tmp; *txt != '\n' && *txt != '\0'; )
      *(line++) = *(txt++);

    if( *txt != '\0' ) {
      *(line++) = '\n';
      *(line++) = '\r';
    }

    *line     = '\0';

    if( *txt == '\n' )
      if( *(++txt) == '\r' )
        txt++;
    
    if( *tmp == '\0' )
      break;

    if( ch->link->character != ch && ch->pcdata != NULL )
    {
      char     header [ MAX_STRING_LENGTH ] = "";
      int        term = ch->pcdata->terminal;
      
      if( term != TERM_DUMB )
        strcat( header, term_table[ term ].codes( ch->pcdata->color[ COLOR_MISCSET ] ) );
      sprintf( header, "%s| %s |", header, ch->Seen_Name( link->player ) );
      if( term != TERM_DUMB )
        strcat( header, term_table[ term ].codes( ch->pcdata->color[ COLOR_DEFAULT ] ) );
      strcat( header, "  " );
      
      if( strlen( tmp ) + strlen( header ) < MAX_STRING_LENGTH )
      {
        strcat( header, tmp );
        strcpy( tmp, header );
      }
    }

    send = new text_data( tmp );
    if( ch->pcdata != NULL
      && is_set( ch->pcdata->pfile->flags, PLR_CONTINUOUS )
      && !is_set( &ch->status, STAT_PAGE_ALL ) )
      append( link->send, send );
    else
      append( link->paged, send );
  }

  return;
}


void fpage( char_data* ch, const char *text )
{
  char buf [ MAX_STRING_LENGTH ];

  if( ch == NULL || ch->link == NULL )
    return;

  format( buf, text );
  strcat( buf, "\n\r" );
  corrupt( buf, MAX_STRING_LENGTH, "Fpage" );

  page( ch, buf );
}    


void clear_pager( char_data* ch )
{
  if( ch->link == NULL || ch->link->paged == NULL )
    return;

  delete_list( ch->link->paged );

  return;
}


void next_page( link_data* link )
{
  text_data*   next;
  text_data*   send;
  char*      letter;
  int         lines  = 0;

  if( link == NULL || ( send = link->paged ) == NULL )
    return;

  for( ; send->next != NULL; send = send->next ) { 
    for( letter = send->message.text; *letter != '\0'; letter++ )
      if( *letter == '\r' )
        lines++;
    if( lines > link->character->pcdata->lines-6 )
      break;
    }

  next        = send->next;
  send->next  = link->send;
  link->send  = link->paged;
  link->paged = next;

  return;
}


void page_centered( char_data* ch, const char* text )
{
  char  tmp1  [ TWO_LINES ];
  char  tmp2  [ TWO_LINES ];

/*
  if( strlen( text ) > 80 ) {
    bug( "Page_Centered: Text > 80 Characters." );
    bug( text );
    return;
    }
*/

  sprintf( tmp1, "%%%ds%%s\n\r", 40-strlen( text )/2 );
  sprintf( tmp2, tmp1, "", text );

  page( ch, tmp2 );

  return;
}


void page_underlined( char_data* ch, const char* text )
{
  char  tmp  [ TWO_LINES ];
  int     i;

  page( ch, text );

  for( i = 0; i < strlen( text ); i++ )
    if( text[i] == ' ' || text[i] == '\n' || text[i] == '\r' )
      tmp[i] = text[i];
    else
      tmp[i] = '-';

  tmp[i] = '\0';

  page( ch, tmp );
}  


/*
 *   SEND_TO_AREA
 */


void send_to_area( const char* string, area_data* area )
{
  char_data *rch;
  room_data *room;

  for( room = area->room_first; room != NULL; room = room->next )   
    for( int i = 0; i < room->contents; i++ )    
      if( ( rch = character( room->contents[i] ) ) != NULL
        && rch->link != NULL ) 
        send( rch, string );

  return;
}


/*
 *   FORMATTED SEND
 */


void fsend( char_data* ch, const char *text )
{
  char buf  [ MAX_STRING_LENGTH ];
  char buf2 [ MAX_STRING_LENGTH ];

  if( ch == NULL || ch->link == NULL )
    return;

  format( buf, text );
  strcat( buf, "\n\r" );
  corrupt( buf, MAX_STRING_LENGTH, "Fsend" );
  
  if( ch->link->character != ch && ch->pcdata != NULL )
  {
    int        term = ch->pcdata->terminal;
    
    ch->pcdata->terminal = TERM_DUMB;
    convert_to_ansi( ch, buf, buf2 );
    ch->pcdata->terminal = term;
    
    strcpy( buf, "" );
    if( term != TERM_DUMB )
      strcat( buf, term_table[ term ].codes( ch->pcdata->color[ COLOR_MISCSET ] ) );
    sprintf( buf, "%s| %s |", buf, ch->Seen_Name( ch->link->player ) );
    if( term != TERM_DUMB )
      strcat( buf, term_table[ term ].codes( ch->pcdata->color[ COLOR_DEFAULT ] ) );
    strcat( buf, "  " );
    
    if( strlen( buf2 ) + strlen( buf ) < MAX_STRING_LENGTH )
    {
      strcat( buf, buf2 );
      strcpy( buf2, buf );
    }
  }
  else
    convert_to_ansi( ch, buf, buf2 );
  
  send( ch->link, buf2 );

  return;
}    


void send_centered( char_data* ch, const char* text )
{
  char  tmp1  [ TWO_LINES ];
  char  tmp2  [ TWO_LINES ];
/*
  if( strlen( text ) > 80 ) {
    bug( "Send_Centered: Text > 80 Characters." );
    bug( text );
    return;
    }
*/
  sprintf( tmp1, "%%%ds%%s\n\r", 40-strlen( text )/2 );
  sprintf( tmp2, tmp1, "", text );

  send( tmp2, ch );

  return;
}


void send_underlined( char_data* ch, const char* text )
{
  char  tmp  [ TWO_LINES ];
  int     i;

  send( text, ch );

  for( i = 0; i < strlen( text ); i++ )
    if( text[i] == ' ' || text[i] == '\n' || text[i] == '\r' )
      tmp[i] = text[i];
    else
      tmp[i] = '-';

  tmp[i] = '\0';

  send( tmp, ch );

  return;
}  


