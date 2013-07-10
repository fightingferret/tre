#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "define.h"
#include "struct.h"

/* probe fixed */
void do_probe( char_data *ch, char *argument )
{
  char buf [ MAX_STRING_LENGTH ];
  char_data *victim;

  if( ch->pcdata == NULL || ch->shdata->skill[SKILL_PROBE] == 0 ) {
    send( "Only psionics can probe the minds of others.\n\r", ch );
    return;
    }

  if( (victim = one_character( ch, argument, "probe", ch->array )) == NULL ) {
    send( "They aren't here.\n\r", ch );
    return;
    }

  sprintf( buf, "Hits: %d/%d\n\r",
    victim->hit, victim->max_hit );
  sprintf( buf+strlen( buf ), "Energy: %d/%d\n\r",
    victim->mana, victim->max_mana );
  sprintf( buf+strlen( buf ), "Exp Value: %d\n\r\n\r",
    xp_compute( victim ) );
  send( buf, ch );

  sprintf( buf, "Alignment: %s\n\r",
    alignment_table[victim->shdata->alignment].name );
  send( buf, ch );
  return;
}

/* sweep half fixed (needs cleaning up) */
void do_sweep( char_data* ch, char* )
{
  char          buf [ MAX_STRING_LENGTH ];
  char          tmp [ MAX_STRING_LENGTH ];
  char_data*    rch;
  room_data*    room;
  int           number[ MAX_SPECIES ];
  bool          found = FALSE;
  int           i, col = 0;
  thing_data*   thing;

  if( ch->pcdata == NULL || ch->shdata->skill[SKILL_SWEEP] == 0 ) {
    send( "Only psionics can sweep an area.\n\r", ch );
    return;
    }

  for( i = 0; i < MAX_SPECIES; i++ )
    number[i] = 0;

  for( room = ch->in_room->area->room_first; room != NULL; room = room->next ) {
     for( i = 0; i < room->contents; i++ ) {
        thing = room->contents[i];
        if( thing->shown > 0 ) {
           rch = character( thing );
           if( rch == NULL )
             continue;
      if( rch == ch || !rch->Seen( ch ) )
        continue;
      if( !found ) {
        send_title( ch, "Creatures in Area" );
        found = TRUE;
        }
      if( rch->species != NULL ) {
        number[rch->species->vnum]++;    
        }
      else {
           sprintf( buf, "%s\n\r", rch->descr->name );
           send( buf, ch );
           }
        }
      }
    }

  for( i = 0; i < MAX_SPECIES; i++ ) {
    if( number[i] == 0 || species_list[i]->shdata->intelligence < 5 )
      continue;

    if( number[i] == 1 ) 
      sprintf( buf, "%s", species_list[i]->descr->name );
    else  
      sprintf( buf, "%s (x%d)",
        species_list[i]->descr->name, number[i] );

    sprintf( tmp, "%28s%s", buf, ++col%2 == 0 ? "\n\r" : ""  );
    send( tmp, ch );
    }

  if( col%2 == 1 )
    send( "\n\r", ch );

  if( !found ) 
    send( "You detect nothing in the area!?\n\r", ch );
  
  return;
}


/*
 *   SPELLS
 */


bool spell_mind_blade( char_data* ch, char_data* victim, void*,
  int level, int, int )
{
  int dam;

  if( ch == NULL ) {
    bug( "Mind_Blade: NULL caster." );
    return TRUE;
    }
 
  dam = ch->shdata->level > 20 ? 20 : ch->shdata->level;

  damage_mind( victim, ch, roll_dice( 2+level/2, dam ),
    "*The mind blade" );

  return TRUE;
}





