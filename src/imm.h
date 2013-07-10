/* New Immortal Levels - Zemus - Jan 4 */
#define LEVEL_AVATAR         200
#define LEVEL_APPRENTICE     201
#define LEVEL_BUILDER        202
#define LEVEL_DESIGNER       203
#define LEVEL_ARCHITECT      204
#define LEVEL_ARTISAN        205
#define LEVEL_CREATOR        206
#define LEVEL_MASONIC        207
#define LEVEL_WIZARD         208
#define LEVEL_IMMORTAL       209
#define LEVEL_SPIRIT         210
#define LEVEL_ANGEL          211
#define LEVEL_DEMIGOD        212
#define LEVEL_GOD            213
#define LEVEL_GREATER_GOD    214
#define LEVEL_SUB_ENTITY     215
#define LEVEL_ENTITY         216
#define MAX_LEVEL            216
/* End Changes */

/*
 *   PERMISSION HEADER
 */


#define PERM_NONE                 -1
#define PERM_ALL_MOBS              0 
#define PERM_ALL_OBJECTS           1 
#define PERM_ALL_ROOMS             2 
#define PERM_APPROVE               3
#define PERM_BASIC                 4
#define PERM_BUILD_CHAN            5
#define PERM_COMMANDS              6
#define PERM_ECHO                  7
#define PERM_GOD_CHAN              8
#define PERM_GOTO                  9
#define PERM_HELP_FILES           10
#define PERM_IMM_CHAN             11
#define PERM_LISTS                12
#define PERM_LOAD_OBJECTS         13
#define PERM_MISC_TABLES          14
#define PERM_MOBS                 15
#define PERM_NOTEBOARD            16
#define PERM_OBJECTS              17
#define PERM_PLAYERS              18
#define PERM_QUESTS               19
#define PERM_REIMB_EXP            20
#define PERM_REIMB_EQUIP          21
#define PERM_ROOMS                22
#define PERM_SHUTDOWN             23
#define PERM_SITE_NAMES           24
#define PERM_SNOOP                25
#define PERM_SPELLS               26
#define PERM_SOCIALS              27
#define PERM_TRANSFER             28
#define PERM_UNFINISHED           29
#define PERM_WRITE_ALL            30
#define PERM_WRITE_AREAS          31
#define PERM_AVATAR_CHAN          32
#define PERM_CLANS                33
#define PERM_RTABLES              34 
#define PERM_DISABLED             35
#define PERM_FORCE_PLAYERS        36
#define PERM_CODECHAN             37
/* Added Permission - Zemus - March 10 */
#define PERM_ADMIN                38
#define PERM_PERMFLAGS            39
#define PERM_ALLFLAGS             40 
#define PERM_NEWS                 41
#define MAX_PERMISSION            42
/* End Changes */

extern char const*  permission_name  [ MAX_PERMISSION+1 ];
extern char const*  imm_title        [ ];


bool   has_permission  ( char_data*, int*, bool = FALSE );
bool   mortal          ( char_data*, char* );

void   check_olp_freeze  ( void );  /* PUIOK28/1/2000*/
void   purge_clone       ( player_data* );

inline bool has_permission( char_data* ch, int flag, bool msg = FALSE )
{
  int array  [ 2 ];

  if( flag >= 32 ) {
    array[0] = 0;
    array[1] = ( 1 << ( flag-32 ) );
    }
  else {
    array[0] = 1 << flag;
    array[1] = 0;
    }

  return has_permission( ch, array, msg );
}


/*
 *   LEVEL MACROS
 */


#define trust  get_trust( ch )
#define bool   inline bool

/* New Permission Stuff - Zemus Jan 4 */

bool is_avatar( char_data* ch )
{
 if( has_permission( ch, PERM_APPROVE ) )
   return TRUE;
 
 return FALSE;
}

bool is_admin( char_data* ch )
{
 if( has_permission( ch, PERM_ADMIN ) )
   return TRUE;
 
 return FALSE;
}

bool is_god ( char_data* ch )
{

 if( has_permission( ch, PERM_ALLFLAGS ) )
   return TRUE;

 if( trust == LEVEL_ENTITY )
   return TRUE;

 return FALSE;
}

bool is_immortal( char_data* ch )
{
 if( ch->shdata->level >= LEVEL_APPRENTICE )
   return TRUE;
 
 return FALSE;
}

/* End Changes */



#undef trust
#undef bool


const char*   level_name   ( int );


