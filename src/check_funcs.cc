#include <stdio.h>
#include "define.h"
#include "struct.h"
#include "condition.h"

int check_condition( const char*, char*, int,
  target_type = NULL, target_type = NULL, target_type = NULL,
  target_type = NULL, target_type = NULL, target_type = NULL,
  target_type = NULL );

/*
 *  ENUMS - enum_list = { const char* name, int value };
 */

enum_list permission_enum_set [] = {
  { "has",        1  },
  { NULL,         0  }
};

enum_list level_enum_set [] = {
  { "avator",     200 },
  { "apprentice", 201 },
  { "builder",    202 },
  { "designer",   203 },
  { "architect",  204 },
  { "artisan",    205 },
  { "creator",    206 },
  { "masonic",    207 },
  { "wizard",     208 },
  { "immortal",   209 },
  { "spirit",     210 },
  { "angel",      211 },
  { "demigod",    212 },
  { "god",        213 },
  { "greatergod", 214 },
  { "subentity",  215 },
  { "entity",     216 }
};

enum_list position_enum_set [] = {
  { "dead",       0  },
  { "mortal",     1  },
  { "incap",      2  },
  { "stunned",    3  },
  { "sleeping",   4  },
  { "meditating", 5  },
  { "resting",    6  },
  { "standing",   8  }
};

enum_list gender_enum_set [] = {
  { "neutral",    0  },
  { "male",       1  },
  { "female",     2  }
};

/*
 * FUNCTIONS - check_func_list =
 *   { const char* name, const char* shrtn, check_func func,
 *     int argc, int valc, enum_list* enum_set };
 */

check_func math_constants;
check_func check_hit;
check_func check_level;
check_func check_mana;
check_func check_maxhit;
check_func check_maxmana;
check_func check_maxmove;
check_func check_move;
check_func check_permission;
check_func check_position;
check_func check_seen;
check_func check_sex;
check_func check_skill;

check_func_list standard_functions [] =
{
  { "mana",       "e",  check_mana,       1, 0, NULL                },
  { "mxmana",     "E",  check_maxmana,    1, 0, NULL                },
  { "hit",        "h",  check_hit,        1, 0, NULL                },
  { "mxhit",      "H",  check_maxhit,     1, 0, NULL                },
  { "skill",      "K",  check_skill,      1, 1, NULL                },
  { "level",      "L",  check_level,      1, 0, level_enum_set      },
  { "position",   "P",  check_position,   1, 0, position_enum_set   },
  { "cansee",     "S",  check_seen,       2, 0, NULL                },
  { "move",       "v",  check_move,       1, 0, NULL                },
  { "mxmove",     "V",  check_maxmove,    1, 0, NULL                },
  { "sex",        "X",  check_sex,        1, 0, gender_enum_set     },
  { "constant",   NULL, math_constants,   0, 1, NULL                },
  { "permission", NULL, check_permission, 1, 1, permission_enum_set },
  { NULL,         NULL, NULL,             0, 0, NULL                }
};


double check_hit( target_type* targv, value_type* valuv )
{
  char_data* ch;
  
  if( ( ch = character( targv[ 0 ] ) ) == NULL )
    return 0.0;
  return ch->hit;
}

double check_move( target_type* targv, value_type* valuv )
{
  char_data* ch;
  
  if( ( ch = character( targv[ 0 ] ) ) == NULL )
    return 0.0;
  return ch->move;
}

double check_mana( target_type* targv, value_type* valuv )
{
  char_data* ch;
  
  if( ( ch = character( targv[ 0 ] ) ) == NULL )
    return 0.0;
  return ch->mana;
}

double check_maxhit( target_type* targv, value_type* valuv )
{
  char_data* ch;
  
  if( ( ch = character( targv[ 0 ] ) ) == NULL )
    return 0.0;
  return ch->max_hit;
}

double check_maxmove( target_type* targv, value_type* valuv )
{
  char_data* ch;
  
  if( ( ch = character( targv[ 0 ] ) ) == NULL )
    return 0.0;
  return ch->max_move;
}

double check_maxmana( target_type* targv, value_type* valuv )
{
  char_data* ch;
  
  if( ( ch = character( targv[ 0 ] ) ) == NULL )
    return 0.0;
  return ch->max_mana;
}

double check_level( target_type* targv, value_type* valuv )
{
  char_data* ch;
  
  if( ( ch = character( targv[ 0 ] ) ) == NULL )
    return 0.0;
  return ch->shdata->level;
}

double check_position( target_type* targv, value_type* valuv )
{
  char_data* ch;
  
  if( ( ch = character( targv[ 0 ] ) ) == NULL )
    return 0.0;
  return ch->position;
}

double check_sex( target_type* targv, value_type* valuv )
{
  char_data* ch;
  
  if( ( ch = character( targv[ 0 ] ) ) == NULL )
    return 0.0;
  return ch->sex;
}

double check_seen( target_type* targv, value_type* valuv )
{
  char_data* ch;
  
  if( ( ch = character( targv[ 0 ] ) ) == NULL )
    return 0.0;
  return targv[ 1 ]->Seen( ch );
}

double check_skill( target_type* targv, value_type* valuv )
{
  char_data* ch;
  
  if( ( ch = character( targv[ 0 ] ) ) == NULL )
    return 0.0;
  for( int i = 0; i < MAX_SKILL; i++ )
    if( !strcmp( skill_table[ i ].name, valuv[ 0 ] ) )
      return ch->get_skill( i );
  return 0.0;
}

double check_permission( target_type* targv, value_type* valuv )
{
  char_data* ch;
  
  if( ( ch = character( targv[ 0 ] ) ) == NULL )
    return 0.0;
  for( int i = 0; i < MAX_PERMISSION; i++ )
    if( !strcmp( permission_name[ i ], valuv[ 0 ] ) )
      return has_permission( ch, i );
  return 0.0;
}

double math_constants( target_type* targv, value_type* valuv )
{
  if( !strcmp( valuv[ 0 ], "pi" ) )
    return 3.14159265359;
  
  return 0.0;
}

/*
 *  CHECK_CONDITION
 */

#define MAX_TARGET_TYPE 2

const char* target_type_name [ MAX_TARGET_TYPE ] = { "ch", "ob" };

int check_condition( const char* condition, char* error_msg, int msg_width,
  target_type vis1, target_type vis2, target_type vis3, target_type vis4,
  target_type vis5, target_type vis6, target_type vis7 )
{
  target_type      targets [ 7 ] = { vis1, vis2, vis3, vis4, vis5, vis6, vis7 };
  int         target_types [ 7 ];
  int           type_count [ MAX_TARGET_TYPE ];
  int         return_value;
  int         target_count;
  int                 i, j;
  char*           char_ptr;
  identifier_list*  ident_list;
  
  bzero( type_count, sizeof( int )*MAX_TARGET_TYPE );
  
  target_count = 0;
  for( i = 0; i < 7 && targets[ i ] != NULL; i++ ) {
    if( character( targets[ i ] ) != NULL )
      target_types[ i ] = 0;
    else if( object( targets[ i ] ) != NULL )
      target_types[ i ] = 1;
    else {
      targets[ i ] = NULL;
      continue;
    }
    target_count++;
  }
  if( target_count > 0 ) {
    ident_list =  new identifier_list [ target_count+1 ];

    for( j = i = 0; target_count > 0; i++ ) {
      if( targets[ i ] == NULL )
        continue;
      ident_list[ j ].name = char_ptr = new char [
        strlen( target_type_name[ target_types[ i ] ] )+2 ];
      sprintf( char_ptr, "%s%d", target_type_name[ target_types[ i ] ],
        ++type_count[ i ] );
      ident_list[ j ].shrtn  = i+1;
      ident_list[ j ].target = targets[ i ];
      target_count--;
      j++;
    }
    ident_list[ j ].name = NULL;
  }
  
  return_value = check_condition( condition, standard_functions, ident_list );
  if( target_count > 0 ) {
    for( i = 0; i < target_count; i++ )
      delete[] ident_list[ i ].name;
    delete[] ident_list;
  }
  if( return_value < 0 && error_msg != NULL ) {
    form_error_message( return_value, condition, error_msg, msg_width );
    return -1;
  }
  return return_value != 0;
}


