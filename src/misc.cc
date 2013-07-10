#include "ctype.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/types.h"
#include <syslog.h>
#include "define.h"
#include "struct.h"

/* Gamestat added by zemus april 7 */

void do_gamestat( char_data* ch, char* argument )
{
 int       flags;
 char       tmp [ MAX_STRING_LENGTH ];

 if( argument == '\0' ) {
   page_title( ch, "Game Settings" );
   page( ch, "[ ] Wizlock\n\r" );
   page( ch, "\n\r" );
   }
}




/*
 *   BUG, IDEA, TYPO FILE ROUTINES
 */


void panic( const char* text )
{
  bug( text );
  bug( "** MUD KILLED **" );

  exit( 1 );
}


void bug( int level, const char* str )
{
  FILE*    fp;

  if( str == NULL || str == empty_string )
    return;

  if( level != BUG_APHID ) {
    if( ( fp = fopen( BUG_FILE, "a" ) ) != NULL ) {
      fprintf( fp, "[%s] %s\n", ltime( current_time )+4, str );
      fclose( fp );
      }
    else {
      info( "", LEVEL_APPRENTICE, "Error opening bug file.", IFLAG_BUGS, 1
);
      fprintf( stderr, "[BUG] Error opening bug file.\n" );
  
    }
    }

  if( level != -1 ) {
    fprintf( stderr, "[BUG] %s\n\r", str );
    
info( "", LEVEL_APPRENTICE, str, IFLAG_BUGS, level );
    }

  return;
}


/*
 *   DEFINE COMMAND
 */


const char* lookup( index_data* index, int number, bool plural )
{
  int i;

  for( i = 0; index[i].value < number && index[i].value != -1; i++ );

  return( plural ? index[i].plural : index[i].singular );
}


void do_define( char_data* ch, char* argument )
{
  char* name [] = { "Acid Damage", "Fire Damage", "Cold Damage",
    "Electrical Damage", "Physical Damage", "Fame", "Piety",
    "Reputation", "" };

  index_data* index [] = { acid_index, fire_index, cold_index,
    electric_index, physical_index, fame_index, piety_index,
    reputation_index };

  int           prev;
  int           i, j;

  if( *argument == '\0' ) {
    send_title( ch, "Defined Terms" );
    for( i = 0; *name[i] != '\0'; i++ ) 
      send( ch, "%20s%s", name[i], i%3 == 2 || *name[i+1] == '\0'
        ? "\n\r" : "" );
    return;
    }

  for( i = 0; *name[i] != '\0'; i++ ) {
    if( fmatches( argument, name[i] ) ) {
      page_title( ch, name[i] );
      prev = ( index[i][0].value < 0 ? -1000 : 0 );
      for( j = 0; ; j++ ) {
        if( j == 0 && index[i][0].value == 0 )
          continue;
        if( index[i][j].value == -1 ) {
          page( ch, "%39s   %d+\n\r", index[i][j].singular, prev );
          return;
          }
        if( index[i][j].value == prev )
          page( ch, "%39s   %d\n\r", index[i][j].singular, prev );
        else
          page( ch, "%39s   %d to %d\n\r",
            index[i][j].singular, prev, index[i][j].value );
        prev = index[i][j].value+1;
        }
      }
    }

  send( ch, "Unknown field - see help define.\n\r" );
}


/*
 *   TYPO COMMAND
 */


void do_typo( char_data* ch, char* argument )
{
  char tmp [ MAX_STRING_LENGTH ];

  if( *argument == '\0' ) {
    send( ch, "Room #%d\n\r\n\r", ch->in_room->vnum );
    send( ch, "What typo do you wish to report?\n\r" );
    return;
    } 

  if( strlen( ch->in_room->comments )
    > MAX_STRING_LENGTH-MAX_INPUT_LENGTH-30 ) {
    send( ch, "Comment field at max length - typo ignored.\n\r" );
    return;
    }

  ch->in_room->area->modified = TRUE;
  remove_bit( &ch->in_room->room_flags, RFLAG_STAT_CORRECTED );

  sprintf( tmp, "%s[%s] %s\n\r", ch->in_room->comments, 
    ch->real_name( ), argument );

  free_string( ch->in_room->comments, MEM_ROOM );
  ch->in_room->comments = alloc_string( tmp, MEM_ROOM );

  send( "Typo noted - thanks.\n\r", ch );

  return;
}    


/*
 *   RANDOM SUPPORT ROUTINES
 */


bool player_in_room( room_data* room )
{
  char_data* ch;

  if( room != NULL )
    for( int i = 0; i < room->contents; i++ )
      if( ( ch = character( room->contents[i] ) ) != NULL 
        && ch->pcdata != NULL )
        return TRUE;

  return FALSE;
};

int check_condition( const char*, char*, int,
  visible_data* = NULL, visible_data* = NULL, visible_data* = NULL,
  visible_data* = NULL, visible_data* = NULL, visible_data* = NULL,
  visible_data* = NULL );

/* TEMPORARY */
void do_ansi( char_data* ch, char* argument )
{
  int      value;
  char error_msg [ 512 ];
  
  if( ( value = check_condition( argument, error_msg, 78, ch ) ) < 0 )
    send( ch, error_msg );
  else
    send( ch, value ? "True\n\r" : "False\n\r" );
  
  
/*
  char tmp [ THREE_LINES ];
  obj_data *obj;
  
  if( player( ch ) == NULL ) {
    send( ch, "Go away, what do mobs know about ansi.\n\r" );
    return;
  }
  send( ch, "[1mYour terminal is NOT[4D[K ansi capable.[0m\n\r" );

  if( ch->shdata->level <= LEVEL_APPRENTICE )
    return;

  for( int i = 0; i < ch->contents; i++ )
  {
    obj = (obj_data*) ch->contents[i];
    
    sprintf( tmp, "%d: %d : %d\n", obj->pIndexData->vnum,
      obj->owner, obj->number );
    send( ch, tmp );
  }
  
  */
  
/*  
  if( ch->shdata->level > LEVEL_APPRENTICE )
  switch_bit( &ch->status, STAT_DREAMWALKING );
*/
/*  
  if( ch->shdata->level > LEVEL_APPRENTICE )
  {
    area_data*       area;
    room_data*       room;
    action_data*   action;
    char              tmp  [ THREE_LINES ];
    
    send( ch, "-- Acode with TIME TRIGGER --\n\r" );
    
    for( area = area_list; area != NULL; area = area->next ) 
      for( room = area->room_first; room != NULL; room = room->next ) 
        for( action = room->action; action != NULL; action = action->next )
          if( action->trigger == TRIGGER_TIME )
          {
            sprintf( tmp, "Room: #%d.\n\r", room->vnum );
            send( ch, tmp );
          }
    send( ch, "-- End of listing --\n\r" );
  }*/
}


/* Clipboard - Zemus - May 20 */
void do_clipboard( char_data* ch, char* argument )
{
 char              arg [ MAX_STRING_LENGTH ];
 char              tmp [ MAX_STRING_LENGTH ];
 player_data*       pc;
 wizard_data*      imm;
 bool           adding = FALSE;
 bool          copying = FALSE;

 if( ch->pcdata == NULL )
   return;

 pc = player( ch );
 
 if( !strcasecmp( argument, "clear" ) ) {
   ch->pcdata->clipboard = empty_string;
   send( ch, "Clipboard cleared.\n\r" );
   return;
   }

 argument = one_argument( argument, arg );

 if( !strcasecmp( arg, "append" ) || !strcasecmp( arg, "add" ) || !strcasecmp( arg, "copy" ) ) {

   if( !strcasecmp( arg, "copy" ) )
     copying = TRUE;

   if( !strcasecmp( arg, "add" ) )
     adding = TRUE;
 
   if( argument == empty_string ) {
     send( ch, "Syntax: clipboard append <keyword>\n\r" );
     return;
     }
 
   if( !strcasecmp( argument, "note" ) ) {
     if( pc->note_edit == NULL ) {
       send( ch, "You aren't editting a note.\n\r" );
       return;
       }
     sprintf( tmp, "%s%s", ch->pcdata->clipboard, pc->note_edit->message );
     arg = tmp;
     if( copying ) {
       pc->note_edit->message = edit_string( ch, arg, pc->note_edit->message, MEM_NOTE );
       return;
       }
     }

   if( !strcasecmp( argument, "help" ) ) {
     if( ch->pcdata->help_edit == NULL ) {
       send( ch, "You aren't editting a help file.\n\r" );
       return;
       }
     sprintf( tmp, "%s%s", ch->pcdata->clipboard, ch->pcdata->help_edit->text );
     arg = tmp;
     }

   if( !strcasecmp( argument, "mail" ) ) {
     if( ch->pcdata->mail_edit == NULL ) {
       send( ch, "You aren't editting a mail message.\n\r" );
       return;
       }
     sprintf( tmp, "%s%s", ch->pcdata->clipboard, ch->pcdata->mail_edit->message );
     arg = tmp;
     }

   if( !strcasecmp( argument, "rdesc" ) ) {
     if( ch->in_room == NULL ) {
       roach( "clipboard:in_room == NULL!? (%s)", ch->descr->name );
       return;
       }
     sprintf( tmp, "%s%s", ch->pcdata->clipboard, ch->in_room->description );
     arg = tmp;
     }

   if( !strcasecmp( argument, "ddata" ) ) {
     if( ( imm = wizard( ch ) ) != NULL ) {
       if( imm->descdata_edit == NULL ) {
         send( ch, "You aren't editting a descdata.\n\r" );
         return;
         }
       sprintf( tmp, "%s%s", ch->pcdata->clipboard, imm->descdata_edit->message );
       arg = tmp;
       }
     }

   if( !strcasecmp( argument, "acode" ) ) {
     if( ( imm = wizard( ch ) ) != NULL ) {
       if( imm->action_edit == NULL ) {
         send( ch, "You aren't editting an action.\n\r" );
         return;
         }
       sprintf( tmp, "%s%s", ch->pcdata->clipboard, imm->action_edit->code );
       arg = tmp;
       }
     }

   }

 if( adding == TRUE )
   ch->pcdata->clipboard = empty_string;

 ch->pcdata->clipboard = edit_string( ch, arg, ch->pcdata->clipboard, MEM_CLIPBOARD );

 if( ch->pcdata->clipboard == empty_string )
   send( ch, "The clipboard is empty.\n\r" );

}

/* typolist - zemus july 13 */
void do_typolist( char_data* ch, char* argument )
{
 area_data*    area;
 room_data*    room;
 char           tmp [ MAX_STRING_LENGTH ];
 int          flags;

 if( is_mob( ch ) )
   return;

 if( !get_flags( ch, argument, &flags, "g", "Typolist" ) )
   return;;

 sprintf( tmp, "%7s %s", "vnum", "Room Name" );
 page_underlined( ch, "%s\n\r", tmp );
 
 if( is_set( &flags, 0 ) ) {
   for( area = area_list; area != NULL; area = area->next )
      if( area->status == AREA_OPEN )
        for( room = area->room_first; room != NULL; room = room->next ) {
            if( !is_set( &room->room_flags, RFLAG_STAT_CORRECTED ) && room->comments != empty_string ) {
              sprintf( tmp, "[%5d] %s", room->vnum, room->name );
              page( ch, "%s\n\r", tmp );
              sprintf( tmp, room->comments );
              page( ch, "%s\n\r", tmp );
              }
           }
   }
 else {
        area = ch->in_room->area;

        for( room = area->room_first; room != NULL; room = room->next ) {
            if( !is_set( &room->room_flags, RFLAG_STAT_CORRECTED ) && room->comments != empty_string ) {
              sprintf( tmp, "[%5d] %s", room->vnum, room->name );
              page( ch, "%s\n\r", tmp );
              sprintf( tmp, room->comments );
              page( ch, "%s\n\r", tmp );
              }
            }
   }

 return;

}

