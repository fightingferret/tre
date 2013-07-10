/* EXPCAT Defs - Zemus march 21 */
#define EXP_WEAPON           0
#define EXP_LANGUAGE         1
#define EXP_OFF_SPELL        2
#define EXP_ASS_SPELL        3
#define EXP_HEAL_SPELL       4
#define EXP_TRADE            5
#define EXP_ALCHEMY          6
#define EXP_FIGHTING         7
#define EXP_THEFT            8
#define EXP_MISC             9
#define EXP_NAVIGATION      10
#define EXP_RELIGION        11
#define EXP_MUSIC           12
#define MAX_EXPCAT          13

extern char const* expcat [ ];

/*
 *   ALIAS HEADERS
 */


char*  subst_alias     ( link_data*, char* );
void   ampersand       ( text_data* );


class Alias_Data
{
 public:
  char*         abbrev;
  char*         command;

  Alias_Data       ( char*, char* );
  ~Alias_Data      ( );

  friend char* name( alias_data* alias ) {
    return alias->abbrev;
    }
};


/*
 *   ATTRIBUTES
 */


extern index_data        fame_index  [];
extern index_data       piety_index  [];
extern index_data  reputation_index  [];


/*
 *   PFILE ROUTINES
 */


#define MAX_VOTE    5


class Pfile_Array
{
 public:
  int           size;
  pfile_data**  list;

  Pfile_Array( ) {
    size = 0;
    list = NULL;
    }

  ~Pfile_Array( ) {
    if( size > 0 )
      delete [] list;
    }
};


class Pfile_Data
{
 public:
  char*              name;
  char*               pwd;
  account_data*   account;
  note_data*         mail;
  char*         last_host;
  pfile_data*        vote  [ MAX_VOTE ];
  int               flags  [ 2 ];
  int            settings;
  int               trust; 
  int              deaths;
  int               level;
  int                race;
  int                clss;
  int                 sex;
  int             last_on;
  int             created;
  int               ident;
  int                 exp;

  int                rank;
  int             guesses;   
  clan_data*         clan;
  char*          homepage;

/* Changes Zemus - Mar 10 */
  char*               icq;   /* Added by Letharin 8/1/2000(d/m/y) */
  int              bounty;
  char*            hunter;   /* hunter zemus 7-24 */
  int              banned;
/* added pkcount - july 13 NEWBC */
  int             pkcount;

  Pfile_Data   ( const char* );
  ~Pfile_Data  ( );

  friend char* name( pfile_data* pfile ) {
    return pfile->name;
    }

  friend int   assign_ident   ( );
};


extern pfile_data*     ident_list  [ MAX_PFILE ];
extern pfile_data**    pfile_list;
extern pfile_data**     site_list;
extern int              max_pfile;
extern int           site_entries;


inline pfile_data* get_pfile( int i )
{
  if( i < 0 || i >= MAX_PFILE )
    return NULL;

  return ident_list[i];
}


int           reference              ( player_data*, thing_array&, int );
void          dereference            ( player_data* );
void          purge                  ( player_data* );
player_data*  find_player            ( pfile_data* );
pfile_data*   find_pfile             ( const char*, char_data* = NULL );
pfile_data*   find_pfile_exact       ( const char* );
pfile_data*   find_pfile_substring   ( const char* );
pfile_data*   player_arg             ( char*& );
int           site_search            ( const char* );
void          extract                ( pfile_data*, link_data* = NULL );
void          add_list               ( pfile_data**&, int&, pfile_data* );
void          remove_list            ( pfile_data**&, int&, pfile_data* );

bool          spam_warning           ( char_data* );  /* PUIOK - 24 July 2000 */


/*
 *   PCDATA
 */


class Pc_Data
{
 public:
  char*            title;
  char*            tmp_keywords;
  char*            tmp_short;
  char*            buffer;
  char*            prompt;
  help_data*       help_edit;
  int              message;
  int              mess_settings; 
  int              trust;
  int              clss;
  int              religion;
  int              color [ 55 ]; /* 1 June 2000 - to 55 from 45 */
  int              terminal;
  int              lines;
  int              condition [ 4 ];
  int              cflags [ 4 ];
  int              speaking;
  int              piety;
  int              practice;
  int              prac_timer;
  int              quest_pts;
  int              quest_flags [ MAX_QUEST ];
  int              level_hit;
  int              level_mana;
  int              level_move;
  int              max_level; 
  int              mod_age;
  int              wimpy;
  int              pkcount;
  note_data*       mail_edit;
  pfile_data*      pfile;
  recognize_data*  recognize;  
  char*            clipboard;
  int              expt [ MAX_EXPCAT ];
  int              levelt [ MAX_EXPCAT ];
};


/* 
 *   RECOGNIZE
 */


class Recognize_Data
{
 public:
  int    size;
  int*   list;

  Recognize_Data    ( int );
  ~Recognize_Data   ( );
};


void   remove                ( recognize_data*&, int );
void   reconcile_recognize   ( char_data* );
int    search                ( recognize_data*, int );
bool   consenting            ( char_data*, char_data*,
                               const char* = empty_string );


/*
 *   REQUESTS
 */


bool  remove      ( request_array&, char_data* );


extern request_array    request_imm;
extern request_array    request_app;



