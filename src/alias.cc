#include <sys/types.h>
#include <stdio.h>
#include <syslog.h>
#include "define.h"
#include "struct.h"


/*
 *   ALIAS NEW AND DELETE
 */


Alias_Data :: Alias_Data( char* arg, char* argument )
{
  record_new( sizeof( alias_data ), MEM_ALIAS );

  abbrev      = alloc_string( arg, MEM_ALIAS );
  command     = alloc_string( argument, MEM_ALIAS );

  return;
}


Alias_Data :: ~Alias_Data( )
{
  record_delete( sizeof( alias_data ), MEM_ALIAS );
  
  free_string( abbrev, MEM_ALIAS );
  free_string( command, MEM_ALIAS );
  
  return;
}

/* ADDED -- PUIOK 10/1/2000 -- VARIABLES AND SUBS */
int     make_substitutions       ( char *, const char * );
int     has_variables            ( char *, bool = FALSE );
 
/*
 *   ALIAS ROUTINES
 */


void do_alias( char_data* ch, char* argument )
{
  char             arg  [ MAX_INPUT_LENGTH ];
  alias_data*    alias;
  player_data*      pc;
  int              pos;

  if( is_mob( ch ) )
    return;

  pc = player( ch );

  argument = one_argument( argument, arg );

  if( *arg == '\0' ) {
    if( is_empty( pc->alias ) ) {
      send( ch, "You have no aliases.\n\r" );
      return;
      }
    page( ch, "Alias:       Substitution:\n\r" );
    for( pos = 0; pos < pc->alias.size; pos++ ) 
      page( ch, "%-12s %s\n\r",
        pc->alias[pos]->abbrev, pc->alias[pos]->command );
    return;
    }

  if( strlen( arg ) > 8 ) {
    send( ch, "Alias must be less than 9 letters.\n\r" );
    return;
    }

  pos = pntr_search( pc->alias.list, pc->alias.size, arg );

  if( *argument == '\0') {
    if( pos >= 0 ) {
      delete pc->alias[pos];
      remove( pc->alias.list, pc->alias.size, pos );
      send( ch, "Alias removed.\n\r" );
      return;
      }
    send( ch, "Alias not found.\n\r" );
    return;
    }

  if( strlen( argument ) > 200 ) {
    send( ch, "Substitution must be less than 201 letters.\n\r" );
    return;
    }

  if( is_number( arg ) ) {
    send( ch, "Due to the prevelant usage of numbers as arguments in commands, it is\n\rill-advised to alias numbers without the use of an asterisk.  See help alias\n\rfor more information.\n\r" );
    return;
    }

  if( !strcmp( arg, "*" ) ) {
    send( ch, "Aliasing nothing doesn't make much sense.\n\r" );
    return;
    }

  if( pos >= 0 ) {
    alias = pc->alias[pos];
    free_string( alias->command, MEM_ALIAS );
    alias->command = alloc_string( argument, MEM_ALIAS );
    }
  else {
    if( pc->alias.size >= 100 ) {
      send( ch, "You are limited to one hundred aliases.\n\r" );
      return;
      }
    alias = new alias_data( arg, argument );
    insert( pc->alias.list, pc->alias.size, alias, -pos-1 );
    }  

  send( ch, "Alias %s -> %s added.\n\r", alias->abbrev, alias->command );
  return;
}


char* subst_alias( link_data* link, char* message )
{
  static char         buf  [ MAX_STRING_LENGTH ];
  player_data*     player;
  alias_data*       alias;
  const char*      abbrev;
  int             i, j, k;
  int              length;
  bool            newline  = TRUE;

  if( ( player = link->player ) == NULL || link->connected != CON_PLAYING 
    || is_empty( player->alias )
    || !strncasecmp( message, "ali", 3 )
    || *message == '_' )
    return message;

  skip_spaces( message );

  for( i = j = 0; message[i] != '\0' && j < MAX_INPUT_LENGTH; ) {
    for( k = 0; k < player->alias.size; k++ ) {
      alias  = player->alias[k];
      if( *alias->abbrev == '*' ) {
        if( !newline )
          continue;
        abbrev = &alias->abbrev[1];    
        }
      else {
        abbrev = alias->abbrev;
        }
      length = strlen( abbrev );
      if( !strncasecmp( message+i, abbrev, length )
        && ( message[ length+i ] == ' '
        || message[ length+i ] == '\0' ) ) {
        
        /* PUIOK 10/1/1000 -- VARS AND SUBS */
        if( has_variables( alias->command ) )
        {
          char variables[ MAX_INPUT_LENGTH ];
          char constants[ MAX_INPUT_LENGTH ];
          
          strcpy( variables, alias->command );
          if( strlen( &message[ length+i ] ) >= MAX_INPUT_LENGTH )
            strncpy( constants, &message[ length+i ], MAX_INPUT_LENGTH - 1 );
          else
            strcpy( constants, &message[ length+i ] );
          for( int l = 0, s = strlen( constants ); l < s; l++ )
            if( constants[l] == '&' && ( l <= 0 || constants[l - 1] == ' ' ) &&
              ( l >= s - 1 || constants[l + 1] == ' ' || constants[l + 1] == '\0' ) )
            {
              int m;
              constants[l] = '\0';
              constants[l - 1] = '\0';
              for( m = l + 1; m < s; m++ )
                if( constants[m] != ' ' )
                  break;
              if( m == s )
                message[ length+i+l-1 ] = '\0';
              break;
            }
          length += strlen( constants );
          if( make_substitutions( variables, constants ) < -1 )
            send( player, "!! Substitutions too long and left incomplete !!\n\r" );

          strcpy( buf+j, variables );
          i += length;
          j += strlen( variables );
        }
        else
        {
          strcpy( buf+j, alias->command );
          i += length;
          j += strlen( alias->command );
        }
        break;
        }
      }
    if( k == player->alias.size ) {
      for( ; message[i] != ' ' && message[i] != '\0'; ) 
        buf[j++] = message[i++];
      }
    for( ; message[i] == ' '; i++ ) {
      buf[j++] = ' ';
      }
    if( message[i] == '&' && message[i+1] == ' ' ) {
      newline = TRUE;
      strcpy( buf+j, "& " );
      j += 2;
      i += 2;
      }
    else {
      newline = FALSE;
      }
    }  

  if( j >= MAX_INPUT_LENGTH ) {
    send( player, "!! Truncating input !!\n\r" );
    buf[ MAX_INPUT_LENGTH-1 ] = '\0';
    }
  else {
    buf[j] = '\0';
    }

  return buf;
}


void ampersand( text_data* receive )
{
  char*        letter;
  text_data*     next;

  if( !strncasecmp( receive->message.text, "ali", 3 ) )
    return; 

  for( letter = receive->message.text; *letter != '\0'; letter++ ) 
    if( !strncmp( letter, " & ", 3 ) ) {  
      next          = new text_data( letter+3 );
      next->next    = receive->next;
      receive->next = next;      
      *letter       = '\0';
      overwrite( receive, receive->message.text );
      break;
      }

  return;
}


/*
 *    VARIABLES AND SUBSTITUTION -- ADDED PUIOK 10/1/2000
 */

int has_variables(char *string, bool keepcount)
{
  int count = 0;

  for(int i = strlen(string) - 1; i > 0; i--)
    if(string[i - 1] == '%' && string[i] >= '0' && string[i] <= '9' &&
      (i <= 1 || string[i - 2] != '%'))
      if(keepcount)
        count++;
      else
        return TRUE;

  return count;
}

int variable_count(const char *string)
{
  int count = 0, length;
  bool has_zero = FALSE;
  
  length = strlen(string);
  
  for(int i = 0; i < length - 1; i++)
    if(string[i] == '%' && string[i + 1] >= '0' && string[i + 1] <= '9' &&
      (i <= 0 || string[i - 1] != '%'))
    {
      if(string[i + 1] == '0')
      {
        if(!has_zero)
          has_zero = TRUE;
      }
      else
        if(count < string[i + 1] - '0')
          count = string[i + 1] - '0';
    }

  return has_zero ? -(++count) : count;
}

/* formats source to a usable string and returns the actual number of subs */

int format_substitutions(char *substitutes, int *count)
{
  bool was_space = TRUE;
  int current_count = 0;
  bool has_zero = FALSE;
  
  if(*count == 0)
    return 0;
  
  if(has_zero = (*count < 0))
    *count *= -1;
    
  for(;*substitutes != '\0'; substitutes++)
    if(*substitutes != ' ')
    {
      if(was_space)
      {
        current_count++;
        was_space = FALSE;
        if(current_count == *count && has_zero)
          break;
      }
    }
    else
    {
      *substitutes = '\0';
      if(current_count == *count)
        break;
      was_space = TRUE;
    }

  return current_count;
}

/* get index substitution from formated string */

char *get_substitute(char *substitutes, int index, int max_strlen)
{
  bool was_terminate = TRUE;
  
  for(int i = 0; i < max_strlen; i++)
    if(substitutes[i] != '\0')
    {
      if(was_terminate)
      {
        index--;
        if(!index)
          return &substitutes[i];
        was_terminate = FALSE;
      }
    }
    else
      was_terminate = TRUE;

  return NULL;
}

int get_sub_length(char *substitutes, int index, int max_strlen)
{
  bool was_terminate = TRUE;
  
  for(int i = 0; i < max_strlen; i++)
    if(substitutes[i] != '\0')
    {
      if(was_terminate)
      {
        index--;
        if(!index)
          index--;
        was_terminate = FALSE;
      }
      else if(index < 0)
        index--;
    }
    else
    {
      if(index < 0)
        return -index;
      was_terminate = TRUE;
    }

  return 0;
}

int make_substitutions(char *vars, const char *subs)
{
  char substitutes[MAX_INPUT_LENGTH];
  char swap[MAX_INPUT_LENGTH];
  int max_variables, max_subslength;
  int actual_variables;
  int subs_index;
  int substitutes_made = 0;
  
  strcpy(substitutes, subs);
  
  max_variables = variable_count(vars);
  max_subslength = strlen(substitutes);
  actual_variables = format_substitutions(substitutes, &max_variables);
  
  for(int i = strlen(vars) - 1; i > 0; i--)
    if(vars[i - 1] == '%' && vars[i] >= '0' && vars[i] <= '9' )
    {
      if(i > 1 && vars[i - 2] == '%')
      {
        strcpy(&vars[i - 1], &vars[i]);
        i--;
      }
      else
      {
        if((subs_index = vars[i] - '0') == 0)
          subs_index = max_variables;
        
        if(subs_index > actual_variables)
          sprintf(&vars[i - 1], "%s%s", "", &vars[i + 1]);
        else
        if(get_sub_length(substitutes, subs_index, max_subslength) +
          strlen(vars) - 2 < MAX_INPUT_LENGTH)
        {
          strcpy(swap, &vars[i + 1]);
          sprintf(&vars[i - 1], "%s%s",
            get_substitute(substitutes, subs_index, max_subslength), swap);
          substitutes_made++;
        }
        else
          return -1 /* result too long */;
      }
    }
  
  return substitutes_made;
}

/* -- END PUIOK */


