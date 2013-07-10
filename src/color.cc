#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "define.h"
#include "struct.h"

const char* color_key = "neEKrRgGyYbBmMcCW";

const char* color_fields [ MAX_COLOR ] = {
  "default",       "room.name",       "tell",            "say",
  "gossip",        "players",         "mobs",            "objects",
  "exits",         "unused",          "titles",          "ctell",
  "chat",          "ooc",             "gtell",           "auction",
  "info",          "to.self",         "to.group",        "by.self",
  "by.group",      "mild.emphasis",   "strong.emphasis", "black",
  "red1",          "red2",            "green1",          "green2",
  "yellow1",       "yellow2",         "blue1",           "blue2",
  "magenta1",      "magenta2",        "cyan1",           "cyan2",
  "white1",        "white2",          "immtalk",         "godtalk",
  "createchan",    "misc.set",        "codechan",        "avatar.chan",
  "news.chan",     "newbiechan",      "improves",        "bash.to.self",
  "bash.by.self",  "chant" };

const char* format_vt100 [] = {
  "none", "bold", "reverse", "underline" };

const int defaults_vt100 [ MAX_COLOR ] = {
  VT100_NORMAL,     VT100_BOLD,   VT100_REVERSE,  VT100_NORMAL,
  VT100_BOLD,       VT100_NORMAL, VT100_NORMAL,   VT100_NORMAL,
  VT100_NORMAL,     VT100_NORMAL, VT100_BOLD,     VT100_NORMAL,
  VT100_NORMAL,     VT100_BOLD,   VT100_NORMAL,   VT100_UNDERLINE,
  VT100_NORMAL,     VT100_BOLD,   VT100_NORMAL,   VT100_UNDERLINE,
  VT100_UNDERLINE,  VT100_BOLD,   VT100_REVERSE,  VT100_NORMAL,
  VT100_NORMAL,     VT100_NORMAL, VT100_NORMAL,   VT100_NORMAL,
  VT100_NORMAL,     VT100_NORMAL, VT100_NORMAL,   VT100_NORMAL,
  VT100_NORMAL,     VT100_NORMAL, VT100_BOLD,     VT100_NORMAL,
  VT100_NORMAL,     VT100_BOLD,   VT100_BOLD,     VT100_NORMAL,
  VT100_NORMAL,     VT100_NORMAL, VT100_NORMAL,   VT100_NORMAL,
  VT100_NORMAL,     VT100_NORMAL
};

const int defaults_ansi [ MAX_COLOR ] = {
  ANSI_NORMAL,      ANSI_BOLD_RED,     ANSI_GREEN,      ANSI_BOLD,
  ANSI_BOLD_YELLOW, ANSI_NORMAL,       ANSI_NORMAL,     ANSI_NORMAL,
  ANSI_NORMAL,      ANSI_NORMAL,       ANSI_BOLD,       ANSI_BOLD_CYAN,
  ANSI_NORMAL,      ANSI_NORMAL,       ANSI_BOLD_GREEN, ANSI_WHITE,
  ANSI_NORMAL,      ANSI_BOLD_RED,     ANSI_GREEN,      ANSI_BLUE,  
  ANSI_YELLOW,      ANSI_BOLD,         ANSI_REVERSE,    ANSI_REVERSE, 
  ANSI_RED,         ANSI_BOLD_RED,     ANSI_GREEN,      ANSI_BOLD_GREEN,
  ANSI_YELLOW,      ANSI_BOLD_YELLOW,  ANSI_BLUE,       ANSI_BOLD_BLUE,
  ANSI_MAGENTA,     ANSI_BOLD_MAGENTA, ANSI_CYAN,       ANSI_BOLD_CYAN,
  ANSI_WHITE,       ANSI_BOLD_WHITE,   ANSI_CYAN,       ANSI_YELLOW,
  ANSI_BLUE,        ANSI_BOLD_MAGENTA, ANSI_BOLD_BLUE,  ANSI_NORMAL,
  ANSI_NORMAL,      ANSI_NORMAL,       ANSI_NORMAL,     ANSI_NORMAL,
  ANSI_NORMAL,      ANSI_NORMAL
};

const char* format_ansi [] = { 
  "bold",        "reverse",      "blinking",    "underlined",
  "black.text",  "red.text",     "green.text",  "yellow.text",
  "blue.text",   "magenta.text", "cyan.text",   "white.text",
  "black.bg",    "red.bg",       "green.bg",    "yellow.bg",
  "blue.bg",     "magenta.bg",   "cyan.bg",     "white.bg"   };

int index_ansi [] = { 1, 7, 5, 4, 30, 31, 32, 33, 34, 35, 36, 37,
  40, 41, 42, 43, 44, 45, 46, 47 };

term_func ansi_codes;
term_func vt100_codes;


const term_type term_table [] =
{
  { "dumb",   0,  NULL,          NULL,         NULL            },
  { "vt100",  4,  format_vt100,  vt100_codes,  defaults_vt100  },
  { "ansi",  20,  format_ansi,   ansi_codes,   defaults_ansi   }
};


/*
 *   ANSI
 */


const char* ansi_colors( int code )
{
  char*   tmp  = static_string( );
  int     i,j;

  if( code == 0 ) 
    return "none";

  *tmp = '\0';

  for( i = 0; code != 0 && i < 5; i++ ) {
    for( j = 0; code%64 != index_ansi[j]; j++ )
      if( j == term_table[ TERM_ANSI ].entries-1 ) 
        return "Impossible Index??";
    sprintf( tmp+strlen( tmp ), "%s%s", *tmp == '\0' ? "" : " ",
      format_ansi[j] );
    code = code >> 6;
    }
 
  return tmp;
}


const char* ansi_codes( int code )
{
  static char   tmp  [ 150 ];
  static int   pntr;  

  pntr = (pntr + 1) % 10;
  char* str = tmp + 15*pntr;

  if ( code == 0 ) {
     strcpy( str, "[0m" );
     return str;
  }
  strcpy( str, "[" );
  int j = 0;
  char a_number[3];
  for ( int i = 0; i < 5; i++ ) {
     j = code % 64;
     if ( j == 0 )
        break;
     if ( i > 0 )
        strcat( str, ";" );
     sprintf( a_number, "%d", j );
     strcat( str, a_number );
     code >>= 6;
  }
  strcat( str, "m" );

  return str;
}


/*
 *   VT100
 */


const char* vt100_codes( int i )
{
  const char* escape_codes [] = { "[0m","[1m", "[7m", "[4m" };

  return escape_codes[i];
}


/*
 *   MAIN COLOR ROUTINE
 */

const char* color_category_names [ MAX_COLOR_CATEGORY ] = {
  "Color.Chan", "Color.Comm", "Color.Display", "Color.Info",
  "Color.Combat", "Color.Misc", "Color.Color"
};

bool in_color_category( int color, int category )
{
  switch( category ) {
    case COLOR_CATEGORY_CHANNEL:
      switch( color ) {
        case COLOR_GOSSIP:
        case COLOR_CTELL:
        case COLOR_CHAT:
        case COLOR_OOC:
        case COLOR_GTELL:
        case COLOR_IMMTALK:
        case COLOR_GODTALK:
        case COLOR_BUILDCHAN:
        case COLOR_CODECHAN:
        case COLOR_AVATARCHAN:
        case COLOR_NEWS:
        case COLOR_NEWBIECHAN:
        case COLOR_CHANT:
          return TRUE;
      }      
      break;
    case COLOR_CATEGORY_COMM:
       switch( color ) {
        case COLOR_TELLS:
        case COLOR_SAYS:
          return TRUE;
      }
      break;
    case COLOR_CATEGORY_DISPLAY:
       switch( color ) {
        case COLOR_DEFAULT:
        case COLOR_ROOM_NAME:
        case COLOR_PLAYERS:
        case COLOR_MOBS:
        case COLOR_OBJECTS:
        case COLOR_EXITS:
          return TRUE;
      }
      break;
    case COLOR_CATEGORY_INFO:
       switch( color ) {
        case COLOR_INFO:
        case COLOR_AUCTION:
        case COLOR_IMPROVES:
          return TRUE;
      }
      break;
    case COLOR_CATEGORY_COMBAT:
      switch( color ) {
        case COLOR_TO_SELF:
        case COLOR_TO_GROUP:
        case COLOR_BY_SELF:
        case COLOR_BY_GROUP:
        case COLOR_BASH_BY_SELF:
        case COLOR_BASH_TO_SELF:
        return TRUE;
      }
      break;
    case COLOR_CATEGORY_MISC:
       switch( color ) {
        case COLOR_TITLES:
        case COLOR_MISCSET:
          return TRUE;
      }
      break;
    case COLOR_CATEGORY_COLOR:
       if( color >= COLOR_MILD && color <= COLOR_BOLD_WHITE )
         return TRUE;
      break;
  }
  return FALSE;
}

void do_color( char_data* ch, char* argument )
{
  char      tmp1  [ MAX_INPUT_LENGTH ];
  char      tmp2  [ MAX_INPUT_LENGTH ];
  char      tmp3  [ MAX_INPUT_LENGTH ];
  int       i, j;
  int*     color;
  int       term;
  int       code;
  bool     found;

  if( ch->link == NULL ) 
    return;

  color = ch->pcdata->color;
  term  = ch->pcdata->terminal;

  if( *argument == '\0' ) {
    page_title( ch, "Terminal Types" );
    for( i = 0; i < MAX_TERM; i++ ) {
      page( ch, "%18s (%1c)%s", term_table[i].name,
        term == i ? '*' : ' ', i%3 == 2 ? "\n\r" : "" );
    }
    if( i%3 != 0 )
      page( ch, "\n\r" );
    page( ch, "\n\r" );

    if( term == TERM_DUMB )
       return;

    page( ch, "%35sLines: %d\n\r\n\r", "", ch->pcdata->lines );

    page_title( ch, "Color Options" );

    for( i = 0; i < term_table[term].entries; i++ ) {
      sprintf( tmp1, "%s", term_table[term].format[i] );
      sprintf( tmp2, "%%%ds%%s",
        18+strlen( tmp1 )-strlen( term_table[term].format[i] ) );
      sprintf( tmp3, tmp2, tmp1, i%4 == 3 ? "\n\r" : "" );
      page( ch, tmp3 );
    }
    if( i%4 != 0 )
      page( ch, "\n\r" );      
    page( ch, "\n\r" );

    page_title( ch, "Color Settings" );
    
    for( i = 0; i < MAX_COLOR_CATEGORY; i++ ) {
      found = FALSE;
      for( j = 0; j < MAX_COLOR; j++ ) {
        if( !in_color_category( j, i ) )
          continue;

        if( ( j == COLOR_IMMTALK && !has_permission( ch, PERM_IMM_CHAN ) )
          || ( j == COLOR_BUILDCHAN && !has_permission( ch, PERM_BUILD_CHAN ) )
          || ( j == COLOR_GODTALK && !has_permission( ch, PERM_GOD_CHAN ) )
          || ( j == COLOR_CODECHAN && !has_permission( ch, PERM_CODECHAN ) ) )
          continue;
        
        if( !found ) {
          page( ch, "\n\r%s%16s - %s -%s\n\r", color_code( ch, COLOR_MILD ),
            "", color_category_names[ i ], c_normal( ch ) );
          found = TRUE;
        }
        page( ch, "%16s : %s%s%s\n\r", color_fields[j], color_code( ch, j ),
          term == TERM_VT100 ? term_table[ TERM_VT100 ].format[
          ch->pcdata->color[j] ] : ansi_colors( ch->pcdata->color[j] ),
          c_normal( ch ) );
      }
    }
    return;
    }

  if( matches( argument, "lines" ) ) {
    if( ( i = atoi( argument ) ) < 10 || i > 500 ) {
      send( ch, "Number of screen lines must be from 10 to 500.\n\r" );
      return;
      }
    ch->pcdata->lines = i;
    setup_screen( ch );
    send( ch, "Number of lines set to %d.\n\r", i );
    return;
    }

  for( i = 0; i < MAX_TERM; i++ )
    if( matches( argument, term_table[i].name ) ) {
      if( i != TERM_DUMB ) {
        for( j = 0; j < MAX_COLOR; j++ )
          color[j] = term_table[i].defaults[j];
        }
      reset_screen( ch );
      ch->pcdata->terminal = i;
      setup_screen( ch );
      send( ch, "Terminal type set to %s.\n\r", term_table[i].name );
      return;
      } 
  
  /* PUIOK 1/6/2000 - color category */
  
  for( i = 0; i < MAX_COLOR_CATEGORY; i++ )
    if( matches( argument, color_category_names[i] ) ) {
      page_title( ch, "Color Options" );

      for( j = 0; j < term_table[term].entries; j++ ) {
        sprintf( tmp1, "%s", term_table[term].format[j] );
        sprintf( tmp2, "%%%ds%%s",
          18+strlen( tmp1 )-strlen( term_table[term].format[j] ) );
        sprintf( tmp3, tmp2, tmp1, j%4 == 3 ? "\n\r" : "" );
        page( ch, tmp3 );
      }
      if( j%4 != 0 )
        page( ch, "\n\r" );      
      page( ch, "\n\r" );

      page_title( ch, "Color Settings" );
      
      found = FALSE;
      for( j = 0; j < MAX_COLOR; j++ ) {
        if( !in_color_category( j, i ) )
          continue;

        if( ( j == COLOR_IMMTALK && !has_permission( ch, PERM_IMM_CHAN ) )
          || ( j == COLOR_BUILDCHAN && !has_permission( ch, PERM_BUILD_CHAN ) )
          || ( j == COLOR_GODTALK && !has_permission( ch, PERM_GOD_CHAN ) )
          || ( j == COLOR_CODECHAN && !has_permission( ch, PERM_CODECHAN ) ) )
          continue;
        
        if( !found ) {
          page( ch, "%s%16s - %s -%s\n\r", color_code( ch, COLOR_MILD ),
            "", color_category_names[ i ], c_normal( ch ) );
          found = TRUE;
        }
        page( ch, "%16s : %s%s%s\n\r", color_fields[j], color_code( ch, j ),
          term == TERM_VT100 ? term_table[ TERM_VT100 ].format[
          ch->pcdata->color[j] ] : ansi_colors( ch->pcdata->color[j] ),
          c_normal( ch ) );
      }
      return;
    }
  
  for( i = 0; i < MAX_COLOR; i++ )
    if( matches( argument, color_fields[i] ) )
      break;
 
  if( i == MAX_COLOR ) {
    send( ch, "Unknown item to color terminal type.\n\r" );
    return;
    }

  if( *argument == '\0' ) {
    send( ch, "To what color do you wish to set %s.\n\r",
      color_fields[i] );
    return;
    }

  if( !strcasecmp( argument, "none" ) ) {
    ch->pcdata->color[i] = 0;
    send( ch, "Color for %s removed.\n\r", color_fields[i] );
    return;
    }

  if( term == TERM_ANSI ) {
    for( code = 0; *argument != '\0'; ) {
      for( j = 0; !matches( argument, term_table[term].format[j] ); j++ ) {
        if( j == term_table[term].entries-1 ) {
          send( ch, "Unknown ansi format.\n\r" );
          return;
	  } 
        }
      code = ( code << 6 )+index_ansi[j];
      }
    ch->pcdata->color[i] = code;
    send( ch, "Color for %s set to %s.\n\r",
      color_fields[i], term == TERM_VT100
      ? term_table[ TERM_VT100 ].format[code] : ansi_colors( code ) );
    return;
    }
      
  for( j = 0; j < term_table[term].entries; j++ )
    if( matches( argument, term_table[term].format[j] ) )
      break;

  if( j == term_table[term].entries ) {
    send( ch, "Unknown format.\n\r" );
    return;
    }

  ch->pcdata->color[i] = j;

  send( ch, "Format of %s set to %s.\n\r",
    color_fields[i], term_table[term].format[j] );

  return;
}
  

/*
 *   CONVERT TEXT TO COLOR CODES
 */

/* PUIOK 22/1/2000 -- Redid output_size code */
/* PUIOK 24/4/2000 -- Uses TERM_DUMB when ch == NULL */

void convert_to_ansi( char_data* ch, const char* input, char* output,
  int output_size )
{
  int          term;
  int             i;
  bool      has_max  =  output_size != -1;

  if( has_max && output_size < 1 )
    return;  

  if( ch != NULL && ch->pcdata == NULL ) {
    *output = '\0';
    return;
    }

  term = ch != NULL ? ch->pcdata->terminal : TERM_DUMB;

  if( has_max )
  {
    *output = '\0';
    if( term == TERM_DUMB )
      output_size--;
    else
      output_size -= strlen( term_table[term].codes(
        ch->pcdata->color[ COLOR_DEFAULT] ) ) - 1;
    
    if( output_size < 0 )
      return;
  }
  
  for( ; *input != '\0' && ( !has_max || output_size > 0 ); input++ ) {
    if( *input != '@' ) {
      *output++ = *input;
      if( has_max ) output_size--;
      continue;
      }

    input++;
  
    switch( *input ) {
      case '@' : 
        *output++ = '@';
        if( has_max ) output_size--;
        continue;
      case '\n':
      case '\0':
      case ' ':
        input--;
        continue;
      case 'I' :
        *output++ = ' ';
        if( has_max && !(--output_size > 0) )
          break;
        *output++ = ' ';
        continue;
    }

    if( term == TERM_DUMB )
      continue;
 
    for( i = 0; color_key[i] != '\0'; i++ )
      if( color_key[i] == *input ) {
        if( has_max && ( output_size -= strlen( term_table[term].codes(
          ch->pcdata->color[ i == 0 ? COLOR_DEFAULT : COLOR_MILD+i-1 ] ) ) ) < 0 )
          break;
        strcpy( output, term_table[term].codes(
          ch->pcdata->color[ i == 0 ? COLOR_DEFAULT : COLOR_MILD+i-1 ] ) );
        output += strlen( output );
      }
    
    /* ADDED -- PUIOK 21/1/2000 */
    
    if( isdigit( *input ) )
    {
      int color_num = *input - '0';
      
      if( *(input + 1) == '@' && isdigit( *(input + 2) ) )
      {
        input += 2;
        color_num = color_num*10 + ( *input - '0' );
      }
      if( color_num < 0 || color_num >= MAX_COLOR )
        color_num = COLOR_DEFAULT;
      
      if( has_max && ( output_size -= strlen( term_table[term].codes(
        ch->pcdata->color[ color_num ] ) ) ) < 0 )
        break; 
      strcpy( output, term_table[term].codes( ch->pcdata->color[ color_num ] ) );
      output += strlen( output );
    }
    
    /* -- END PUIOK */
    
  }

  if( term == TERM_DUMB ) {
    *output = '\0';
    return;
    }

  strcpy( output, term_table[term].codes( ch->pcdata->color[ COLOR_DEFAULT] ) );
  
  if( has_max && output_size < 0 )
    bug( "Convert_to_ansi: had to truncate output to requested output_size" );
  
  return;
}


/*
 *  WINDOW OPERATIONS
 */


void scroll_window( char_data* ch )
{
  int lines = ch->pcdata->lines;

  lock_keyboard( ch );
  save_cursor( ch );
  cursor_off( ch );
  scroll_region( ch, 1, lines-2 );
  move_cursor( ch, lines-2, 1 );
  
  return;
}


void command_line( char_data* ch )
{
  int lines = ch->pcdata->lines;

  scroll_region( ch, lines, lines );
  restore_cursor( ch );
  cursor_on( ch );
  unlock_keyboard( ch );

  return;
}


void setup_screen( char_data* ch )
{
  int lines = ch->pcdata->lines;

  reset_screen( ch );
  clear_screen( ch );

  if( ch->pcdata->terminal == TERM_DUMB 
    || !is_set( ch->pcdata->pfile->flags, PLR_STATUS_BAR ) ) 
    return;
 
  move_cursor( ch, lines, 1 );
  scroll_window( ch );

  return;
}


void clear_screen( char_data* ch )
{
  if( ch->pcdata->terminal != TERM_DUMB )
    send( ch, "[2J[1;1H" );
  else
    send( ch, "\n\r" );

  return;
}


void reset_screen( char_data* ch )
{
  if( ch->pcdata->terminal != TERM_DUMB ) {
    send( ch, "c" );
    send( ch, "[2J[1;1H" );
    }

  return;
}


/*
 *   SEND_COLOR
 */


const char* get_color( char_data* ch, int type )
{
  int  color  = ch->pcdata->color[type];

  if( ch == NULL )
    return "";

  if( ch->pcdata->terminal != TERM_DUMB )
    return term_table[ch->pcdata->terminal].codes( color );
  else 
    return "";
}

void send_color( char_data* ch, int type, const char* msg )
{
  int   term, color;
  
  if( ch == NULL || ch->link == NULL )
    return;

  if( ch->pcdata == NULL ) {
    send( ch, msg );
    return;
    }

  term  = ch->pcdata->terminal;
  color = ch->pcdata->color[type];

  if( term != TERM_DUMB && ch->link->character == ch )  /* PUIOK 28/12/1999 */
    send( ch, "%s%s%s",
      term_table[term].codes( color ), msg,
      term_table[term].codes( ch->pcdata->color[ COLOR_DEFAULT ] ) );
  else 
    send( ch, msg );

  return;
}


void page_color( char_data* ch, int type, const char* msg )
{
  int   term  = ch->pcdata->terminal;
  int  color  = ch->pcdata->color[type];

  if( term != TERM_DUMB ) 
    page( ch, "%s%s%s",
      term_table[term].codes( color ), msg,
      term_table[term].codes( ch->pcdata->color[ COLOR_DEFAULT ] ) );
  else 
    page( ch, msg );

  return;
}


/*
 *   VT100 COLOR ROUTINES
 */


const char* bold_v( char_data* ch )
{
  if( ch->pcdata == NULL )
    return empty_string;

  switch( ch->pcdata->terminal ) {
    case TERM_VT100:  return vt100_codes( VT100_BOLD );
    case TERM_ANSI:   return  ansi_codes( ANSI_BOLD*64+ANSI_WHITE );
    }

  return empty_string;
}


const char* bold_red_v( char_data* ch )
{
  if( ch->pcdata == NULL )
    return empty_string;

  switch( ch->pcdata->terminal ) {
    case TERM_VT100:  return vt100_codes( VT100_BOLD );
    case TERM_ANSI:   return  ansi_codes( ANSI_BOLD*64+ANSI_RED );
    }

  return empty_string;
}


const char* bold_cyan_v( char_data* ch )
{
  if( ch->pcdata == NULL )
    return empty_string;

  switch( ch->pcdata->terminal ) {
    case TERM_VT100:  return vt100_codes( VT100_BOLD );
    case TERM_ANSI:   return  ansi_codes( ANSI_BOLD*64+ANSI_CYAN );
    }

  return empty_string;
}


/*
 *   ANSI COLOR ROUTINES
 */


const char* c_normal( char_data* ch )
{
  if( ch->pcdata == NULL || ch->pcdata->terminal == TERM_DUMB )
    return empty_string;

  return term_table[ ch->pcdata->terminal ].codes(
    ch->pcdata->color[ COLOR_DEFAULT ] );
}


const char* red( char_data* ch )
{
  if( ch->pcdata == NULL || ch->pcdata->terminal == TERM_DUMB )
    return empty_string;

  return term_table[ ch->pcdata->terminal ].codes(
    ch->pcdata->color[ COLOR_RED ] );
}


const char* green( char_data *ch )
{
  if( ch->pcdata == NULL || ch->pcdata->terminal == TERM_DUMB )
    return empty_string;

  return term_table[ ch->pcdata->terminal ].codes(
    ch->pcdata->color[ COLOR_GREEN ] );
}


const char* blue( char_data* ch )
{
  if( ch->pcdata == NULL || ch->pcdata->terminal == TERM_DUMB )
    return empty_string;

  return term_table[ ch->pcdata->terminal ].codes(
    ch->pcdata->color[ COLOR_BLUE ] );
}


const char* yellow( char_data* ch )
{
  if( ch->pcdata == NULL || ch->pcdata->terminal == TERM_DUMB )
    return empty_string;

  return term_table[ ch->pcdata->terminal ].codes(
    ch->pcdata->color[ COLOR_YELLOW ] );
}


/*
 *   COLOR SCALE 
 */


const char* color_scale( char_data* ch, int i )
{
  if( ch->pcdata == NULL || ch->pcdata->terminal != TERM_ANSI )
    return empty_string;

  i = range( 0, i, 6 );

  return term_table[ ch->pcdata->terminal ].codes(
    ch->pcdata->color[ COLOR_WHITE-2*i ] );
}


/*
 *   BATTLE COLOR FUNCTIONS
 */


const char* damage_color( char_data* rch, char_data* ch, char_data* victim )
{
  int term;

  if( rch->pcdata == NULL || ( term = rch->pcdata->terminal ) == TERM_DUMB )
    return empty_string;

  if( rch == victim )
    return term_table[term].codes( rch->pcdata->color[ COLOR_TO_SELF ] );

  if( is_same_group( rch, victim ) )
    return term_table[term].codes( rch->pcdata->color[ COLOR_TO_GROUP ] );

  if( rch == ch )
    return term_table[term].codes( rch->pcdata->color[ COLOR_BY_SELF ] );

  if( is_same_group( rch, ch ) )
    return term_table[term].codes( rch->pcdata->color[ COLOR_BY_GROUP ] );
 
  return empty_string;
}


const char* by_self( char_data* ch )
{
  if( ch->pcdata == NULL || ch->pcdata->terminal == TERM_DUMB )
    return empty_string;

  return term_table[ ch->pcdata->terminal ].codes(
    ch->pcdata->color[ COLOR_BY_SELF ] );
}


const char* to_self( char_data* ch )
{
  if( ch->pcdata == NULL || ch->pcdata->terminal == TERM_DUMB )
    return empty_string;

  return term_table[ ch->pcdata->terminal ].codes(
    ch->pcdata->color[ COLOR_TO_SELF ] );
}















