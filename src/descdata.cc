#include <sys/types.h>
#include <stdio.h>
#include "stdlib.h"
#include "define.h"
#include "struct.h"

#define DESCDATA_FILE "descdata.dbd"
#define DESCDATA_DIR "../descdata"

note_data*   descdata_list  [ MAX_DESCDATA_INDEX ];

char**       interpret_desctype           ( char_data*, char*, bool, int* );
int          find_ddescription            ( char_data*, char*, const char*, bool = FALSE );
bool         dd_checkname                 ( char_data*, char* );
bool         dd_can_edit                  ( char_data*, note_data* );
void         dd_purge                     ( int );
void         list_ddatabase               ( char_data*, char*, bool );
void         stat_ddatabase               ( char_data*, int );

extern bool help_modified;


void do_ddatabase( char_data* ch, char* argument )
{
  char               arg  [ MAX_INPUT_LENGTH ];
  char               tmp  [ MAX_STRING_LENGTH ];
  char          orig_arg  [ MAX_INPUT_LENGTH ];
  int              flags;
  char**       edit_text;
  int         data_index;
  int           mem_type;
  note_data*   desc_data;
  wizard_data*       imm;
  
  if( ( imm = wizard( ch ) ) == NULL )
    return;
  
  if( !get_flags( ch, argument, &flags, "lm", "DDatabase" ) )
    return;
  
  if( *argument == '\0' )
  {
    if( imm->descdata_edit != NULL )
    {
      page( ch, "Name: %s\n\r\n\r", imm->descdata_edit->title );
      
      imm->descdata_edit->message = edit_string( ch, argument,
        imm->descdata_edit->message, MEM_NOTE );
    }
    else
      list_ddatabase( ch, "", is_set( &flags, 1 ) );
    return;
  }
  
  strcpy( orig_arg, argument );
  argument = one_argument( argument, arg );
  
  if( imm->descdata_edit == NULL && !strcmp( arg, "cp" ) )
  {
    argument = one_argument( argument, arg );
        
    if( ( data_index = find_ddescription( ch, arg, "Copy" ) ) < 0 )
      return;
    
    desc_data = descdata_list[data_index];
    
    edit_text = interpret_desctype( ch, argument, TRUE, &mem_type );
    if( edit_text == NULL )
      return;
    
    free_string( *edit_text, mem_type );
    *edit_text = alloc_string( desc_data->message, mem_type );
    
    send( ch, "Description copied.\n\r" );
  }
  else if( !strcmp( arg, "rn" ) || !strcmp( arg, "title" ) )
  {
    if( ( desc_data = imm->descdata_edit ) == NULL )
    {
      argument = one_argument( argument, arg );
  
      if( ( data_index = find_ddescription( ch, arg, "Rename" ) ) < 0 )
        return;
      
      desc_data = descdata_list[data_index];
    }
    
    if( !dd_can_edit( ch, desc_data ) )
      return;
    
    free_string( desc_data->title, MEM_NOTE );
    
    if( !dd_checkname( ch, argument ) )
      return;
    
    if( *argument == '\0' )
    {
      desc_data->title = empty_string;
      send( ch, "Description renamed to nothing.\n\r" );
    }
    else
    {
      desc_data->title =  alloc_string( argument, MEM_NOTE );
      send( ch, "Description renamed to %s\n\r", desc_data->title );
    }
  }
  else if( !strcmp( arg, "rm" ) || !strcmp( arg, "remove" ) )
  {
    if( ( desc_data = imm->descdata_edit ) == NULL )
    {
      if( ( data_index = find_ddescription( ch, argument, "Remove" ) ) < 0 )
        return;
      
      desc_data = descdata_list[data_index];
    }
    
    if( !dd_can_edit( ch, desc_data )
      && !is_set( &desc_data->noteboard, DD_PURGABLE ) )
      return;
    
    if( is_set( &desc_data->noteboard, DD_PROTECTED ) )
    {
      send( ch, "That description is protected.\n\r" );
      return;
    }
    
    dd_purge( data_index );
    
    sprintf( tmp, "Description %d removed.\n\r", data_index + 1 );
    send( ch, tmp );
  }
  else if( imm->descdata_edit == NULL && ( !strcmp( arg, "own" )
    || !strcmp( arg, "share" ) ) )
  {
    argument = one_argument( argument, arg );
    
    if( ( data_index = find_ddescription( ch, arg, "Set" ) ) < 0 )
      return;
    
    desc_data = descdata_list[data_index];
    
    if( !dd_can_edit( ch, desc_data ) )
      return;
    
    strcpy( tmp, ch->descr->name );
    if( *argument != '\0' )
    {
      if( strlen( argument ) > TWO_LINES )
        argument[ TWO_LINES ] = '\0';
      sprintf( tmp, "%s %s", tmp, argument );
    }
    
    free_string( desc_data->from, MEM_NOTE );
    desc_data->from = alloc_string( tmp, MEM_NOTE );
    
    send( ch, "Creater set to %s.\n\r", tmp );
  }
  else if( !strcmp( arg, "add" ) )
  {
    argument = one_argument( argument, arg );
    
    if( *arg == '\0' )
    {
      send( ch, "Add what?\n\r" );
      return;
    }
    
    if( !dd_checkname( ch, argument ) )
      return;
    
    if( imm->descdata_edit != NULL )
      if( ( data_index = find_ddescription( ch, argument, NULL, TRUE ) ) >= 0 )
        fsend( ch, "Warning: '%s' is an ambiguous name.\n\r", argument );
    
    edit_text = interpret_desctype( ch, arg, FALSE, NULL );
    if( edit_text == NULL )
      return;
    
    if( imm->descdata_edit != NULL )
    {
      if( strlen( *edit_text ) >= MAX_STRING_LENGTH - 2 )
        send( ch, "Error: Addition too long.\n\r" );
      else
        imm->descdata_edit->message = edit_string( ch, *edit_text,
          imm->descdata_edit->message, MEM_NOTE );
      return;
    }
    
    for( data_index = 0; data_index < MAX_DESCDATA_INDEX; data_index++ )
      if( descdata_list[data_index] == NULL )
        break;
    
    if( data_index == MAX_DESCDATA_INDEX )
    {
      sprintf( tmp,
        "The description database has reached its maximum limit of %d.\n\r",
        MAX_DESCDATA_INDEX );
      send( ch, tmp );
      return;
    }
    
    desc_data = new note_data;
    desc_data->title     = alloc_string( argument, MEM_NOTE );
    desc_data->from      = alloc_string( ch->descr->name, MEM_NOTE );
    desc_data->message   = alloc_string( *edit_text, MEM_NOTE );
    desc_data->date      = data_index;
    desc_data->noteboard = 0;
    
    descdata_list[data_index] = desc_data;
    
    sprintf( tmp, "Description added to %d.\n\r", data_index + 1 );
    send( ch, tmp );
  }
  else if( !strcmp( arg, "put" ) )
  {
    if( imm->dd_last_seen == 0 && imm->descdata_edit == NULL )
    {
      send( ch, "Use 'dd <vnum>|<name>' to select a description.\n\r" );
      return;
    }
    
    if( ( desc_data = imm->descdata_edit ) == NULL )
      if( ( desc_data = descdata_list[imm->dd_last_seen - 1] ) == NULL )
      {
        send( ch, "Selected description is no longer valid.\n\r" );
        return;
      }
    
    edit_text = interpret_desctype( ch, argument, TRUE, &mem_type );
    if( edit_text == NULL )
      return;
    
    free_string( *edit_text, mem_type );
    *edit_text = alloc_string( desc_data->message, mem_type );
    
    send( ch, "Description put.\n\r" );
  }
  else if( !strcmp( arg, "list" ) || is_set( &flags, 0 ) )
  {
    list_ddatabase( ch, argument, is_set( &flags, 1 ) );
  }
  else if( !strncmp( arg, "protect", strlen( arg ) )
    && ( imm->descdata_edit == NULL || *argument == '\0' ) )
  {
    if( ( desc_data = imm->descdata_edit ) == NULL )
    {
      if( ( data_index = find_ddescription( ch, argument, "Protect" ) ) < 0 )
        return;
      
      desc_data = descdata_list[data_index];
    }
      
    if( !dd_can_edit( ch, desc_data ) )
      return;
    
    switch_bit( &desc_data->noteboard, DD_PROTECTED );
    
    sprintf( tmp, "Description %d %s.\n\r", (int) desc_data->date + 1,
      is_set( &desc_data->noteboard, DD_PROTECTED ) ? "protected"
      : "unprotected" );
    send( ch, tmp );
  }
  else if( imm->descdata_edit != NULL
    && ( !strcmp( arg, "done" ) || !strcmp( arg, "exit" ) ) )
  {
    imm->descdata_edit = NULL;
    send( ch, "Done.\n\r" );
    return;
  }
  else if( imm->descdata_edit != NULL )
  {
    page( ch, "Name: %s\n\r\n\r", imm->descdata_edit->title );
    
    imm->descdata_edit->message = edit_string( ch, orig_arg,
      imm->descdata_edit->message, MEM_NOTE );
    return;
  }
  else if( !strncmp( arg, "purgable", strlen( arg ) ) )
  {
    if( ( data_index = find_ddescription( ch, argument, "Purgable" ) ) < 0 )
      return;
    
    desc_data = descdata_list[data_index];
    
    if( !dd_can_edit( ch, desc_data ) )
      return;
    
    switch_bit( &desc_data->noteboard, DD_PURGABLE );
    
    sprintf( tmp, "Description %d %s purgable.\n\r", data_index + 1,
      is_set( &desc_data->noteboard, DD_PURGABLE ) ? "set" : "unset" );
    send( ch, tmp );
  }
  else if( !strcmp( arg, "new" ) )
  {
    if( !dd_checkname( ch, argument ) )
      return;
    
    if( ( data_index = find_ddescription( ch, argument, NULL, TRUE ) ) >= 0 )
      fsend( ch, "Warning: '%s' is an ambiguous name.\n\r", argument );
        
    for( data_index = 0; data_index < MAX_DESCDATA_INDEX; data_index++ )
      if( descdata_list[data_index] == NULL )
        break;
    
    if( data_index == MAX_DESCDATA_INDEX )
    {
      sprintf( tmp,
        "The description database has reached its maximum limit of %d.\n\r",
        MAX_DESCDATA_INDEX );
      send( ch, tmp );
      return;
    }
    
    desc_data = new note_data;
    desc_data->title     = alloc_string( argument, MEM_NOTE );
    desc_data->from      = alloc_string( ch->descr->name, MEM_NOTE );
    desc_data->date      = data_index;
    desc_data->noteboard = 0;

    imm->dd_last_seen  = data_index + 1;
    imm->descdata_edit = desc_data;
    
    descdata_list[data_index] = desc_data;
    
    sprintf( tmp, "New description created at %d with the name '%s'.\n\r",
      data_index + 1, argument );
    send( ch, tmp );
  }
  else if( !strcmp( arg, "edit" ) )
  {
    if( ( data_index = find_ddescription( ch, argument, "Edit" ) ) < 0 )
      return;
    
    desc_data = descdata_list[data_index];
    
    if( !dd_can_edit( ch, desc_data ) )
      return;

    if( is_set( &desc_data->noteboard, DD_PROTECTED ) )
    {
      send( ch, "That description is protected.\n\r" );
      return;
    }
    
    imm->descdata_edit = desc_data;
    imm->dd_last_seen = data_index + 1;
    
    sprintf( tmp, "You now edit description #%d.\n\r", data_index + 1 );
    send( ch, tmp );
  }
  else
  {
    if( ( data_index = find_ddescription( ch, orig_arg, "Stat" ) ) < 0 )
      return;
    
    imm->dd_last_seen = data_index + 1;
    stat_ddatabase( ch, data_index );
  }
}

void stat_ddatabase( char_data* ch, int vnum )
{
  char             tmp  [ MAX_STRING_LENGTH ];
  char           owner  [ ONE_LINE ];
  char           title  [ ONE_LINE ];
  
  note_data* desc_data;
  
  if( vnum < 0 || vnum >= MAX_DESCDATA_INDEX
    || ( desc_data = descdata_list[vnum] ) == NULL )
  {
    send( ch, "There is no description at that number.\n\r" );
    return;
  }
  
  strcpy( tmp, desc_data->from );
  strcpy( owner, truncate( tmp, 68 ) );

  strcpy( tmp, desc_data->title );
  strcpy( title, truncate( tmp, 68 ) );
  
  sprintf( tmp,
    "   Vnum: %-4d\n\r   Name: %s\n\r  Owner: %s\n\r  Flags: Purgable [%c]  Protected [%c]\n\r",
    vnum + 1, title, owner,
    is_set( &desc_data->noteboard, DD_PURGABLE ) ? 'X' : ' ',
    is_set( &desc_data->noteboard, DD_PROTECTED ) ? 'X' : ' ' );
  page( ch, tmp );
  
  page( ch, scroll_line[0] );
  
  sprintf( tmp, "Description:\n\r%s", desc_data->message );
  page( ch, tmp );
}

void list_ddatabase( char_data* ch, char* argument, bool me_only )
{
  char           title  [ ONE_LINE ];
  char           owner  [ ONE_LINE ];
  char             tmp  [ TWO_LINES ];
  int            start  = 0;
  int          compare  = 0;
  note_data* desc_data;
  
  if( *argument != '\0' )
  {
    if( is_number( argument ) )
    {
      start = atoi( argument ) - 1;
      
      if( start < 0 || start >= MAX_DESCDATA_INDEX )
      {
        send( ch, "Invalid starting value.\n\r" );
        return;
      }
    }
    else
      compare = strlen( argument );
  }
  
  *tmp = '\0';
  
  for( int i = start; i < MAX_DESCDATA_INDEX; i++ )
    if( ( desc_data = descdata_list[i] ) != NULL )
    {
      if( me_only && strcmp( ch->descr->name, desc_data->from ) )
        continue;
      
      if( compare )
        if( strncmp( argument, desc_data->title, compare ) )
          continue;

      if( *tmp == '\0' )
      {
        sprintf( tmp, "Vnum   %-20s %-35s Prg Prt\n\r", "Name",
          "Owner" );
        page_underlined( ch, tmp );
      }
            
      if( strlen( desc_data->title ) > 20 )
      {
        strncpy( title, desc_data->title, 20 );
        title[20] = '\0';
      }
      else
        strcpy( title, desc_data->title );

      if( strlen( desc_data->from ) > 35 )
      {
        strncpy( owner, desc_data->from, 35 );
        strcpy( &owner[32], "..." );
      }
      else
        strcpy( owner, desc_data->from );

      sprintf( tmp, "[%4d] %-20s %-35s   %s   %s\n\r", i + 1, title,
        owner, is_set( &desc_data->noteboard, DD_PURGABLE ) ? "*" : " ",
        is_set( &desc_data->noteboard, DD_PROTECTED ) ? "*" : " " );
      page( ch, tmp );
    }
  
  if( *tmp == '\0' ) 
    send( ch, "There are no descriptions.\n\r" );
}

void dd_add_new( char* title, char* owner, char* text )
{
  char               arg  [ MAX_INPUT_LENGTH ];
  char               tmp  [ MAX_STRING_LENGTH ];
  int         data_index;
  note_data*   desc_data;
  
  for( data_index = 0; data_index < MAX_DESCDATA_INDEX; data_index++ )
    if( descdata_list[data_index] == NULL )
      break;
  
  if( data_index == MAX_DESCDATA_INDEX )
  {
    sprintf( tmp,
      "The description database has reached its maximum limit of %d.\n\r",
      MAX_DESCDATA_INDEX );
    bug( tmp );
    return;
  }
  
  desc_data = new note_data;
  desc_data->title     = alloc_string( title, MEM_NOTE );
  desc_data->from      = alloc_string( owner, MEM_NOTE );
  desc_data->message   = alloc_string( text, MEM_NOTE );
  desc_data->date      = data_index;
  desc_data->noteboard = 0;
  
  descdata_list[data_index] = desc_data;
  
  sprintf( tmp, "Description '%s' added to #%d for %s.\n\r", title,
    data_index + 1, owner );
  info( empty_string, LEVEL_APPRENTICE, tmp, IFLAG_WRITES, 2 );
}
/*
bool load_descdata( const char* filename )
{
  char              tmp  [ TWO_LINES ];
  char           buffer  [ MAX_STRING_LENGTH*3 ];
  char             text  [ MAX_STRING_LENGTH*3 ];
  char            title  [ ONE_LINE ];
  char            owner  [ ONE_LINE ];
  char*      ptr1, ptr2;
  long           length
  FILE*              fp;

  sprintf( tmp, "%s%s", DESCDATA_DIR, filename );

  if( ( fp = fopen( tmp, "r" ) ) == NULL ) 
    return FALSE;
  
  if( fread( &buffer, MAX_STRING_LENGTH * 3, sizeof( char ) ) == 0 )
  {
    fclose( fp );
    return FALSE;
  }
  
  ptr1 = buffer;
  ptr2 = owner;
  
  while( ptr1 != '\n' && ptr1 != '\0' )
    *(ptr2++) = *(ptr1++);
  
  ptr1++;
  ptr2 = title;
  
  while( ptr1 != '\n' && ptr1 != '\0' )
    *(ptr2++) = *(ptr1++);
  
  
}
*/
/*
void update_ddatabase( void )
{
  DIR*            dirp;
  struct dirent*    dp;
  
  dirp = opendir( DESCDATA_DIR );

  for( dp = readdir( dirp ); dp != NULL; dp = readdir( dirp ) ) {
    if( dp->d_name[0] == '.' )
      continue;
    
    if( !load_descdata( dp->d_name ) ) {
      bug( "Load_descdata: error reading file. (%s)", dp->d_name );
      continue;
    }
  }
  closedir( dirp );
  
  return;
}
*/
bool dd_checkname( char_data* ch, char* name )
{
  if( strlen( name ) > 18 )
  {
    send( ch, "The name must be less than or equal to 18 characters.\n\r" );
    return FALSE;
  }

  return TRUE;
}

bool dd_can_edit( char_data* ch, note_data* desc_data )
{
  if( ( has_permission( ch, PERM_ALL_ROOMS )
    && has_permission( ch, PERM_ALL_MOBS )
    && has_permission( ch, PERM_ALL_OBJECTS ) )
    || is_name( ch->descr->name, desc_data->from ) )
    return TRUE;

  send( ch, "You don't have permission to alter this description.\n\r" );

  return FALSE;
}

void dd_purge( int data_index )
{
  note_data* desc_data;
  wizard_data*     imm;
  
  if( ( desc_data = descdata_list[data_index] ) == NULL )
    return;
  
  for( int i = 0; i < player_list; i++ )
    if( player_list[i]->In_Game( )
      && ( imm = wizard( player_list[i] ) ) != NULL )
    {
      if( imm->descdata_edit == desc_data )
      {
        imm->descdata_edit = NULL;
        send( imm, "The description you were editing has been deleted.\n\r" );
      }
      
      if( imm->dd_last_seen == data_index + 1 )
        imm->dd_last_seen = 0;
    }
  
  descdata_list[data_index] = NULL;
  delete desc_data;
}

char** interpret_desctype( char_data* ch, char* argument,
  bool modify, int* mem_type )
{
  int               length;
  wizard_data*         imm;
  player_data*          pc;
  obj_clss_data*  obj_clss;
  room_data*          room  = ch->in_room;
  species_data*    species;
  
  if( ( imm = wizard( ch ) ) == NULL )
    return NULL;
  
  pc = player( ch );
  
  if( ( length = strlen( argument ) ) == 0 )
  {
    send( ch, "Where?\n\r" );
    return NULL;
  }
  
  if( !strncmp( argument, "rdesc", length ) )
  {
    if( room == NULL )
    {
      send( ch, "You aren't in a room.\n\r" );
      return NULL;
    }
    if( modify && !can_edit( ch, room ) )
      return NULL;
    
    if( mem_type != NULL )
      *mem_type = MEM_ROOM;
    return &room->description;
  }
  else if( !strncmp( argument, "rextra", length ) )
  {
    if( room == NULL )
    {
      send( ch, "You aren't in a room.\n\r" );
      return NULL;
    }
    if( modify && !can_edit( ch, room ) )
      return NULL;

    if( imm->room_edit == NULL )
      send( "You aren't editing any extra.\n\r", ch );
    else
    {
      if( mem_type != NULL )
        *mem_type = MEM_EXTRA;
      return &imm->room_edit->text;
    }
  }
  else if( !strncmp( argument, "adata", length ) )
  {
    if( modify && !can_edit( ch, room ) )
      return NULL;
    
    if( imm->adata_edit == NULL )
      send( ch, "You aren't editing any adata.\n\r" );
    else
    {
      if( mem_type != NULL )
        *mem_type = MEM_EXTRA;
      return &imm->adata_edit->text;
    }
  }
  else if( !strncmp( argument, "odesc", length ) )
  {
    if( ( obj_clss = imm->obj_edit ) == NULL ) {
      send( ch, "You aren't editing any object.\n\r" );
      return NULL;
    }

    if( modify && !ch->can_edit( obj_clss ) )
      return NULL;

    if( imm->oextra_edit == NULL )
      send( "You aren't editing any extra.\n\r", ch );
    else
    {
      if( mem_type != NULL )
        *mem_type = MEM_OBJ_CLSS;
      return &imm->oextra_edit->text;
    }
  }
  else if( !strncmp( argument, "opdata", length ) )
  {
    if( ( obj_clss = imm->obj_edit ) == NULL ) {
      send( ch, "You aren't editing any object.\n\r" );
      return NULL;
    }
    
    if( modify && !ch->can_edit( obj_clss ) )
      return NULL;
    
    if( imm->opdata_edit == NULL )
      send( "You aren't editing any opdata.\n\r", ch );
    else
    {
      if( mem_type != NULL )
        *mem_type = MEM_EXTRA;
      return &imm->opdata_edit->text;
    }
  }
  else if( !strncmp( argument, "mdesc", length ) )
  {
    if( ( species = imm->mob_edit ) == NULL ) {
      send( ch, "You aren't editing any mob - use medit <mob>.\n\r" );
      return NULL;
    }
    
    if( modify && !ch->can_edit( species ) )
      return NULL;
    
    if( mem_type != NULL )
      *mem_type = MEM_DESCR;
    
    return &species->descr->complete;
  }
  else if( !strncmp( argument, "mpdata", length ) )
  {
    if( ( species = imm->mob_edit ) == NULL ) {
      send( ch, "You aren't editing any mob.\n\r" );
      return NULL;
    }
    
    if( modify && !ch->can_edit( species ) )
      return NULL;
    
    if( imm->mpdata_edit == NULL )
      send( ch, "You aren't editing any mpdata.\n\r" );
    else
    {
      if( mem_type != NULL )
        *mem_type = MEM_EXTRA;
      return &imm->mpdata_edit->text;
    }
  }
  else if( !strncmp( argument, "note", length ) )
  {
    if( pc->note_edit == NULL )
      send( ch, "You aren't editing any notes.\n\r" );
    else
    {
      if( mem_type != NULL )
        *mem_type = MEM_NOTE;
      return &pc->note_edit->message;
    }
  }
  else if( !strncmp( argument, "mail", length ) )
  {
    if( ch->pcdata->mail_edit == NULL )
      send( ch, "You aren't editing any mail.\n\r" );
    else
    {
      if( mem_type != NULL )
        *mem_type = MEM_NOTE;
      return &ch->pcdata->mail_edit->message;
    }
  }
  else if( !strncmp( argument, "hdesc", length ) )
  {
    if( ch->pcdata->help_edit == NULL )
      send( ch, "You are not editing any subject.\n\r" );
    else
    {
      if( mem_type != NULL )
        *mem_type = MEM_HELP;
      if( modify )
        help_modified = TRUE;
      return &ch->pcdata->help_edit->text;
    }
  }
  else
  {
    send( ch, "Invalid location.\n\r" );
  }
  
  return NULL;
}

int find_ddescription( char_data* ch, char* argument, const char* verb,
  bool me_only )
{
  int          number  = -1;
  note_data* descdata;
  int               i;
  int          length;
  
  if( *argument == '\0' )
  {
    if( verb != NULL )
      send( ch, "%s what?\n\r", verb );
    return -1;
  }
  
  if( is_number( argument ) )
  {
    number = atoi( argument ) - 1;

    if( number < 0 || number >= MAX_DESCDATA_INDEX
      || descdata_list[number] == NULL )
    {
      if( verb != NULL )
        send( ch, "There is no description by that number.\n\r" );
      return -1;
    }
    return number;
  }
  
  length = strlen( argument );
  
  for( i = 0; i < MAX_DESCDATA_INDEX; i++ )
    if( ( descdata = descdata_list[i] ) != NULL )
      if( !strcmp( ch->descr->name, descdata->from ) &&
        !strncmp( argument, descdata->title, length ) )
        return i;
  
  
  if( !me_only )
    for( i = 0; i < MAX_DESCDATA_INDEX; i++ )
      if( ( descdata = descdata_list[i] ) != NULL )
        if( strcmp( ch->descr->name, descdata->from ) &&
          !strncmp( argument, descdata->title, length ) )
          return i;
  
  if( verb != NULL )
    send( ch, "There is no description by that name.\n\r" );
  return -1;
}


void load_ddatabase( void )
{
  note_data*   note;
  char*       title;
  FILE*          fp;

  echo( "Loading Description Database ...\n\r" );
  vzero( descdata_list, MAX_DESCDATA_INDEX );

  fp = open_file( AREA_DIR, DESCDATA_FILE, "r", TRUE );

  if( strcmp( fread_word( fp ), "#DESCDATA" ) )
    panic( "Load_DDatabase: missing header" );

  for( ; ; ) {
    int    vnum;
    char letter;

    if( ( letter = fread_letter( fp ) ) != '#' ) 
      panic( "Load_DDatabase: # not found." );

    if( ( vnum = fread_number( fp ) ) == 0 )
       break;
    
    if( vnum < 0 || vnum >= MAX_DESCDATA_INDEX ) 
      panic( "Load_DDatabase: vnum out of range." );
    
    if( descdata_list[vnum] != NULL ) 
      panic( "Load_DDatabase: vnum %d duplicated.", vnum );
    
    note            = new note_data;
    note->title     = fread_string( fp, MEM_NOTE );    /* Name */
    note->from      = fread_string( fp, MEM_NOTE );    /* Owner */
    note->message   = fread_string( fp, MEM_NOTE );    /* Description */
    note->date      = vnum;                            /* Vnum */
    note->noteboard = fread_number( fp );              /* Flags */
    
    descdata_list[vnum]  = note;
  }

  fclose( fp );
  return;
}


void save_ddatabase( void )
{ 
  FILE*          fp;
  note_data*   note;
  int             i;
  
  rename_file( AREA_DIR, DESCDATA_FILE,
    AREA_PREV_DIR, DESCDATA_FILE );
  
  if( ( fp = open_file( AREA_DIR, DESCDATA_FILE, "w" ) ) == NULL ) 
    return;

  fprintf( fp, "#DESCDATA\n" );

  for( i = 0; i < MAX_DESCDATA_INDEX; i++ ) {
    if( ( note = descdata_list[i] ) == NULL )
      continue;
    
    fprintf( fp, "#\n%d\n",  (int) (note->date) );
    fprintf( fp, "%s~\n", note->title );
    fprintf( fp, "%s~\n", note->from );
    fprintf( fp, "%s~\n", note->message );
    fprintf( fp, "%d\n",  note->noteboard );
  }
  
  fprintf( fp, "#0\n\n" );
  fclose( fp );

  return;
}
