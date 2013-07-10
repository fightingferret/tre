#include "sys/types.h"
#include <ctype.h>
#include "stdlib.h"
#include "stdio.h"
#include "define.h"
#include "struct.h"
#include "bounty.h"

void do_bounty( char_data* ch, char* argument )
{
  char                arg [ MAX_STRING_LENGTH ];
  char                tmp [ ONE_LINE ];
  pfile_data*       pfile;
  player_data*         pc;
  int              amount;
  bool          newbounty = FALSE;
  link_data*         link;
  player_data*     victim;
  char_data*          vch = NULL;

  if( is_switched( ch ) )
    return;

  if( !is_set( &ch->in_room->room_flags, RFLAG_BOUNTY_OFFICE ) ) {
    send( ch, "You see no bounty office here.\n\r" );
    return;
    }

  if( *argument == '\0' ) {
    display_bounty( ch );
    return;
    }
 
  argument = one_argument( argument, arg );

  pfile = find_pfile( arg );
  if( pfile == NULL ) {
    send( ch, "That player does not exist.\n\r" );
    return;
    }

  for( int i = 0; i < player_list; i++ ) {
     if( player_list[i] != NULL ) {
       if( player_list[i]->pcdata->pfile == pfile ) {
         vch = player_list[i];
         continue;
         }
       }
     }

  if( vch != NULL ) {
    if( vch->pcdata == NULL ) {
      send( ch, "You can only set a bounty on players.\n\r" );
      return;
      }
    }

  pc = player( ch );

  if( pfile->level >= 201 && !has_permission( ch, PERM_PLAYERS ) ) {
    send( ch, "Adding a bounty to an immortal would have a cruel and\
 unmerciful outcome.\n\r" );
    return;
    }

  if( pfile == ch->pcdata->pfile && !has_permission( ch, PERM_PLAYERS ) ) {
    send( ch, "Adding a bounty to yourself doesn't make sense.\n\r");
    return;
    }

  if( *argument == '\0' ) {
    send( ch, "Set the bounty to what?\n\r" );
    return;
    }

  amount = atoi( argument );
  if( amount > pc->bank ) {
    send( ch, "The bounty is not accepted due to insufficient funds\
 in your bank account.\n\r" );
    return;
    }

  if( pfile->bounty == 0
    && amount <  ( 100 + min( pfile->pkcount*100, 50000 ) ) ) {
    send( ch, "The minimum bounty for %s is %d.\n\r", pfile->name,
      ( 100 + min( pfile->pkcount*100, 50000 ) ) );
    return;
    }

  if( amount < 1 && !has_permission( ch, PERM_PLAYERS ) ) {
    send( ch, "You may not set a negative bounty.\n\r" );
    return;
    }

  pc->bank -= amount;

  if( pfile->bounty == 0 )
    newbounty = TRUE;

  if( vch == NULL ) {
    link = new link_data;
    if( !load_char( link, pfile->name, PLAYER_DIR ) ) {
      bug( "do_bounty: non-existent player file" ); 
      return;
      }          
 
    victim = link->player;
    link->connected = CON_PLAYING;
    }

    pfile->bounty += amount;
    send( ch, "Bounty on %s is now %d cp.  %d cp was subtracted\
 from your bank account.\n\r", pfile->name, pfile->bounty, amount );

  free_string( pfile->hunter, MEM_PFILE );
  pfile->hunter = alloc_string( ch->descr->name, MEM_PFILE );

  if( vch == NULL ) {
    write( victim );
    victim->Extract( );
    delete link;
    }
  else
   write( player( vch ) );

  if( amount > 0 ) {
    if( newbounty ) {
      sprintf( tmp, "%s has set a %d cp bounty on %s.",
        ch->descr->name, pfile->bounty, pfile->name );
      }
    else {
      sprintf( tmp, "Bounty on %s has been increased to %d cp by %s.",
        pfile->name, pfile->bounty, ch->descr->name );
      }
    info( tmp, LEVEL_APPRENTICE, tmp, IFLAG_BOUNTY, 0 );
    }
  return;
}

void display_bounty( char_data* ch )
{
 pfile_data*      pfile;
 bool             found = FALSE;

 page( ch, "%-13s %-10s %-5s %s\n\r", "Player", "Race", "Kills", "Bounty" );
 page( ch, "%-13s %-10s %-5s %s\n\r", "------", "----", "-----", "------" );

 for( int i = 0; i < max_pfile; i++ ) {
     if( pfile_list[i]->bounty != 0 ) {
       found = TRUE;
       pfile = pfile_list[i];
       page( ch, "%-13s %-10s %5d %6d [ %s ]\n\r", pfile->name,
         race_table[ pfile->race ].name, pfile->pkcount, pfile->bounty,
         pfile->hunter == empty_string ? "Unknown" : pfile->hunter );
       }
   }

 if( !found )
   page( ch, "None\n\r" );

 return;
}

