#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "define.h"
#include "struct.h"


void   room_info          ( char_data*, room_data* );
void   show_secret        ( char_data* );
void   show_tracks        ( char_data* );
bool   show_characters    ( char_data* );
void   show               ( char_data*, thing_data* );

void   look_in            ( char_data*, char* );
void   look_in            ( char_data*, obj_data* );

void   look_at_character  ( char_data*, char_data* );
void   show_equipment     ( char_data*, char_data* );
void   show_inventory     ( char_data*, char_data* );
void   show_description   ( char_data*, char_data* );

/*
bool   show_night         ( char_data*, room_data* );
*/
/* PUIOK 21/1/2000 */
/*
bool   show_dark          ( char_data*, room_data* );
*/
void   room_title         ( char_data*, room_data*, bool );

#define is_night( x )  !isday()

/*
 *   CAN_SEE ROUTINES
 */


bool Char_Data :: Can_See( bool msg )
{
  if( position <= POS_SLEEPING || is_set( affected_by, AFF_BLIND ) ) {
    if( msg )
      send( this, "You can't see a thing!\n\r" );
    return FALSE;
    }

  return TRUE;
}


bool Room_Data :: Seen( char_data* ch )
{
  if( !ch->Can_See( ) )
    return FALSE;

  if( ch->pcdata != NULL && is_set( ch->pcdata->pfile->flags, PLR_HOLYLIGHT ) )
    return TRUE;

  if( is_dark( ch ) )
    return FALSE;

  return TRUE;
}


bool Char_Data :: Seen( char_data* ch )
{
  wizard_data* imm;

  if( !In_Game( ) )
    return FALSE;

  if( this == ch || ch == NULL )
    return TRUE;

  if( ( imm = wizard( this ) ) != NULL
    && is_set( pcdata->pfile->flags, PLR_WIZINVIS )
    && get_trust( ch ) < imm->wizinvis )
    return FALSE;

  if( !in_character )
    return TRUE;

  if( !ch->Can_See( ) )
    return FALSE;
 
  if( ch->pcdata != NULL && is_set( ch->pcdata->pfile->flags, PLR_HOLYLIGHT ) )
    return TRUE;
    
  if( in_room != NULL && in_room->is_dark( ch )
      && !is_set( affected_by, AFF_SANCTUARY )
      && ( !is_set( ch->affected_by, AFF_INFRARED ) 
	   || ( species != NULL
		&& !is_set( &species->act_flags, ACT_WARM_BLOODED ) ) ) )
    return FALSE;
  
  if( is_set( affected_by, AFF_INVISIBLE )
      && !ch->Sees_Invis( ) )
    return FALSE;
  
  if( (is_set( affected_by, AFF_CAMOUFLAGE )
       || is_set( &status, STAT_CAMOUFLAGED ) )
      && !ch->Sees_Hidden( )
      && !includes( seen_by, ch ) )
    return FALSE;
  
  if( is_set( &status, STAT_HIDING )
    && !ch->Sees_Hidden( )
    && !includes( seen_by, ch ) )
    return FALSE;

  if( !same_plane( ch, this ) )
    return FALSE;

  return TRUE;
}


bool Obj_Data :: Seen( char_data* ch )
{
  wizard_data* imm;
  if( ( wizard( ch ) == NULL
    || !is_set( ch->pcdata->pfile->flags, PLR_SHOW_NO_SHOW ) )
    && is_set( extra_flags, OFLAG_COOKIE ) )
    return FALSE;
  
  if( array == &ch->contents
    || array == &ch->wearing )
    return TRUE;

  if( !ch->Can_See( ) )
    return FALSE;
  
  if( (imm = wizard( ch )) != NULL && 
      is_set( ch->pcdata->pfile->flags, PLR_HOLYLIGHT ) )
    return TRUE;

  if( ch->in_room != NULL
    && ( ch->in_room->is_dark( ch ) ) )
  {
    if( !is_set( extra_flags, OFLAG_GLOW ) )
      return FALSE;
  }

  if( is_set( extra_flags, OFLAG_IS_INVIS ) && !ch->Sees_Invis( ) )
    return FALSE;

  return TRUE;
}


const char* bad_keyword [4] = { "night", "dark_room", "title_night", "title_dark" };


/* PUIOK 16/5/2000 -- bad Extra Description hide */
bool Extra_Data :: Seen( char_data* ch )
{
  for( int i = 0; i < 4; i++ )
    if( !strcasecmp( keyword, bad_keyword[i] ) )
      return FALSE;
  
  if( *keyword == '%' )
    return FALSE;
  
  return TRUE;
}

/* ADDED -- PUIOK 24/1/2000 -- Can't see because too dark */

bool Obj_Data :: Too_Dark( char_data* ch )
{
  wizard_data* imm;
  if( ( wizard( ch ) == NULL
    || !is_set( ch->pcdata->pfile->flags, PLR_SHOW_NO_SHOW ) )
    && is_set( extra_flags, OFLAG_COOKIE ) )
    return FALSE;

  if( array == &ch->contents
    || array == &ch->wearing )
    return FALSE;

  if( !ch->Can_See( ) )
    return FALSE;
  
  if( (imm = wizard( ch )) != NULL && 
      is_set( ch->pcdata->pfile->flags, PLR_HOLYLIGHT ) )
    return FALSE;

  if( is_set( extra_flags, OFLAG_IS_INVIS ) && !ch->Sees_Invis( ) )
    return FALSE;

  if( ch->in_room != NULL
    && ( ch->in_room->is_dark( ch ) ) )
      return TRUE;

  return FALSE;
}

bool Char_Data :: Too_Dark( char_data* ch )
{
  wizard_data* imm;

  if( !In_Game( ) )
    return FALSE;

  if( this == ch || ch == NULL )
    return FALSE;

  if( ( imm = wizard( this ) ) != NULL
    && is_set( pcdata->pfile->flags, PLR_WIZINVIS )
    && get_trust( ch ) < imm->wizinvis )
    return FALSE;

  if( !in_character )
    return FALSE;

  if( !ch->Can_See( ) )
    return FALSE;
 
  if( ch->pcdata != NULL && is_set( ch->pcdata->pfile->flags, PLR_HOLYLIGHT ) )
    return FALSE;
    
  if( is_set( affected_by, AFF_INVISIBLE )
      && !ch->Sees_Invis( ) )
    return FALSE;

  if( (is_set( affected_by, AFF_CAMOUFLAGE )
       || is_set( &status, STAT_CAMOUFLAGED ) )
      && !ch->Sees_Hidden( )
      && !includes( seen_by, ch ) )
    return FALSE;

  if( is_set( &status, STAT_HIDING )
    && !ch->Sees_Hidden( )
    && !includes( seen_by, ch ) )
    return FALSE;

  if( !same_plane( ch, this ) )
    return FALSE;

  if( in_room != NULL && in_room->is_dark( ch )
      && !is_set( affected_by, AFF_SANCTUARY )
      && ( !is_set( ch->affected_by, AFF_INFRARED ) 
	   || ( species != NULL
		&& !is_set( &species->act_flags, ACT_WARM_BLOODED ) ) ) )
    return TRUE;

  return FALSE;
}

/* -- END PUIOK */

/*
 *   ATTRIBUTES
 */


bool char_data :: detects_evil( )
{
  return( is_set( affected_by, AFF_TRUE_SIGHT ) 
    || is_set( affected_by, AFF_DETECT_EVIL ) );
}


bool char_data :: detects_good( )
{
  return( is_set( affected_by, AFF_TRUE_SIGHT ) 
    || is_set( affected_by, AFF_DETECT_GOOD ) );
}


bool Char_Data :: Sees_Invis( )
{
  if( pcdata != NULL
    && is_set( pcdata->pfile->flags, PLR_HOLYLIGHT ) )
    return TRUE;

  return( is_set( affected_by, AFF_SEE_INVIS ) 
    || is_set( affected_by, AFF_TRUE_SIGHT ) );
}


bool Char_Data :: Sees_Hidden( )
{
  if( pcdata != NULL
    && is_set( pcdata->pfile->flags, PLR_HOLYLIGHT ) )
    return TRUE;

  return( is_set( affected_by, AFF_DETECT_HIDDEN ) 
    || is_set( affected_by, AFF_SENSE_LIFE ) );
}


/* 
 *   SHOW OBJECT ROUTINES
 */


void page( char_data* ch, thing_array& array )
{
  thing_data*   thing;
  bool        nothing  = TRUE;

  //Set the list of selected things to the list of things
  select( array );
  //Compile the list so that like looking items are grouped together.
  rehash( ch, array );

  /* Step through the array, 'pagin' the items to the character.  If nothing
     is shown, page 'nothing' to the character */
  for( int i = 0; i < array; i++ ) {
    thing = array[i];
    if( thing->shown > 0 ) {
      nothing = FALSE;
      page( ch, "  %s\n\r", thing );
      }
    }

  //If nothing is displayed .. send nothing :)
  if( nothing )
    page( ch, "  nothing\n\r" );
}


void send( char_data* ch, thing_array& array )
{
  thing_data*   thing;
  bool        nothing  = TRUE;

  select( array );
  rehash( ch, array );

  for( int i = 0; i < array; i++ ) {
    thing = array[i];
    if( thing->shown > 0 ) {
      nothing = FALSE;
      send( ch, "  %s\n\r", thing );
      }
    }

  if( nothing )
    send( ch, "  nothing\n\r" );
}


/* 
 *   TRACK AND SEARCH ROUTINES
 */


void show_secret( char_data* ch ) 
{
  exit_data*      exit;
  action_data*  action;

  if( ch->pcdata == NULL
    || !is_set( ch->pcdata->pfile->flags, PLR_SEARCHING )  
    || ch->move == 0 )
    return; 

  ch->move--;

  for( int i = 0; i < ch->in_room->exits; i++ ) {
    exit = ch->in_room->exits[i];
    if(  is_set( &exit->exit_info, EX_SEARCHABLE )
      && is_set( &exit->exit_info, EX_CLOSED )
      && is_set( &exit->exit_info, EX_SECRET ) 
      && !includes( ch->seen_exits, exit )
      && ch->check_skill( SKILL_SEARCHING ) ) {
      send( ch, "\n\r%s>> You detect something unusual %s. <<%s\n\r",
        bold_v( ch ), dir_table[ exit->direction ].name, c_normal( ch ) );
      ch->seen_exits += exit;
      }
    }
  
  for( action = ch->in_room->action; action != NULL; action = action->next )
    if( action->trigger == TRIGGER_SEARCHING && number_range( 0, 10 ) < 5 )
    {
      send( ch, "\n\r%s>> You detect something hidden here. <<%s\n\r",
        bold_v( ch ), c_normal( ch ) );
      break;
    }
}


/*
 *   LOOK AT
 */


void Char_Data :: Look_At( char_data* ch )
{
  known_by += ch;

  if( ch != this ) {
    if( ch->Seen( this ) )
      send( this, "%s looks at you.\n\r", ch );
    send_seen( ch, "%s looks at %s.\n\r", ch, this );
    }
  else {
    send_seen( ch, "%s looks at %sself.\n\r",
      ch, ch->Him_Her( ) );
    }
  
  show_description( ch, this );
  page( ch, scroll_line[0] );
  glance( ch, this );
  page( ch, "\n\r" );
  show_equipment( ch, this );
}


bool short_appearance( char_data* ch, exit_data* exit )
{
  char         tmp [ MAX_STRING_LENGTH ];
  char*        ptr;
  const char* desc;
  room_data*  room = exit->to_room;
    
  if( ( desc = one_extra_desc( &room->extra_descr, "%appearance" ) ) == NULL )
    return FALSE;

  for( ptr = tmp; *desc != '\0' && *desc != '\n'; *ptr++ = *desc++ ) ;
  *ptr = '\0';

  if( *tmp == '\0' )
    return FALSE;

  send( ch, "To the %s is %s.\n\r", dir_table[ exit->direction ].name, tmp );
  return TRUE;
}


void Exit_Data :: Look_At( char_data* ch )
{
  if( !is_set( &exit_info, EX_ISDOOR ) ) {
    if( short_appearance( ch, this ) )
      return;
    send( ch, "To the %s is %s.\n\r",
       dir_table[ direction ].name,
       to_room->name );
    }
  else {   
    send( ch, "The %s is %s.\n\r", name,
      is_set( &exit_info, EX_CLOSED ) ? "closed" : "open" );
    }
}


void Extra_Data :: Look_At( char_data* ch )
{
  char tmp [ MAX_STRING_LENGTH ];

  convert_to_ansi( ch, text, tmp );
  send( ch, tmp );
}

   


/* 
 *   SHOW CHARACTER FUNCTIONS
 */


void do_peek( char_data* ch, char* argument )
{
  char_data *victim;

  if( !ch->Can_See( TRUE ) )
    return;

  if( ( victim = one_character( ch, argument, "peek at",
    ch->array ) ) == NULL )
    return;

  show_equipment( ch, victim );
  page( ch, "\n\r" );
 
  if( ch->get_skill( SKILL_PEEK ) != UNLEARNT )
    {
      show_inventory( ch, victim );
      ch->improve_skill( SKILL_PEEK );
    }

  return;
}


void do_qlook( char_data *ch, char *argument )
{
  char_data* victim;

  if( !ch->Can_See( TRUE ) )
    return;

  if( *argument == '\0' ) {
    send( ch, "Look quickly at whom?\n\r" );
    return;
    }

  if( ( victim = one_character( ch, argument, "look quickly at",
    ch->array ) ) == NULL )
    return;

  show_description( ch, victim );

  return;
}


void show_inventory( char_data* ch, char_data* victim )
{
  if( ch == victim ) 
    page( ch, "You are carrying:\n\r" );
  else 
    page( ch, "%s is carrying:\n\r", victim );

  page( ch, victim->contents );

  return;
}


void show_description( char_data* ch, char_data* victim )
{
  char tmp  [ 3*MAX_STRING_LENGTH ];
 
  if( *victim->descr->complete != '\0' ) {
    convert_to_ansi( ch, victim->descr->complete, tmp );
    page( ch, tmp );
  }
  else
  {
    fpage( ch, "You see nothing special about %s.  In fact, you doubt %s has any special distinguishing characteristics at all.",
      victim->Him_Her( ), victim->He_She( ) );
  }
  return;
}


/*
 *   EQUIPMENT 
 */


void do_equipment( char_data* ch, char* )
{
  show_equipment( ch, ch );
}


void show_equipment( char_data* ch, char_data* victim )
{
  char*       format  = "%-22s %-42s %s\n\r";
  char*          tmp  = static_string( );
  bool         found  = FALSE;
  obj_data**    list  = (obj_data**) victim->wearing.list;
  int           i, j;

  for( i = 0; i < victim->wearing; i = j ) { 

    if( victim != ch )  
      for( j = i+1; j < victim->wearing
        && list[i]->position == list[j]->position; j++ )
        if( is_set( list[j]->pIndexData->extra_flags, OFLAG_COVER ) )
          i = j;

    for( j = i; j < victim->wearing
      && list[i]->position == list[j]->position; j++ ) {
      if( ch == victim || list[j]->Seen( ch ) ) {
        if( !found ) {
          page_centered( ch, "+++ Equipment +++" );
          sprintf( tmp, format, "Body Location", "Item", "Condition" );
          page_underlined( ch, tmp );
          found = TRUE;
          }
        page( ch, format,
          j == i ? where_name[ list[i]->position ] : "", 
          list[j]->Name( ch, 1, TRUE ),
          list[j]->condition_name( ch, TRUE ) );
        }
      }
    }

  if( !found ) {
    if( ch == victim )
      page( ch, "You have nothing equipped.\n\r" );
    else
      page( ch, "%s has nothing equipped.\n\r", victim );
    }
  else
  {
    if( ch == victim ) 
      page( ch, "\n\rWeight: %.2f lbs\n\r", 
        (float) ch->wearing.weight/100 );
  }
}

/*
 *   LOOK IN OBJECT
 */


thing_data* custommsg( thing_data* thing, char_data* ch, thing_data* )
{
  obj_data* obj;
  
  if( ( obj = object( thing ) ) != NULL ) {
    switch( obj->pIndexData->item_type ) {
      case ITEM_FOUNTAIN  :
        if( send_extra( ch, &obj->pIndexData->extra_descr, "%lookin" ) )
          return NULL;
    }
  }

  return thing;
}

thing_data* cant( thing_data* thing, char_data*, thing_data* )
{
  return object( thing );
}


thing_data* notcontainer( thing_data* thing, char_data* ch, thing_data* )
{
  obj_data* obj = (obj_data*) thing;

  switch( obj->pIndexData->item_type ) {
    case ITEM_KEYRING   :
    case ITEM_CONTAINER :
    case ITEM_CORPSE    :
      return obj;
    }
    
  return NULL;
}


thing_data* closed( thing_data* thing, char_data*, thing_data* )
{
  obj_data* obj = (obj_data*) thing;

  if( obj->pIndexData->item_type == ITEM_CONTAINER 
    && is_set( &obj->value[1], CONT_CLOSED ) )
    return NULL;

  return obj;
}
 
    
thing_data* empty( thing_data* thing, char_data*, thing_data* )
{
  return( is_empty( thing->contents ) ? (thing_data*) NULL : thing );
}


thing_data* lookin( thing_data* thing, char_data*, thing_data* )
{
  return thing;
}


void look_in( char_data* ch, char* argument )
{
  thing_array* array;

  if( *argument == '\0' ) {
    send( ch, "Look in what?\n\r" );
    return;
    }

  if( ( array = several_things( ch, argument,
    "look in", &ch->contents, ch->array, &ch->wearing ) ) == NULL ) 
    return;

  thing_array   subset  [ 6 ];
  thing_func*     func  [ 6 ]  = { custommsg, cant, notcontainer, closed,
                                   empty, lookin };

  sort_objects( ch, *array, NULL, 6, subset, func );

  page_priv( ch, NULL, empty_string );
  page_priv( ch, &subset[1], "can't look in" );
  page_priv( ch, &subset[2], NULL, NULL,
    "isn't a container", "aren't containers" );
  page_priv( ch, &subset[3], NULL, NULL, "is closed", "are closed" );
  page_priv( ch, &subset[4], NULL, NULL, "is empty", "are empty" );

  for( int i = 0; i < subset[5]; i++ ) 
    look_in( ch, (obj_data*) subset[5].list[i] );

  delete array;
}


void look_in( char_data* ch, obj_data* obj )
{
  switch( obj->pIndexData->item_type ) {
    default:
      page( ch, "%s is not a container.\n\r", obj );
      return;

    case ITEM_SPELLBOOK :
      page( ch, "The spellbook is blank.\n\r" );
      return;

    case ITEM_DRINK_CON:
      page( ch, "It's %sfull of %s.\n\r",
        obj->value[1] < obj->pIndexData->value[0] / 2 ? "less than half "
        : ( obj->value[1] < 3* obj->pIndexData->value[0] / 4
        ? "more than half " : "" ),
        obj->value[2] < 0 || obj->value[2] >= table_max[TABLE_LIQUID]
        ? "[BUG]" : liquid_table[obj->value[2]].color );
      return;

    case ITEM_KEYRING :
    case ITEM_CONTAINER:
    case ITEM_CORPSE :
      break;
    }

  page( ch, "%s contains:\n\r", obj );
  page( ch, obj->contents );
}


/*
 *   MAIN LOOK ROUTINE
 */


void do_look( char_data* ch, char* argument )
{
  visible_data* visible;

  if( ch->link == NULL || !ch->Can_See( TRUE ) )
    return;

  if( *argument == '\0' ) {
    show_room( ch, ch->in_room, FALSE, "-n" );
    return;
    }

  if(  !strncasecmp( argument, "in ", 3 ) 
    || !strncasecmp( argument, "i ", 2 ) ) {
    argument += ( argument[1] == 'n' ? 3 : 2 );
    look_in( ch, argument );
    return;
    }

  if( !strncasecmp( argument, "at ", 3 ) )
    argument += 3;

  if( ( visible = one_visible( ch, argument, "look at",
    (visible_array*) ch->array,
    (visible_array*) &ch->contents,
    (visible_array*) &ch->wearing,  
    (visible_array*) &ch->in_room->extra_descr,
    (visible_array*) &ch->in_room->exits ) ) == NULL )
    return;

  visible->Look_At( ch );
}

/*  PUIOK -- changed my mind, 21/1/2000, clear if more than 2 weeks old
bool show_night( char_data* ch, room_data* room )
{
  char          tmp [ 3*MAX_STRING_LENGTH ];
  char*        desc;

  if( ( desc = one_extra_desc( &room->extra_descr, "night" ) ) != NULL )
  {
    convert_to_ansi( ch, desc, tmp );
    send( ch, tmp );
    return TRUE;
  }
  
  return FALSE;
}

bool show_dark( char_data* ch, room_data* room )
{
  char          tmp [ 3*MAX_STRING_LENGTH ];
  char*        desc;

  if( ( desc = one_extra_desc( &room->extra_descr, "dark_room" ) ) != NULL )
  {
    convert_to_ansi( ch, desc, tmp );
    send( ch, tmp );
    return TRUE;
  }
  
  return FALSE;
}
*/

/* ADDED -- PUIOK 24/1/2000 */

void darken_description( char_data* ch, room_data* room, const char* input,
  char* output, int buffer_size )
{
  const char* complete = "The area is very dark and you can make out no details.";
  int        sentences = 0;
  int          written = 0;
  int           length;
  const char      *ptr;
  
  ptr = input;
  while( length = sentence_length( (const char*) ptr ) )
  {
    sentences++;
    ptr += length;
  }

  sentences = (int) ( sentences
    * ( (float) room->Light( 1 ) / ( (int) exp( 3 ) - 1 ) ) + 0.5 );
  
  buffer_size -= 2;  /* reserve 2 for EOL marker */ 
  
  ptr = input;
  for( ; sentences > 0; sentences-- )
  {
    length = sentence_length( ptr );
    
    if( written + length < buffer_size ) {
      strncpy( output, ptr, length );
      output += length;
      ptr += length;
      written += length;
    }
    else
      break;
  }

  if( written == 0 )
  {
    if( ( length = strlen( complete ) ) >= buffer_size )
      length = buffer_size - 1;
      
      strncpy( output, complete, length );
      output += length;
  }
  strcpy( output, "\n\r\0" );
}


extern char_data* forcing_char;

/* PUIOK 24/1/2000
 * no_shows: t = title, i = infobox, d = description, b = builder's comments
             s = secrets, r = tracks, c = contents, n = autoscan,
             k = dark(always lit), l = light(make dark),
             g = night(don't use night extra)
             y = day(try to use night regardless of time)
 */

void show_room( char_data* ch, room_data* room, bool brief, char* no_shows )
{
  char           tmp  [ 3*MAX_STRING_LENGTH ];
  char          tmp2  [ MAX_STRING_LENGTH ];
  char*         desc = NULL;
  thing_data*  thing;
  obj_data*      obj;
  int      dont_show = 0;
  bool       can_see;
  bool    show_night;
  int   hum_and_eyes = 0;

  if( ch->pcdata == NULL )
    return;
  
  if( no_shows != NULL )
    if( !get_flags( NULL, no_shows, &dont_show, "tidbsrcnklgyhe", "" ) )
      bug( "show_room(), look.cc received invalid no show flag" );
  
  can_see = room->Seen( ch ) || is_set( &dont_show, 8 );
  
  if( can_see && is_set( &dont_show, 9 ) )
    can_see = FALSE;
  
  if( !is_set( &dont_show, 0 ) )
    room_title( ch, room, can_see );
  if( !is_set( &dont_show, 1 ) )
    room_info( ch, room );

  if( !is_set( &dont_show, 2 ) )
    if( !brief || !is_set( ch->pcdata->pfile->flags, PLR_BRIEF ) )
    {
      show_night = ( is_night( room ) && !is_set( &dont_show, 10 ) ) ||
        is_set( &dont_show, 11 );
        
      if( can_see )
      {
        if( show_night )
          desc = one_extra_desc( &room->extra_descr, "night" );
        if( desc == NULL )
          desc = room->description;
      }
      else
      {
        desc = one_extra_desc( &room->extra_descr, "dark_room" );
        if( desc == NULL && show_night )
          desc = one_extra_desc( &room->extra_descr, "night" );
        if( desc == NULL )
        {
          darken_description( ch, room, (const char*) room->description,
            tmp2, MAX_STRING_LENGTH );
          desc = tmp2;
        }
      }
      convert_to_ansi( ch, desc, tmp );
      send( ch, tmp );
    }
  
  if( !is_set( &dont_show, 3 ) )
    if( is_immortal( ch ) && *room->comments != '\0' )
    {
      send( ch, "%37s-----\n\r", "" );
      send( ch, room->comments ); 
    }
  
  if( !is_set( &dont_show, 4 ) || !is_set( &dont_show, 5 )
    && !is_set( &ch->status, STAT_DREAMWALKING ))
  {
    room_data* original_room = ch->in_room;
    ch->in_room = room;
    
    if( !is_set( &dont_show, 4 ) )
      show_secret( ch );
    if( !is_set( &dont_show, 5 ) )
      show_tracks( ch );
  
    ch->in_room = original_room;
  }
  
  select( room->contents, ch, TRUE );  /* Other code might rely on this -PUIOK */
  
  if( !is_set( &dont_show, 6 ) )
  {
    for( int i = 0; i < room->contents; i++ ) 
      if( ( obj = object( room->contents[i] ) ) != NULL  
        && is_set( obj->extra_flags, OFLAG_NOSHOW )
        && ( ch->shdata->level < LEVEL_APPRENTICE
        || !is_set( ch->pcdata->pfile->flags, PLR_SHOW_NO_SHOW ) ) )  /* PUIOK 28/12/1999 */
        room->contents[i]->selected = 0; 
    
    rehash( ch, room->contents );
    
    bool found = FALSE;
    
    bool use_lifo;  /* PUIOK 27/7/2000 LIFO/FIFO option */
    if( forcing_char == NULL )
      use_lifo = ( ch == NULL || ch->pcdata == NULL
        || !is_set( ch->pcdata->pfile->flags, PLR_FIFO_ORDERING ) );
    else
      use_lifo = forcing_char->pcdata == NULL
        || !is_set( forcing_char->pcdata->pfile->flags, PLR_FIFO_ORDERING );

    int size = room->contents.size;
    for( int i = 0; i < size; i++ ) {
      thing = room->contents[ use_lifo ? ( size - i - 1 ) : i ];
      if( ( thing->shown > 0 ) && thing != ch ) {
        if( !found ) {
          found = TRUE;
          send( ch, "\n\r" );
        }
        show( ch, thing );
      }
    }
    
    /* ADDED -- PUIOK 24/1/2000 */

    if( !is_set( &dont_show, 12 ) )
      hum_and_eyes += 1;
    if( !is_set( &dont_show, 13 ) )
      hum_and_eyes += 2;
    
    if( hum_and_eyes ) {
      char*     eye_text = "of glowing eyes watch from the darkness.\n\r";
      char*     hum_text = "There is something humming here.\n\r";
      char_data* victim;
      obj_data*    obj2;
      int           hums = 0;
      int           eyes = 0;
      
      for( int i = 0; i < room->contents; i++ )
        if( !room->contents[i]->selected )
        {
          if( hum_and_eyes - 2 )
            if( ( obj2 = object( room->contents[i] ) ) != NULL )
              if( obj2->Too_Dark( ch )
                && is_set( obj2->extra_flags, OFLAG_HUM ) )
                hums++;

          if( hum_and_eyes - 1 )
            if( ( victim = character( room->contents[i] ) ) != NULL )
              if( victim->Too_Dark( ch ) && IS_AFFECTED( victim, AFF_INFRARED )
                && ( victim->species == NULL
                || is_set( &victim->species->act_flags, ACT_HAS_EYES ) ) )
                eyes++;
        }
      
      if( hums ) {
        if( !found ) {
          found = TRUE;
          send( ch, "\n\r" );
        }
        send( ch, hum_text );
      }
      
      if( eyes ) {
        send( ch, "\n\r" );
        if( eyes > 1 )
          sprintf( tmp2, "%s pairs %s", number_word( eyes ), eye_text );
        else
          sprintf( tmp2, "a pair %s", eye_text );
        tmp2[0] = toupper( tmp2[0] );
        send( ch, tmp2 );
      }
    }
    
    /* -- END PUIOK */
  }
  
  if( !is_set( &dont_show, 7 ) )
  {
    int level = level_setting( &ch->pcdata->pfile->settings, SET_AUTOSCAN );

    if( level != 0
      && ( level == 3 || !is_set( &room->room_flags, RFLAG_NO_AUTOSCAN ) ) )
    {
      room_data* original_room = ch->in_room;
      ch->in_room = room;
      do_scan( ch, "shrt" );
      ch->in_room = original_room;
    }
  }
}

void show( char_data* ch, thing_data* thing )
{
  obj_data*      obj;
  char_data*     rch;
  
  if( ( ( obj = object( thing ) ) != NULL
    && is_set( obj->extra_flags, OFLAG_NOSHOW ) )
    || ( ( rch = character( thing ) ) != NULL
    &&  !same_plane( ch, rch ) ) )
    fsend( ch, "*%s", thing->Show( ch, thing->shown ) );
  else
    fsend( ch, "%s", thing->Show( ch, thing->shown ) );
}


/*
 *   ROOM INFO BOX
 */


char* room_flags( room_data* room )
{
  char* tmp = static_string( );

  if( is_set( &room->room_flags, RFLAG_SAFE ) )
    sprintf( tmp, "safe" );
  else
    *tmp = '\0';

  if( is_set( &room->room_flags, RFLAG_NO_MOB ) ) 
    sprintf( tmp+strlen( tmp ), "%s%s",
      *tmp == '\0' ? "" : ", ", "no.mob" );

  if( is_set( &room->room_flags, RFLAG_INDOORS ) ) 
    sprintf( tmp+strlen( tmp ), "%s%s",
      *tmp == '\0' ? "" : ", ", "inside" );

  if( *tmp == '\0' )
    return "--";

  return tmp;
}

/* ADDED -- PUIOK 21/1/2000 */

void room_title( char_data* ch, room_data* room, bool can_see )
{
  char          tmp[ MAX_STRING_LENGTH ];
  char         tmp2[ TWO_LINES ];
  char*    darkness = "DARKNESS";
  char*   room_name = NULL;
  int          term  = ch->pcdata->terminal;
  int    info_level;
  int        length;

  
  if( can_see )
  {
    if( is_night( room ) )
      room_name= one_extra_desc( &room->extra_descr, "title_night" );
    if( room_name == NULL )
      room_name = room->name;
  }
  else
  {
    room_name = one_extra_desc( &room->extra_descr, "title_dark" );
    if( room_name == NULL )
      room_name = darkness;
  }

  length = strlen( room_name );
  if( length < TWO_LINES )
    strcpy( tmp2, room_name );
  else {
    strncpy( tmp2, room_name, TWO_LINES - 1 );
    tmp2[ TWO_LINES - 1 ] = '\0';
  }
  for( room_name = tmp2; *room_name != '\0'; room_name++ )
    if( *room_name == '\r' || *room_name == '\n' ) {
      *room_name = '\0';
      break;
    }
  room_name = tmp2;
  
  info_level = level_setting( &ch->pcdata->pfile->settings, SET_ROOM_INFO );
  if( info_level < 2 )
  {
    if( is_immortal( ch ) )
      send_color( ch, COLOR_ROOM_NAME, "#%d : %s\n\r", room->vnum, room_name );
    else
      send_color( ch, COLOR_ROOM_NAME, "%s\n\r", room_name );
    
    if( info_level == 1 && can_see )
      autoexit( ch, room );
    send( ch, "\n\r" );
  }
  else if( term != TERM_DUMB )
  {
    sprintf( tmp, "%%%ds%s%%s%s\n\r", 40-strlen( room_name )/2,
      term_table[term].codes( ch->pcdata->color[ COLOR_ROOM_NAME ] ), c_normal( ch ) );
    send( ch, tmp, "", room_name );
  }
  else
    send_centered( ch, room_name );
}

/* -- END PUIOK */

void room_info( char_data* ch, room_data* room )
{
  char*    tmp  = static_string( );
  int        i;
  
  if( level_setting( &ch->pcdata->pfile->settings, SET_ROOM_INFO ) < 2 )
    return;
    
  send( ch, scroll_line[2] );

  if( !is_set( &ch->status, STAT_DREAMWALKING ) )
  {
    send( ch, "|   Lighting: %-15s Time: %-16s  Terrain: %-13s |\n\r",
      light_name( room->Light(1) ),
      is_set( &room->room_flags, RFLAG_INDOORS ) ? "???" : sky_state( ),
      terrain[ room->sector_type ].name );
  }
  else
  {
    send( ch, "|   Lighting: %-15s Time: %-16s  Terrain: %-13s |\n\r",
      "Dimly Lit", "???", terrain[ room->sector_type ].name );
  }

  i = exits_prompt( tmp, ch, room );
  add_spaces( tmp, 12-i );

  if( !is_set( &ch->status, STAT_DREAMWALKING ) )
  {
    send( ch, "|      Exits: %s Weather: %-15s Room Size: %-13s |\n\r",
      tmp, is_set( &room->room_flags, RFLAG_INDOORS ) ? "???" : "Clear",
      size_name[room->size] );
  }
  else
  {
    send( ch, "|      Exits: %s Weather: %-15s Room Size: %-13s |\n\r",
      tmp, "???", size_name[room->size] );
  }

  if( is_immortal( ch ) )
    send( ch, "|       Vnum: %-14d Flags: %-40s |\n\r",
      room->vnum, room_flags( room ) );
  
  send( ch, scroll_line[2] );
}
