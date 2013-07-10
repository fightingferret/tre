#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "define.h"
#include "struct.h"
#include "condition.h"

#define PRE_FUNC_TAG( c )			( c == '\'' )
#define POST_FUNC_TAG( c )          ( c == ',' )
#define IDENT_JOINER( c )           ( c == ':' )
#define VALUE_TAG( c )              ( c == '#' )
#define PRE_VALUE_TAG( c )          ( c == '{' )
#define POST_VALUE_TAG( c )         ( c == '}' )
                                 
/* DUPLICATE MEANINGS */

#define POST_ARG_TAG( c )           ( c == ',' )
#define FUNC_JOINER( c )            ( c == ':' )

#define POST_UNCLOSED_TAG( c )      POST_FUNC_TAG( c ) || FUNC_JOINER( c ) \
                                 || VALUE_TAG( c ) || PRE_VALUE_TAG( c )

#define MAX_ARG       5
#define MAX_DUB_ARG  10
#define MAX_DUB_FUNC 10
#define MAX_VALUES    5

char* cond_idents[ MAX_ARG ][ MAX_DUB_ARG ];
char* cond_functs[ MAX_DUB_FUNC ];
char* cond_values[ MAX_DUB_FUNC ][ MAX_VALUES ];

bool        first_run = TRUE;
const char* error_ptr;

check_func_list* check_functions;
identifier_list* target_list;

int		push_ident				( const char*, int = -1 );
int 	push_func				( const char* );
int 	push_value				( const char*, int );

void    alloc_error_value       ( const char*, int = -1 );

/*
 *  QUEUE HANDLERS - USED IN FUNCTION PARSING
 */

int push_ident( const char* identifier, int index )
{
  int i;
  
  if( index < 0 ) {
    for( index = 0; index < MAX_ARG; index++ )
      if( cond_idents[ index ][ 0 ] == NULL )
        break;
  }
  
  if( index >= MAX_ARG ) {
    error_ptr--;
    return TOO_MANY_TARGETS_ERROR;
  }
  
  for( i = 0; i < MAX_DUB_ARG; i++ )
    if( cond_idents[ index ][ i ] == NULL )
      break;
  
  if( i == MAX_DUB_ARG ) {
    error_ptr--;
    return JOINED_ARG_LIMIT_ERROR;
  }
  
  cond_idents[ index ][ i ] = new char [ strlen( identifier )+1 ];
  strcpy( cond_idents[ index ][ i ], identifier );
  
  return index;
}

int push_func( const char* function )
{
  int i;
  
  for( i = 0; i < MAX_DUB_FUNC; i++ )
    if( cond_functs[ i ] == NULL )
      break;
  
  if( i == MAX_DUB_FUNC ) {
    error_ptr--;
    return JOINED_FUNC_LIMIT_ERROR;
  }
  
  cond_functs[ i ] = new char [ strlen( function )+1 ];
  strcpy( cond_functs[ i ], function );
  
  return i;
}

int push_value( const char* value, int index )
{
  int i;
  
  if( index >= MAX_DUB_FUNC )
    return UNEXPECTED_PARSE_ERROR;
  
  for( i = 0; i < MAX_VALUES; i++ )
    if( cond_values[ index ][ i ] == NULL )
      break;
  
  if( i == MAX_VALUES ) {
    error_ptr--;
    return TOO_MANY_VALUES_ERROR;
  }
  
  cond_values[ index ][ i ] = new char [ strlen( value )+1 ];
  strcpy( cond_values[ index ][ i ], value );
  
  return index;
}

void init_queues( void )
{
  bzero( cond_idents, sizeof( char* )*MAX_ARG*MAX_DUB_ARG );
  bzero( cond_functs, sizeof( char* )*MAX_DUB_FUNC );
  bzero( cond_values, sizeof( char* )*MAX_DUB_FUNC*MAX_VALUES );
}

void clear_idents( void )
{
  for( int i = 0; i < MAX_ARG; i++ )
    for( int j = 0; j < MAX_DUB_ARG && cond_idents[ i ][ j ] != NULL; j++ ) {
      delete[] cond_idents[ i ][ j ];
      cond_idents[ i ][ j ] = NULL;
    } 
}

void clear_queues( void )
{
  clear_idents( );
  
  for( int i = 0; i < MAX_DUB_FUNC && cond_functs[ i ] != NULL; i++ ) {
    delete[] cond_functs[ i ];
    cond_functs[ i ] = NULL;
  }
  
  for( int i = 0; i < MAX_DUB_FUNC; i++ )
    for( int j = 0; j < MAX_VALUES && cond_values[ i ][ j ] != NULL; j++ ) {
      delete[] cond_values[ i ][ j ];
      cond_values[ i ][ j ] = NULL;
    }
}

enum parse_types {
  NO_TYPE             = 0,
  BASIC_TYPE          = 1,
  NUMBER_TYPE         = 2,
  ENUMED_TYPE         = 3,
  IDENTIFIER_TYPE     = 4,
  FUNCTION_TYPE       = 5,
  PRE_TAG_VALUE_TYPE  = 6,
  CLOSED_VALUE_TYPE   = 7,
  PARSE_SUCCESSFUL    = 8
};

/*
 *  FUNCTION PARSING
 */

/*
 *  get_something
 *
 *  Used by parse_set ( ... )
 *
 */

int get_something( const char*& buffer, const char* end, char* something,
  int expected_type )
{
  const char*         s1 = buffer;
  const char*         s2 = something;
  bool  short_identifier = FALSE;
  
  if( buffer > end ) {
    error_ptr = buffer;
    return SYNTAX_ERROR;
  }
  
  if( expected_type == IDENTIFIER_TYPE )
    short_identifier = isdigit( *s1 );
  
  if( expected_type == FUNCTION_TYPE )
    if( isdigit( *s1 ) ) {
      expected_type = IDENTIFIER_TYPE;
      short_identifier = TRUE;
    }
  
  for( ; s1 <= end; *something++ = *s1++ ) {
    if( expected_type == CLOSED_VALUE_TYPE ) {
      if( POST_VALUE_TAG( *s1 ) )
        break;
      continue;
    }
    
    if( expected_type == PRE_TAG_VALUE_TYPE ) {
      if( POST_UNCLOSED_TAG( *s1 ) )
        break;
      continue;
    }
    
    if( PRE_FUNC_TAG( *s1 ) || POST_FUNC_TAG( *s1 )
      || IDENT_JOINER( *s1 ) || VALUE_TAG( *s1 )
      || PRE_VALUE_TAG( *s1 ) )
      break;
    
    if( !isalnum( *s1 ) ) {
      error_ptr = s1;
      return UNEXPECTED_CHAR_ERROR;
    }
    
    if( short_identifier && isalpha( *s1 ) )
      break;
    if( expected_type == FUNCTION_TYPE && isdigit( *s1 ) )
      break;
  }
  
  *something = '\0';
  error_ptr = buffer = s1;
  
  if( expected_type == CLOSED_VALUE_TYPE ) {
    if( s1 > end )
      return UNCLOSED_VALUE_ERROR;
    if( something == s2 )
      return EMPTY_VALUE_ERROR;
    return expected_type;
  }
  
  if( something == s2 )
    return SYNTAX_ERROR;
  
  if( expected_type == PRE_TAG_VALUE_TYPE )
    return expected_type;
  
  if( s1 > end )
    return expected_type;
  
  if( expected_type == IDENTIFIER_TYPE ) {
    if( isalpha( *s1 ) || PRE_FUNC_TAG( *s1 )
      || IDENT_JOINER( *s1 ) || POST_ARG_TAG( *s1 ) )
      return expected_type;
  }
  else
    if( isdigit( *s1 ) || POST_FUNC_TAG( *s1 )
      || VALUE_TAG( *s1 ) || PRE_VALUE_TAG( *s1 )
      || FUNC_JOINER( *s1 ) )
      return expected_type;
  
  if( PRE_FUNC_TAG( *s1 ) )
    return IDENTIFIER_TYPE;
  
  if( short_identifier )
    return SYNTAX_ERROR;
  
  return FUNCTION_TYPE;
}

/*
 *  get_basic
 *
 *  Used by parse_set ( ... )
 *
 */

int get_basic( const char* buffer, const char* end, char* basic )
{
  int alpha_number;
  
  if( !( alpha_number = isalpha( *buffer ) != 0 ) )
    if( !( alpha_number = ( isdigit( *buffer ) != 0 )*2 ) ) {
      error_ptr = buffer;
      return SYNTAX_ERROR;
    }
  
  *basic++ = *buffer++;
  
  for( ; buffer <= end; *basic++ = *buffer++ )
    if( alpha_number == 1 && !isalpha( *buffer )
      || alpha_number == 2 && !isdigit( *buffer ) ) {

      error_ptr = buffer;

      if( alpha_number == 2 ) {
        if( isalpha( *buffer ) || PRE_FUNC_TAG( *buffer )
          || IDENT_JOINER( *buffer ) )
          return IDENTIFIER_TYPE;
        if( POST_FUNC_TAG( *buffer ) )
          return MISSING_FUNCTION_ERROR;
        if( VALUE_TAG( *buffer ) || PRE_VALUE_TAG( *buffer ) )
          return SYNTAX_ERROR;
      }
      else {
        if( isdigit( *buffer ) ) {
          for( ; ( isalnum( *buffer ) || IDENT_JOINER( *buffer ) )
            && buffer <= end; buffer++ ) ;
          if( buffer > end || POST_FUNC_TAG( *buffer )
            || VALUE_TAG( *buffer ) || PRE_VALUE_TAG( *buffer ) )
            return FUNCTION_TYPE;
          if( PRE_FUNC_TAG( *buffer ) )
            return IDENTIFIER_TYPE;
          return UNEXPECTED_CHAR_ERROR;
        }
        if( POST_FUNC_TAG( *buffer ) || VALUE_TAG( *buffer )
          || PRE_VALUE_TAG( *buffer ) )
          return FUNCTION_TYPE;
        if( FUNC_JOINER( *buffer ) ) {
          for( ; buffer <= end; buffer++ )
            if( PRE_FUNC_TAG( *buffer ) )
              return IDENTIFIER_TYPE;
          return FUNCTION_TYPE;
        }
        if( PRE_FUNC_TAG( *buffer ) )
          return IDENTIFIER_TYPE;
      }
      return UNEXPECTED_CHAR_ERROR;
    }
  
  *basic = '\0';
  
  return BASIC_TYPE;
}

/*
 *  parse_set
 *
 *  Parses a string blocked by 'buffer' and 'end' and retrieves relavent
 *  information, filling the queues.  syntax checks performed on string
 *
 *  Must call init_queues( ) at least once before it's initial use
 *  and clear_queues( ) after every use.
 *
 *  Used by convert_set ( ... )
 *
 *  Returns:
 *      BASIC_TYPE
 *      PARSE_SUCCESSFUL
 *    Or < 0 for error
 *
 */

int parse_set( const char* buffer, const char* end )
{
  char    something [ end-buffer ];
  int     pre_ident;
  int            rv;
  int          k, i;
  bool closed_value;
  
  if( ( rv = get_basic( buffer, end, something ) ) < 0 )
    return rv;
  
  if( rv == BASIC_TYPE ) {
    if( ( k = push_ident( something ) ) < 0 )
      return k;
    return BASIC_TYPE;
  }
  
  if( rv == IDENTIFIER_TYPE ) {
    if( ( rv = get_something( buffer, end, something, IDENTIFIER_TYPE ) ) < 0 )
      return rv;
    
    if( ( k = push_ident( something ) ) < 0 )
      return k;
    
    while( IDENT_JOINER( *buffer ) ) {
      if( ( rv = get_something( ++buffer, end, something,
        IDENTIFIER_TYPE ) ) < 0 )
        return rv;
    
      if( rv == FUNCTION_TYPE )
        return UNEXPECTED_TYPE_ERROR;
    
      if( ( k = push_ident( something, k ) ) < 0 )
        return k;
    }
    
    if( !isalpha( *buffer ) )
      if( PRE_FUNC_TAG( *buffer ) )
        buffer++;
      else if( POST_FUNC_TAG( *buffer ) )
        return MISSING_FUNCTION_ERROR;
  }
  
  for( i = 0; ; i++ ) {
    if( ( rv = get_something( buffer, end, something, FUNCTION_TYPE ) ) < 0 )
      return rv;
    
    if( rv == IDENTIFIER_TYPE ) {
      if( i == 0 || !PRE_FUNC_TAG( *buffer ) )
        error_ptr--;
      return i == 0 ? DOUBLE_TARGET_ERROR : SYNTAX_ERROR;
    }
    
    if( ( k = push_func( something ) ) < 0 )
      return k;
    
    if( buffer > end )
      return PARSE_SUCCESSFUL;
    
    closed_value = FALSE;
    while( VALUE_TAG( *buffer ) || PRE_VALUE_TAG( *buffer ) ) {
      if( PRE_VALUE_TAG( *buffer ) )
        closed_value = TRUE;
      if( ( rv = get_something( ++buffer, end, something,
        closed_value ? CLOSED_VALUE_TYPE : PRE_TAG_VALUE_TYPE ) ) < 0 )
        return rv;
      
      if( ( k = push_value( something, k ) ) < 0 )
        return k;
      
      if( closed_value )
        buffer++;
      
      if( buffer > end )
        return PARSE_SUCCESSFUL;
    }

    if( isdigit( *buffer ) )
      break;
    
    if( FUNC_JOINER( *buffer ) ) {
      buffer++;
      continue;
    }
    if( POST_FUNC_TAG( *buffer ) ) {
      buffer++;
      break;
    }
    
    if( closed_value && isalpha( *buffer ) )
      break;
    
    error_ptr = buffer;
    return UNEXPECTED_CHAR_ERROR;
  }
  
  for( k = -1; ; ) {
    if( ( rv = get_something( buffer, end, something,
      IDENTIFIER_TYPE ) ) < 0 )
      return rv;
    
    if( rv == FUNCTION_TYPE )
      return SYNTAX_ERROR;
    
    if( ( k = push_ident( something, k ) ) < 0 )
      return k;
    
    if( buffer > end )
      break;
    
    if( PRE_FUNC_TAG( *buffer ) )
      return SYNTAX_ERROR;
    
    if( isalpha( *buffer ) )
      return UNEXPECTED_TYPE_ERROR;
    
    if( POST_ARG_TAG( *buffer ) )
      k = -1;
    buffer++;  // POST_ARG_TAG or IDENT_JOINER
  }
  return PARSE_SUCCESSFUL;
}

typedef class One_Func {
 public:
  int func_index;
  int       valc;
  char**  values;
} one_func;

typedef class Func_Set {
 public:
  int          argc;
  int*      targetc;
  int**     targets;
  int          fncc;
  one_func* functns;
  
  Func_Set ( ) {
    argc = -1;
  }
  
  ~Func_Set ( ) {
    if( argc != -1 ) {
      for( int i = 0; i < fncc; i++ )
        if( functns[ i ].valc > 0 ) {
          for( int j = 0; j < functns[ i ].valc; j++ )
            delete[] functns[ i ].values[ j ];
          delete[] functns[ i ].values;
        }
      delete[] functns;
      
      for( int i = 0; i < argc; i++ )
        delete[] targets[ i ];
      delete[] targets;
      delete[] targetc;
    }
  }
} func_set;

/*
 *  Returns NUMBER_TYPE
 *          ENUMED_TYPE
 *          FUNCTION_TYPE
 *    Or and error value < 0
 *  
 *  if FUNCTION_TYPE is returned, funcset is filled and must be
 *  deallocated when finished.
 *
 */

int convert_set( const char* buffer, const char* end, func_set*& funcset )
{
  int              i, j;
  int              argc;
  int              jagc;
  int              fncc;
  int              valc;
  int                rv;
  int             found;
  int        targ_index;
  int             error = 0;
  
  if( ( rv = parse_set( buffer, end ) ) < 0 ) {
    clear_queues( );
    return rv;
  }
  
  if( rv == BASIC_TYPE ) {
    if( isdigit( *cond_idents[ 0 ][ 0 ] ) ) {
      funcset = (func_set*) atoi( cond_idents[ 0 ][ 0 ] );
      clear_queues( );
      return NUMBER_TYPE;
    }
    (char*) funcset = cond_idents[ 0 ][ 0 ];
    cond_idents[ 0 ][ 0 ] = NULL;
    clear_queues( );
    return ENUMED_TYPE;
  }
  
  error_ptr = end;
  for( argc = 0; argc < MAX_ARG && cond_idents[ argc ][ 0 ] != NULL; argc++ ) ;
  for( fncc = 0; fncc < MAX_DUB_FUNC && cond_functs[ fncc ] != NULL; fncc++ ) ;
  if( fncc == 0 ) {
    clear_queues( );
    return MISSING_FUNCTION_ERROR;
  }
  
  funcset = new func_set;
  
  funcset->fncc = fncc;
  funcset->functns = new one_func [ fncc ];
    
  for( i = 0; i < fncc; i++ ) {
    found = -1;
    for( int k = 0; check_functions[ k ].name != NULL; k++ ) {
      if( !strcmp( check_functions[ k ].name, cond_functs[ i ] )
        || ( check_functions[ k ].shrtn != NULL
        && !strcmp( check_functions[ k ].shrtn, cond_functs[ i ] ) ) ) {
        found = k;
        break;
      }
    }
    if( found == -1 ) {
      error = UNKNOWN_FUNCTION_ERROR;
      break;
    }

    if( argc > check_functions[ found ].argc )
      error = TOO_MANY_TARGETS_ERROR;
    else if( argc < check_functions[ found ].argc )
      error = NOT_ENOUGH_TARGETS_ERROR;
    if( error )
      break;
    
    for( j = 0; j < MAX_VALUES && cond_values[ i ][ j ] != NULL; j++ ) ;  
    
    if( j < check_functions[ found ].valc )
      error = NOT_ENOUGH_VALUES_ERROR;
    else if( j > check_functions[ found ].valc )
      error = TOO_MANY_VALUES_ERROR;
    if( error )
      break;
    
    funcset->functns[ i ].func_index = found;
  }
  if( error ) {
    alloc_error_value( cond_functs[ i ] );
    delete[] funcset->functns;
    delete funcset;
    clear_queues( );
    return error;
  }
  
  funcset->argc = -1;  // Incomplete Indicator
  funcset->targetc = new int [ argc ];
  funcset->targets = new int* [ argc ];

  for( i = 0; i < argc; i++ ) {
    for( jagc = 1; jagc < MAX_DUB_ARG && cond_idents[ i ][ jagc ]
      != NULL; jagc++ ) ;
    funcset->targetc[ i ] = jagc;
    funcset->targets[ i ] = new int [ jagc ];
    
    for( j = 0; j < jagc; j++ ) {
      if( isdigit( *cond_idents[ i ][ j ] ) ) {
        if( strlen( cond_idents[ i ][ j ] ) > 2 ) {
          error = UNINDEXED_IDENTIFIER_ERROR;
          break;
        }
        targ_index = atoi( cond_idents[ i ][ j ] );
      }
      else
        targ_index = -1;
      
      found = -1;
      for( int k = 0; target_list[ k ].name != NULL; k++ ) {
        if( targ_index == -1 ) {
          if( !strcmp( target_list[ k ].name, cond_idents[ i ][ j ] ) ) {
            found = k;
            break;
          }
        }
        else if( target_list[ k ].shrtn == targ_index ) {
            found = k;
            break;
        }
      }
      if( found == -1 ) {
        error = UNKNOWN_IDENTIFIER_ERROR;
        break;
      }
      funcset->targets[ i ][ j ] = found;
    }
    if( error )
      break;
  }
  if( error ) {
    alloc_error_value( cond_idents[ i ][ j ] );
    for( j = 0; j < argc && j < i+1; j++ )
      delete[] funcset->targets[ j ];
    delete[] funcset->targets;
    delete[] funcset->targetc;
    delete[] funcset->functns;
    delete funcset;
    clear_queues( );
    return error;
  }

  for( i = 0; i < fncc; i++ ) {
    for( valc = 0; valc < MAX_VALUES && cond_values[ i ][ valc ]
      != NULL; valc++ ) ;
    if( ( funcset->functns[ i ].valc = valc ) > 0 ) {
      funcset->functns[ i ].values = new char* [ valc ];
    
      for( j = 0; j < valc; j++ ) {
        funcset->functns[ i ].values[ j ] = cond_values[ i ][ j ];
        cond_values[ i ][ j ] = NULL;
      }
    }
  }
  
  clear_queues( );
  
  funcset->argc = argc;  // Success
  return FUNCTION_TYPE;
}

/*
 *  BINARY PAIRING & ORDER OF PRECEDENCE FORMING
 */

typedef class Binary_Pair {
 public:
  const char*     start;
  Binary_Pair*    pair1;
  const char* operation;
  const char*       end;
  Binary_Pair*    pair2;
  
  Binary_Pair( ) {
    start = NULL;
    pair1 = NULL;
    end   = NULL;
    pair2 = NULL;
  }
  
  Binary_Pair( class Binary_Pair* bpair ) {
    start = NULL;
    pair1 = bpair;
    end   = NULL;
    pair2 = NULL;
  }
  
  ~Binary_Pair( ) {
    if( pair1 != NULL )
      delete pair1;
    if( pair2 != NULL )
      delete pair2;
  }
  
  int Check_Syntax( void );
  
} binary_pair;

/*
 *  BINARY OPERATOR TABLE
 */

typedef bool op_checker ( double, double );
typedef int op_func
  ( int, binary_pair*, int, binary_pair*, double*&, op_checker*, double );

typedef class Op_Funcs {
 public:
  char    opsign;
  op_func*  func;
} op_funcs;

op_func or_operator;
op_func and_operator;
op_func equals_operator;
op_func noteql_operator;
op_func lessthan_operator;
op_func greater_operator;
op_func lesseql_operator;
op_func greateql_operator;
op_func add_operator;
op_func minus_operator;
op_func multiply_operator;
op_func divide_operator;

#define MAX_PRECEDENCE_ROWS    7

op_funcs op_functions [ MAX_PRECEDENCE_ROWS ][ 5 ] =
{
  {
    { '|', or_operator       },
    { '\0', NULL             }
  },
  {
    { '&', and_operator      },
    { '\0', NULL             }
  },
  {
    { '=', equals_operator   },
    { '!', noteql_operator   },
    { '\0', NULL             }
  },
  {
    { '<', lessthan_operator },
    { '>', greater_operator  },
    { '[', lesseql_operator  },
    { ']', greateql_operator },
    { '\0', NULL             }
  },
  {
    { '+', add_operator      },
    { '-', minus_operator    },
    { '\0', NULL             }
  },
  {
    { '*', multiply_operator },
    { '/', divide_operator   },
    { '\0', NULL             }
  },
  {
    { '(', NULL              },
    { ')', NULL              },
    { '\0', NULL             }
  }
};


/*
 * op_checkers
 *
 */

bool equals_check   ( double x, double y ) { return x == y; }
bool noteql_check   ( double x, double y ) { return x != y; }
bool lessthan_check ( double x, double y ) { return x < y; }
bool greater_check  ( double x, double y ) { return x > y; }
bool lesseql_check  ( double x, double y ) { return x <= y; }
bool greateql_check ( double x, double y ) { return x >= y; }

/*
 *             **** BINARY OPERATORS ****
 */

const char* fbpair_type_ptr = NULL;
const char* number_type_ptr = "N";  // internal use
const char* enumed_type_ptr = "E";  // internal use
const char* functs_type_ptr = "F";  // internal use

int	operate_fbpair	( binary_pair*, double*&, op_checker*, double );
int	operate_funcset	( func_set*, double*&, op_checker*, double );


/*
 *  TRUTH OPERATORS
 */

int or_operator( int ltype, binary_pair* lset, int rtype, binary_pair* rset,
  double*& result, op_checker* checker, double checker_arg2 )
{
  int          return_value;
  int             wrk_value;
  double*        wrk_result;
    
  if( ltype == (int) number_type_ptr )
    wrk_value = (int) lset != 0;
  else {
    if( ltype == (int) fbpair_type_ptr )
      return_value = operate_fbpair( lset, wrk_result, noteql_check, 0.0 );
    else  // ltype == functs_type_ptr
      return_value =
        operate_funcset( (func_set*) lset, wrk_result, noteql_check, 0.0 );
    if( ( wrk_value = return_value != -1 ) && return_value != 0 )
      delete[] wrk_result;
  }
  
  if( wrk_value == 0 ) {
    if( rtype == (int) number_type_ptr )
      wrk_value = (int) rset != 0;
    else {
      if( rtype == (int) fbpair_type_ptr )
        return_value = operate_fbpair( rset, wrk_result, noteql_check, 0.0 );
      else  // rtype == functs_type_ptr
        return_value =
          operate_funcset( (func_set*) rset, wrk_result, noteql_check, 0.0 );
      if( ( wrk_value = return_value != -1 ) && return_value != 0 )
        delete[] wrk_result;
    }
  }

  if( checker != NULL && !checker( wrk_value, checker_arg2 ) )
    return -1;
  result = (double*) wrk_value;
  return 0;
}

int and_operator( int ltype, binary_pair* lset, int rtype, binary_pair* rset,
  double*& result, op_checker* checker, double checker_arg2 )
{
  int          return_value;
  int             wrk_value;
  double*        wrk_result;
    
  if( ltype == (int) number_type_ptr )
    wrk_value = (int) lset != 0;
  else {
    if( ltype == (int) fbpair_type_ptr )
      return_value = operate_fbpair( lset, wrk_result, noteql_check, 0.0 );
    else  // ltype == functs_type_ptr
      return_value =
        operate_funcset( (func_set*) lset, wrk_result, noteql_check, 0.0 );
    if( ( wrk_value = return_value != -1 ) && return_value != 0 )
      delete[] wrk_result;
  }
  
  if( wrk_value == 1 ) {
    if( rtype == (int) number_type_ptr )
      wrk_value = (int) rset != 0;
    else {
      if( rtype == (int) fbpair_type_ptr )
        return_value = operate_fbpair( rset, wrk_result, noteql_check, 0.0 );
      else  // rtype == functs_type_ptr
        return_value =
          operate_funcset( (func_set*) rset, wrk_result, noteql_check, 0.0 );
      if( ( wrk_value = return_value != -1 ) && return_value != 0 )
        delete[] wrk_result;
    }
  }

  if( checker != NULL && !checker( wrk_value, checker_arg2 ) )
    return -1;
  result = (double*) wrk_value;
  return 0;
}

int equals_operator( int ltype, binary_pair* lset, int rtype, binary_pair* rset,
  double*& result, op_checker* checker, double checker_arg2 )
{
  int             wrk_value = 1;
  double*           lresult;
  double*           rresult;
  int        lcount, rcount;
  double     lckarg, rckarg;
  op_checker*      lchecker = NULL;
  op_checker*      rchecker = NULL;
  int                     i;
  
  if( ltype == (int) number_type_ptr ) {
    lcount = 0;
    rckarg = (int) ( lresult = (double*) lset );
    rchecker = equals_check;
  }
  if( rtype == (int) number_type_ptr ) {
    rcount = 0;
    lckarg = (int) ( rresult = (double*) rset );
    lchecker = equals_check;
  }
    
  if( ltype != (int) number_type_ptr ) {
    if( ltype == (int) fbpair_type_ptr )
      lcount = operate_fbpair( lset, lresult, lchecker, lckarg );
    else  // ltype == functs_type_ptr
      lcount =
        operate_funcset( (func_set*) lset, lresult, lchecker, lckarg );
    if( lchecker != NULL )
      wrk_value = lcount != -1;
    else {
      if( lcount == 0 )
        rckarg = (int) lresult;
      else
        for( i = 1, rckarg = lresult[ 0 ]; i < lcount; i++ )
          if( lresult[ i ] != rckarg ) {
            wrk_value = 0;
            break;
          }
      rchecker = equals_check;
    }
  }
  
  if( rtype != (int) number_type_ptr && wrk_value != 0 ) {
    if( rtype == (int) fbpair_type_ptr )
      rcount = operate_fbpair( rset, rresult, rchecker, rckarg );
    else  // rtype == functs_type_ptr
      rcount =
        operate_funcset( (func_set*) rset, rresult, rchecker, rckarg );
    wrk_value = rcount != -1;
  }
  
  if( ltype == (int) number_type_ptr && rtype == (int) number_type_ptr )
    wrk_value = rckarg == lckarg;
  
  
  if( lcount > 0 )
    delete[] lresult;
  if( rcount > 0 )
    delete[] rresult;
  
  if( checker != NULL && !checker( wrk_value, checker_arg2 ) )
    return -1;
  result = (double*) wrk_value;
  return 0;
}

int noteql_operator( int ltype, binary_pair* lset, int rtype, binary_pair* rset,
  double*& result, op_checker* checker, double checker_arg2 )
{
  int             wrk_value = 1;
  double*           lresult;
  double*           rresult;
  int        lcount, rcount;
  double     lckarg, rckarg;
  op_checker*      lchecker = NULL;
  op_checker*      rchecker = NULL;
  bool       check_unneeded = FALSE;
  
  if( ltype == (int) number_type_ptr ) {
    lcount = 0;
    rckarg = (int) ( lresult = (double*) lset );
    rchecker = noteql_check;
  }
  if( rtype == (int) number_type_ptr ) {
    rcount = 0;
    lckarg = (int) ( rresult = (double*) rset );
    lchecker = noteql_check;
  }
    
  if( ltype != (int) number_type_ptr ) {
    if( ltype == (int) fbpair_type_ptr )
      lcount = operate_fbpair( lset, lresult, lchecker, lckarg );
    else  // ltype == functs_type_ptr
      lcount =
        operate_funcset( (func_set*) lset, lresult, lchecker, lckarg );
    if( lchecker != NULL ) {
      wrk_value = lcount != -1;
      check_unneeded = TRUE;
    }
    else {
      rckarg = ( lcount == 0 ? (int) lresult : lresult[ 0 ] );
      rchecker = noteql_check;
    }
  }
  
  if( rtype != (int) number_type_ptr ) {
    if( rtype == (int) fbpair_type_ptr )
      rcount = operate_fbpair( rset, rresult, rchecker, rckarg );
    else  // rtype == functs_type_ptr
      rcount =
        operate_funcset( (func_set*) rset, rresult, rchecker, rckarg );
    if( rcount == -1 )
      wrk_value = 0;
    if( lcount == 0 || wrk_value == 0 )
      check_unneeded = TRUE;
  }
  
  if( !check_unneeded ) {
    for( int i = 0; i < rcount || ( i == 0 && rcount == 0 ); i++ ) {
      for( int j = 0; j < lcount || ( j == 0 && lcount == 0 ); j++ )
        if( ( lcount == 0 ? (int) lresult : lresult[ j ] ) == 
          ( rcount == 0 ? (int) rresult : rresult[ i ] ) ) {
          wrk_value = 0;
          break;
        }
      if( wrk_value == 0 )
        break;
    }
  }
  
  if( lcount > 0 )
    delete[] lresult;
  if( rcount > 0 )
    delete[] rresult;
  
  if( checker != NULL && !checker( wrk_value, checker_arg2 ) )
    return -1;
  result = (double*) wrk_value;
  return 0;
}

int lessthan_operator( int ltype, binary_pair* lset, int rtype, binary_pair* rset,
  double*& result, op_checker* checker, double checker_arg2 )
{
  int             wrk_value;
  double*           lresult;
  double*           rresult;
  int        lcount, rcount;
  double     lckarg, rckarg;
  op_checker*      lchecker = NULL;
  op_checker*      rchecker = NULL;
  int                     i;
  
  if( ltype == (int) number_type_ptr ) {
    lcount = 0;
    rckarg = (int) ( lresult = (double*) lset );
    rchecker = greateql_check;
  }
  if( rtype == (int) number_type_ptr ) {
    rcount = 0;
    lckarg = (int) ( rresult = (double*) rset );
    lchecker = lessthan_check;
  }
    
  if( ltype != (int) number_type_ptr ) {
    if( ltype == (int) fbpair_type_ptr )
      lcount = operate_fbpair( lset, lresult, lchecker, lckarg );
    else  // ltype == functs_type_ptr
      lcount =
        operate_funcset( (func_set*) lset, lresult, lchecker, lckarg );
    if( lchecker != NULL )
      wrk_value = lcount != -1;
    else {
      if( lcount == 0 )
        rckarg = (int) lresult;
      else
        for( i = 1, rckarg = lresult[ 0 ]; i < lcount; i++ )
          if( lresult[ i ] > rckarg )
            rckarg = lresult[ i ];
      rchecker = greateql_check;
    }
  }
  
  if( rtype != (int) number_type_ptr ) {
    if( rtype == (int) fbpair_type_ptr )
      rcount = operate_fbpair( rset, rresult, rchecker, rckarg );
    else  // rtype == functs_type_ptr
      rcount =
        operate_funcset( (func_set*) rset, rresult, rchecker, rckarg );
    wrk_value = rcount == -1;
  }
  
  if( ltype == (int) number_type_ptr && rtype == (int) number_type_ptr )
    wrk_value = rckarg < lckarg;
  
  if( lcount > 0 )
    delete[] lresult;
  if( rcount > 0 )
    delete[] rresult;
  
  if( checker != NULL && !checker( wrk_value, checker_arg2 ) )
    return -1;
  result = (double*) wrk_value;
  return 0;
}

int greater_operator( int ltype, binary_pair* lset, int rtype, binary_pair* rset,
  double*& result, op_checker* checker, double checker_arg2 )
{
  int             wrk_value;
  double*           lresult;
  double*           rresult;
  int        lcount, rcount;
  double     lckarg, rckarg;
  op_checker*      lchecker = NULL;
  op_checker*      rchecker = NULL;
  int                     i;
  
  if( ltype == (int) number_type_ptr ) {
    lcount = 0;
    rckarg = (int) ( lresult = (double*) lset );
    rchecker = lesseql_check;
  }
  if( rtype == (int) number_type_ptr ) {
    rcount = 0;
    lckarg = (int) ( rresult = (double*) rset );
    lchecker = greater_check;
  }
    
  if( ltype != (int) number_type_ptr ) {
    if( ltype == (int) fbpair_type_ptr )
      lcount = operate_fbpair( lset, lresult, lchecker, lckarg );
    else  // ltype == functs_type_ptr
      lcount =
        operate_funcset( (func_set*) lset, lresult, lchecker, lckarg );
    if( lchecker != NULL )
      wrk_value = lcount != -1;
    else {
      if( lcount == 0 )
        rckarg = (int) lresult;
      else
        for( i = 1, rckarg = lresult[ 0 ]; i < lcount; i++ )
          if( lresult[ i ] > rckarg )
            rckarg = lresult[ i ];
      rchecker = lesseql_check;
    }
  }
  
  if( rtype != (int) number_type_ptr ) {
    if( rtype == (int) fbpair_type_ptr )
      rcount = operate_fbpair( rset, rresult, rchecker, rckarg );
    else  // rtype == functs_type_ptr
      rcount =
        operate_funcset( (func_set*) rset, rresult, rchecker, rckarg );
    wrk_value = rcount == -1;
  }
  
  if( ltype == (int) number_type_ptr && rtype == (int) number_type_ptr )
    wrk_value = rckarg > lckarg;
  
  if( lcount > 0 )
    delete[] lresult;
  if( rcount > 0 )
    delete[] rresult;
  
  if( checker != NULL && !checker( wrk_value, checker_arg2 ) )
    return -1;
  result = (double*) wrk_value;
  return 0;
}

int lesseql_operator( int ltype, binary_pair* lset, int rtype, binary_pair* rset,
  double*& result, op_checker* checker, double checker_arg2 )
{
  int             wrk_value;
  double*           lresult;
  double*           rresult;
  int        lcount, rcount;
  double     lckarg, rckarg;
  op_checker*      lchecker = NULL;
  op_checker*      rchecker = NULL;
  int                     i;
  
  if( ltype == (int) number_type_ptr ) {
    lcount = 0;
    rckarg = (int) ( lresult = (double*) lset );
    rchecker = greater_check;
  }
  if( rtype == (int) number_type_ptr ) {
    rcount = 0;
    lckarg = (int) ( rresult = (double*) rset );
    lchecker = lesseql_check;
  }
    
  if( ltype != (int) number_type_ptr ) {
    if( ltype == (int) fbpair_type_ptr )
      lcount = operate_fbpair( lset, lresult, lchecker, lckarg );
    else  // ltype == functs_type_ptr
      lcount =
        operate_funcset( (func_set*) lset, lresult, lchecker, lckarg );
    if( lchecker != NULL )
      wrk_value = lcount != -1;
    else {
      if( lcount == 0 )
        rckarg = (int) lresult;
      else
        for( i = 1, rckarg = lresult[ 0 ]; i < lcount; i++ )
          if( lresult[ i ] > rckarg )
            rckarg = lresult[ i ];
      rchecker = greater_check;
    }
  }
  
  if( rtype != (int) number_type_ptr ) {
    if( rtype == (int) fbpair_type_ptr )
      rcount = operate_fbpair( rset, rresult, rchecker, rckarg );
    else  // rtype == functs_type_ptr
      rcount =
        operate_funcset( (func_set*) rset, rresult, rchecker, rckarg );
    wrk_value = rcount == -1;
  }
  
  if( ltype == (int) number_type_ptr && rtype == (int) number_type_ptr )
    wrk_value = rckarg <= lckarg;
  
  if( lcount > 0 )
    delete[] lresult;
  if( rcount > 0 )
    delete[] rresult;
  
  if( checker != NULL && !checker( wrk_value, checker_arg2 ) )
    return -1;
  result = (double*) wrk_value;
  return 0;
}

int greateql_operator( int ltype, binary_pair* lset, int rtype, binary_pair* rset,
  double*& result, op_checker* checker, double checker_arg2 )
{
  int             wrk_value;
  double*           lresult;
  double*           rresult;
  int        lcount, rcount;
  double     lckarg, rckarg;
  op_checker*      lchecker = NULL;
  op_checker*      rchecker = NULL;
  int                     i;
  
  if( ltype == (int) number_type_ptr ) {
    lcount = 0;
    rckarg = (int) ( lresult = (double*) lset );
    rchecker = lessthan_check;
  }
  if( rtype == (int) number_type_ptr ) {
    rcount = 0;
    lckarg = (int) ( rresult = (double*) rset );
    lchecker = greateql_check;
  }
    
  if( ltype != (int) number_type_ptr ) {
    if( ltype == (int) fbpair_type_ptr )
      lcount = operate_fbpair( lset, lresult, lchecker, lckarg );
    else  // ltype == functs_type_ptr
      lcount =
        operate_funcset( (func_set*) lset, lresult, lchecker, lckarg );
    if( lchecker != NULL )
      wrk_value = lcount != -1;
    else {
      if( lcount == 0 )
        rckarg = (int) lresult;
      else
        for( i = 1, rckarg = lresult[ 0 ]; i < lcount; i++ )
          if( lresult[ i ] > rckarg )
            rckarg = lresult[ i ];
      rchecker = lessthan_check;
    }
  }
  
  if( rtype != (int) number_type_ptr ) {
    if( rtype == (int) fbpair_type_ptr )
      rcount = operate_fbpair( rset, rresult, rchecker, rckarg );
    else  // rtype == functs_type_ptr
      rcount =
        operate_funcset( (func_set*) rset, rresult, rchecker, rckarg );
    wrk_value = rcount == -1;
  }
  
  if( ltype == (int) number_type_ptr && rtype == (int) number_type_ptr )
    wrk_value = rckarg >= lckarg;
  
  if( lcount > 0 )
    delete[] lresult;
  if( rcount > 0 )
    delete[] rresult;
  
  if( checker != NULL && !checker( wrk_value, checker_arg2 ) )
    return -1;
  result = (double*) wrk_value;
  return 0;
}

/*
 *  ARITHMETIC OPERATORS
 */

#define NUM_SET_LIMIT  256

int add_operator( int ltype, binary_pair* lset, int rtype, binary_pair* rset,
  double*& result, op_checker* checker, double checker_arg2 )
{
  double*           lresult;
  double*           rresult;
  int        lcount, rcount;
  op_checker*      lchecker = NULL;
  op_checker*      rchecker = NULL;
  int               resultc;
  
  if( ltype == (int) number_type_ptr ) {
    lcount = 0;
    if( (int) ( lresult = (double*) lset ) == 0 )
      rchecker = checker;
  }
  if( rtype == (int) number_type_ptr ) {
    rcount = 0;
    if( (int) ( rresult = (double*) rset ) == 0 )
      lchecker = checker;
  }
  
  if( ltype != (int) number_type_ptr ) {
    if( ltype == (int) fbpair_type_ptr )
      lcount = operate_fbpair( lset, lresult, lchecker, checker_arg2 );
    else  // ltype == functs_type_ptr
      lcount =
        operate_funcset( (func_set*) lset, lresult, lchecker, checker_arg2 );
    if( lcount == -1 )
      return -1;
  }
  
  if( rtype != (int) number_type_ptr ) {
    if( rtype == (int) fbpair_type_ptr )
      rcount = operate_fbpair( rset, rresult, rchecker, checker_arg2 );
    else  // rtype == functs_type_ptr
      rcount =
        operate_funcset( (func_set*) rset, rresult, rchecker, checker_arg2 );
    if( rcount == -1 )
      return -1;
  }
  
  if( lcount > 1 && rcount > 1 ) {
    if( ( resultc = lcount*rcount ) > NUM_SET_LIMIT ) {
      delete[] lresult;
      delete[] rresult;
      lresult = rresult = (double*) 0;
      resultc = 0;
    }
  }
  else
    resultc = ( lcount < rcount ? rcount : lcount );
  
  if( resultc == 0 ) {
    result = (double*) ( (int) lresult + (int) rresult );
    if( checker != NULL && !checker( (int) result, checker_arg2 ) )
      return -1;
    return 0;
  }

  result = new double [ resultc ];
  
  for( int i = 0; i < rcount || ( i == 0 && rcount == 0 ); i++ ) {
    for( int j = 0; j < lcount || ( j == 0 && lcount == 0 ); j++ ) {
      result[ i*j ] = ( lcount == 0 ? (int) lresult : lresult[ j ] )
        + ( rcount == 0 ? (int) rresult : rresult[ i ] );
      if( checker != NULL && !checker( result[ i*j ], checker_arg2 ) ) {
        delete[] result;
        resultc = -1;
        break;
      }
    }
    if( resultc == -1 )
      break;
  }    
  
  if( lcount > 0 )
    delete[] lresult;
  if( rcount > 0 )
    delete[] rresult;

  return resultc;
}

int minus_operator( int ltype, binary_pair* lset, int rtype, binary_pair* rset,
  double*& result, op_checker* checker, double checker_arg2 )
{
  double*           lresult;
  double*           rresult;
  int        lcount, rcount;
  op_checker*      lchecker = NULL;
  int               resultc;
  
  if( ltype == (int) number_type_ptr ) {
    lcount = 0;
    lresult = (double*) lset;
  }
  if( rtype == (int) number_type_ptr ) {
    rcount = 0;
    if( (int) ( rresult = (double*) rset ) == 0 )
      lchecker = checker;
  }
  
  if( ltype != (int) number_type_ptr ) {
    if( ltype == (int) fbpair_type_ptr )
      lcount = operate_fbpair( lset, lresult, lchecker, checker_arg2 );
    else  // ltype == functs_type_ptr
      lcount =
        operate_funcset( (func_set*) lset, lresult, lchecker, checker_arg2 );
    if( lcount == -1 )
      return -1;
  }
  
  if( rtype != (int) number_type_ptr ) {
    if( rtype == (int) fbpair_type_ptr )
      rcount = operate_fbpair( rset, rresult, NULL, 0.0 );
    else  // rtype == functs_type_ptr
      rcount =
        operate_funcset( (func_set*) rset, rresult, NULL, 0.0 );
  }
  
  if( lcount > 1 && rcount > 1 ) {
    if( ( resultc = lcount*rcount ) > NUM_SET_LIMIT ) {
      delete[] lresult;
      delete[] rresult;
      lresult = rresult = (double*) 0;
      resultc = 0;
    }
  }
  else
    resultc = ( lcount < rcount ? rcount : lcount );
  
  if( resultc == 0 ) {
    result = (double*) ( (int) lresult - (int) rresult );
    if( checker != NULL && !checker( (int) result, checker_arg2 ) )
      return -1;
    return 0;
  }

  result = new double [ resultc ];
  
  for( int i = 0; i < rcount || ( i == 0 && rcount == 0 ); i++ ) {
    for( int j = 0; j < lcount || ( j == 0 && lcount == 0 ); j++ ) {
      result[ i*j ] = ( lcount == 0 ? (int) lresult : lresult[ j ] )
        - ( rcount == 0 ? (int) rresult : rresult[ i ] );
      if( checker != NULL && !checker( result[ i*j ], checker_arg2 ) ) {
        delete[] result;
        resultc = -1;
        break;
      }
    }
    if( resultc == -1 )
      break;
  }    
  
  if( lcount > 0 )
    delete[] lresult;
  if( rcount > 0 )
    delete[] rresult;

  return resultc;
}

int multiply_operator( int ltype, binary_pair* lset, int rtype,
  binary_pair* rset, double*& result, op_checker* checker, double checker_arg2 )
{
  double*           lresult;
  double*           rresult;
  int        lcount, rcount;
  op_checker*      lchecker = NULL;
  op_checker*      rchecker = NULL;
  int               resultc;
  
  if( ltype == (int) number_type_ptr ) {
    lcount = 0;
    if( (int) ( lresult = (double*) lset ) == 1 )
      rchecker = checker;
  }
  if( rtype == (int) number_type_ptr ) {
    rcount = 0;
    if( (int) ( rresult = (double*) rset ) == 1 )
      lchecker = checker;
  }
  
  if( ltype != (int) number_type_ptr ) {
    if( ltype == (int) fbpair_type_ptr )
      lcount = operate_fbpair( lset, lresult, lchecker, checker_arg2 );
    else  // ltype == functs_type_ptr
      lcount =
        operate_funcset( (func_set*) lset, lresult, lchecker, checker_arg2 );
    if( lcount == -1 )
      return -1;
  }
  
  if( rtype != (int) number_type_ptr ) {
    if( rtype == (int) fbpair_type_ptr )
      rcount = operate_fbpair( rset, rresult, rchecker, checker_arg2 );
    else  // rtype == functs_type_ptr
      rcount =
        operate_funcset( (func_set*) rset, rresult, rchecker, checker_arg2 );
    if( rcount == -1 )
      return -1;
  }
  
  if( lcount > 1 && rcount > 1 ) {
    if( ( resultc = lcount*rcount ) > NUM_SET_LIMIT ) {
      delete[] lresult;
      delete[] rresult;
      lresult = rresult = (double*) 0;
      resultc = 0;
    }
  }
  else
    resultc = ( lcount < rcount ? rcount : lcount );
  
  if( resultc == 0 ) {
    result = (double*) ( (int) lresult * (int) rresult );
    if( checker != NULL && !checker( (int) result, checker_arg2 ) )
      return -1;
    return 0;
  }

  result = new double [ resultc ];
  
  for( int i = 0; i < rcount || ( i == 0 && rcount == 0 ); i++ ) {
    for( int j = 0; j < lcount || ( j == 0 && lcount == 0 ); j++ ) {
      result[ i*j ] = ( lcount == 0 ? (int) lresult : lresult[ j ] )
        * ( rcount == 0 ? (int) rresult : rresult[ i ] );
      if( checker != NULL && !checker( result[ i*j ], checker_arg2 ) ) {
        delete[] result;
        resultc = -1;
        break;
      }
    }
    if( resultc == -1 )
      break;
  }    
  
  if( lcount > 0 )
    delete[] lresult;
  if( rcount > 0 )
    delete[] rresult;

  return resultc;
}

int divide_operator( int ltype, binary_pair* lset, int rtype, binary_pair* rset,
  double*& result, op_checker* checker, double checker_arg2 )
{
  double*           lresult;
  double*           rresult;
  int        lcount, rcount;
  op_checker*      lchecker = NULL;
  int            resultc, i;
  double         tmp_double;
  bool          div_by_zero = FALSE;
  
  if( ltype == (int) number_type_ptr ) {
    lcount = 0;
    lresult = (double*) lset;
  }
  if( rtype == (int) number_type_ptr ) {
    rcount = 0;
    if( (int) ( rresult = (double*) rset ) == 1 )
      lchecker = checker;
  }
  
  if( ltype != (int) number_type_ptr ) {
    if( ltype == (int) fbpair_type_ptr )
      lcount = operate_fbpair( lset, lresult, lchecker, checker_arg2 );
    else  // ltype == functs_type_ptr
      lcount =
        operate_funcset( (func_set*) lset, lresult, lchecker, checker_arg2 );
    if( lcount == -1 )
      return -1;
  }
  
  if( rtype != (int) number_type_ptr ) {
    if( rtype == (int) fbpair_type_ptr )
      rcount = operate_fbpair( rset, rresult, NULL, 0.0 );
    else  // rtype == functs_type_ptr
      rcount =
        operate_funcset( (func_set*) rset, rresult, NULL, 0.0 );
  }
  
  if( lcount > 1 && rcount > 1 ) {
    if( ( resultc = lcount*rcount ) > NUM_SET_LIMIT ) {
      delete[] lresult;
      delete[] rresult;
      lresult = rresult = (double*) 0;
      resultc = 0;
    }
  }
  else
    resultc = ( lcount < rcount ? rcount : lcount );
  
  if( rcount == 0 && (int) rresult == 0 )
    div_by_zero = TRUE;
  for( i = 0; !div_by_zero && i < rcount; i++ )
    if( rresult[ i ] == 0.0 )
      div_by_zero = TRUE;
  if( div_by_zero ) {
    if( lcount > 0 )
      delete[] lresult;
    if( rcount > 0 )
      delete[] rresult;
    if( checker != NULL
      && !checker( (int) ( result = (double*) 0 ), checker_arg2 ) )
      return -1;
    return 0;
  }
  
  if( resultc == 0 ) {
    tmp_double = ( (int) lresult / (int) rresult );
    if( checker != NULL && !checker( tmp_double, checker_arg2 ) )
      return -1;
    result = new double [ resultc = 1 ];
    result[ 0 ] = tmp_double;
    return resultc;
  }

  result = new double [ resultc ];
  
  for( int i = 0; i < rcount || ( i == 0 && rcount == 0 ); i++ ) {
    for( int j = 0; j < lcount || ( j == 0 && lcount == 0 ); j++ ) {
      result[ i*j ] = ( lcount == 0 ? (int) lresult : lresult[ j ] )
        / ( rcount == 0 ? (int) rresult : rresult[ i ] );
      if( checker != NULL && !checker( result[ i*j ], checker_arg2 ) ) {
        delete[] result;
        resultc = -1;
        break;
      }
    }
    if( resultc == -1 )
      break;
  }    

  if( lcount > 0 )
    delete[] lresult;
  if( rcount > 0 )
    delete[] rresult;

  return resultc;
}

/*
 *  OPERATOR MAPPERS
 */

int operate_fbpair( binary_pair* fbpair, double*& result, op_checker* checker,
  double checker_arg2 )
{
  for( int i = 0; ; i++ )
    for( int j = 0; op_functions[ i ][ j ].opsign != '\0'; j++ )
      if( op_functions[ i ][ j ].opsign == *fbpair->operation ) {
        return ( op_functions[ i ][ j ].func )
          ( (int) fbpair->start, fbpair->pair1, (int) fbpair->end,
          fbpair->pair2, result, checker, checker_arg2 );
      }
}

/*
 *  operate_funcset
 *  
 *  returns the number of elements in result
 *  result is allocaed a double array of size returned int and must be
 *  deallocated when finished with
 */

int operate_funcset( func_set* funcset, double*& result, op_checker* checker,
  double checker_arg2 )
{
  int                resultc;
  double            tmp_rslt;
  int               arg_setc = 1;
  int*              arg_sets;
  target_type*       targets;
  
  for( int i = 0; i < funcset->argc; i++ )
    arg_setc *= funcset->targetc[ i ];
  resultc = arg_setc*funcset->fncc;
  
  result = new double [ resultc ];
  
  arg_sets = new int [ funcset->argc ];
  bzero( arg_sets, sizeof( int )*funcset->argc );
  
  targets = new target_type [ funcset->argc ];
  
  for( int i = 0; i < funcset->fncc; i++ )
    for( int j = 0; j < arg_setc; j++ ) {
      for( int k = 0; k < funcset->argc; k++ ) {
        targets[ k ] = target_list[ funcset->targets[ k ][ arg_sets[ k ] ] ].target;
        ++arg_sets[ k ] %= funcset->targetc[ k ];
      }
      tmp_rslt = result[ i*j ] =
        ( check_functions[ funcset->functns[ i ].func_index ].func )
        ( targets, (const char**) funcset->functns[ i ].values );
      if( checker != NULL && !checker( tmp_rslt, checker_arg2 ) ) {
        delete[] arg_sets;
        delete[] targets;
        delete[] result;
        return -1;
      }
    }
  delete[] arg_sets;
  delete[] targets;
  
  return resultc;
}


/*
 *  ORDER OF PRECEDENCE SORTING & BINARY PAIR PARSING
 */

typedef class Binary_Set {
 public:
  const char*  start;
  int          count;
  const char** operation;
  const char*  end;
  
  Binary_Set( ) {
    bzero( this, sizeof( Binary_Set ) );
  }
  
  ~Binary_Set( ) {
    if( operation != NULL )
      delete[] operation;
  }

} binary_set;

const char* next_breaker( const char* buffer )
{
  bool in_vbracket = FALSE;
  
  for( ; *buffer != '\0'; buffer++ ) {
    if( PRE_VALUE_TAG( *buffer ) )
      in_vbracket = TRUE;
    if( in_vbracket && POST_VALUE_TAG( *buffer ) )
      in_vbracket = FALSE;
    if( in_vbracket )
      continue;
    
    for( int i = 0; i < MAX_PRECEDENCE_ROWS; i++ )
      for( int j = 0; op_functions[ i ][ j ].opsign != '\0'; j++ )
        if( op_functions[ i ][ j ].opsign == *buffer )
          return buffer;
  }
  return buffer;
}

/*
 *  smash_operations
 *
 *  Returns
 *        NO_OPERATION
 *        UNARY_OPERATION
 *        BPAIR_OPERATION
 *     Or < 0 for error
 *  
 *  bpair is filled when BPAIR_OPERATION is returned and must be deallocated
 *  when finished with.
 */

int clean_brackets	( binary_set* );
int sort_operations	( binary_pair*&, binary_set*, int );

enum {
  NO_OPERATION,
  UNARY_OPERATION,
  BPAIR_OPERATION
};

int smash_operations( binary_pair*& bpair, const char* buffer )
{
  const char*  s1 = buffer;
  const char*  breaker;
  int          break_count = 0;
  binary_set*  bset;
  int          rv;
  const char*  last_breaker = NULL;
  
  if( *buffer == '\0' ) {
    return NO_OPERATION;
  }
  
  while( *( breaker = next_breaker( buffer ) ) != '\0' ) {
    if( breaker == buffer ) {
      if( *breaker == '(' &&
        last_breaker != NULL && *last_breaker == ')' ) {
        error_ptr = last_breaker;
        return MISSING_OPERATOR_AFTER_ERROR;
      }
      else if( *breaker != '('
        && ( last_breaker == NULL || *last_breaker != ')' ) ) {
        error_ptr = breaker;
        return NO_OPERAND_ERROR;
      }
    }
    else if( *breaker == '(' ) {
      error_ptr = breaker;
      return MISSING_OPERATOR_BEFORE_ERROR;
    }
    else if( last_breaker != NULL && *last_breaker == ')' ) {
      error_ptr = last_breaker;
      return MISSING_OPERATOR_AFTER_ERROR;
    }
    break_count++;
    buffer = ( last_breaker = breaker )+1;
  }

  if( break_count == 0 ) {
    return UNARY_OPERATION;
  }
  if( buffer == breaker ) {
    if( *last_breaker != ')' ) {
      error_ptr = breaker;
      return NO_OPERAND_ERROR;
    }
  }
  else if( *last_breaker == ')' ) {
    error_ptr = last_breaker;
    return MISSING_OPERATOR_AFTER_ERROR;
  }
  
  bset = new binary_set;
  bset->operation = new const char* [ break_count ];
  bset->start = buffer = s1;

  while( *( breaker = next_breaker( buffer ) ) != '\0' ) {
    bset->operation[ bset->count++ ] = breaker;
    buffer = breaker+1;
  }
  
  bset->end = breaker-1;
  
  if( ( rv = clean_brackets( bset ) ) >= 0 )
    rv = sort_operations( bpair, bset, 0 );
  
  delete bset;
  
  if( rv < 0 )
    return rv;
  
  return BPAIR_OPERATION;
}

int clean_brackets( binary_set* bset )
{
  int  brackets = 1;
  int last_open;
  int        rv;
    
  if( *bset->operation[ 0 ] != '(' )
    return FALSE;
  
  error_ptr = bset->operation[ 0 ];
  
  for( int i = 1; i < bset->count; i++ )
    if( *bset->operation[ i ] == '(' )
      brackets++;
    else if( *bset->operation[ i ] == ')' )
      if( --brackets == 0 ) {
        if( i != bset->count-1 )
          return FALSE;
        break;
      }
  if( brackets > 0 )
    return UNMATCHED_BRACKET_ERROR;
  
  if( ( bset->count -= 2 ) == 0 )
    return MEANINGLESS_BRACKET_ERROR;
  
  bset->start = bset->operation[ 0 ]+1;
  bset->end = bset->operation[ bset->count+1 ]-1;
  memcpy( bset->operation, &bset->operation[ 1 ],
    sizeof( const char* )*bset->count );

  return ( rv = clean_brackets( bset ) ) < 0 ? rv : TRUE;
}

int sort_operations( binary_pair*& bpair, binary_set* bset, int oplevel )
{
  int             brackets = 0;
  int           last_match = -1;
  binary_set*     tmp_bset;
  int              opcount;
  int               bclean;
  bool               match;
  int                   rv;
  
  bpair = NULL;
  
  for( int i = 0; i <= bset->count; i++ ) {
    if( i != bset->count ) {
      if( *bset->operation[ i ] == '(' ) {
        brackets++;
        continue;
      }
      if( *bset->operation[ i ] == ')' ) {
        if( --brackets < 0 ) {
          if( bpair != NULL ) {
            delete bpair;
            bpair = NULL;
          }
          error_ptr = bset->operation[ i ];
          return TOO_MANY_CLOSE_BRACKET_ERROR;
        }
        
        continue;
      }
    }
    if( brackets > 0 )
      continue;
    
    if( i != bset->count && bset->count != 1 ) {
      match = FALSE;
      for( int j = 0; op_functions[ oplevel ][ j ].opsign != '\0' ; j++ )
        if( op_functions[ oplevel ][ j ].opsign == *bset->operation[ i ] ) {
          match = TRUE;
          break;
        }
      if( !match )
        continue;
    }
    
    if( i == bset->count && last_match == -1 )
      return sort_operations( bpair, bset, oplevel+1 );
    
    if( ( opcount = i-(last_match+1) ) > 0 ) {
      tmp_bset            = new binary_set;
      tmp_bset->start     =
        ( last_match == -1 ? bset->start : bset->operation[ last_match ]+1 );
      tmp_bset->end       =
        ( i == bset->count ? bset->end : bset->operation[ i ]-1 );
      tmp_bset->count     = opcount;
      tmp_bset->operation = new const char* [ tmp_bset->count ];
      memcpy( tmp_bset->operation, &bset->operation[ last_match+1 ],
        sizeof( const char* )*( tmp_bset->count ) );
      if( ( bclean = clean_brackets( tmp_bset ) ) < 0 ) {
        delete tmp_bset;
        if( bpair != NULL ) {
          delete bpair;
          bpair = NULL;
        }
        return bclean;
      }
    }
    
    if( bpair == NULL ) {
      bpair = new binary_pair;
      if( opcount > 0 ) {
        rv = sort_operations( bpair->pair1, tmp_bset, bclean ? 0 : oplevel+1 );
        delete tmp_bset;
        if( rv < 0 ) {
          delete bpair;
          return rv;
        }
      }
      else
        bpair->start =
          ( last_match == -1 ? bset->start : bset->operation[ last_match ]+1 );
    }
    else {
      if( opcount > 0 ) {
        rv = sort_operations( bpair->pair2, tmp_bset, bclean ? 0 : oplevel+1 );
        delete tmp_bset;
        if( rv < 0 ) {
          delete bpair;
          return rv;
        }
      }
      else
        bpair->end =
          ( i == bset->count ? bset->end : bset->operation[ i ]-1 );
      if( i == bset->count )
        break;
      bpair = new binary_pair( bpair );
    }
    
    bpair->operation = bset->operation[ i ];
    last_match = i;
  }

  if( brackets > 0 ) {
    if( bpair != NULL ) {
      delete bpair;
      bpair = NULL;
    }
    error_ptr = bset->end+1;
    return UNMATCHED_BRACKET_ERROR;
  }

  return TRUE;
}

/*
 *  BINARY PAIR TO FUNCTION ENHANCED BINARY PAIR CONVERTION
 */

int is_enumerated( const char* word, func_set* funcset, int* value )
{
  enum_list*    enumset;
  int        enum_value;
  bool            found = FALSE;
  int              i, j;
  
  for( i = 0; i < funcset->fncc; i++ ) {
    enumset = check_functions[ funcset->functns[ i ].func_index ].enum_set;
    if( enumset == NULL )
      return FALSE;
    for( j = 0; enumset[ j ].name != NULL; j++ ) {
      if( !strcmp( enumset[ j ].name, word ) ) {
        if( found && enum_value != enumset[ j ].value ) {
          alloc_error_value( enumset[ j ].name );
          return DIFFERENT_VALUES_ENUM_ERROR;
        }
        enum_value = enumset[ j ].value;
        found = TRUE;
        break;
      }
    }
    if( enumset[ j ].name == NULL )
      return FALSE;
  }
  *value = enum_value;
  return TRUE;
}

enum_list basic_enum_set [] = {
  { "false", 0 },
  { "true",  1 },
  { NULL,    0 }
};

bool is_basic_enum( const char* word, int* value )
{
  for( int i = 0; basic_enum_set[ i ].name != NULL; i++ )
    if( !strcmp( basic_enum_set[ i ].name, word ) ) {
      *value = basic_enum_set[ i ].value;
      return TRUE;
    }
  return FALSE;
}

int convert_enumerations( binary_pair* fbpair )
{
  int    rv, rv2;
  int enum_value;
  
  if( fbpair->start == NULL && fbpair->pair1 != NULL ) {
    if( ( rv = convert_enumerations( fbpair->pair1 ) ) < 0 )
      return rv;
  } else if( fbpair->start == enumed_type_ptr ) {
    if( !is_basic_enum( (const char*) fbpair->pair1, &enum_value ) ) {
      if( fbpair->end != functs_type_ptr ) {
        error_ptr = fbpair->operation;
        return BAD_ENUM_OPERATION_ERROR;
      }
      
      if( ( rv2 = is_enumerated( (const char*) fbpair->pair1,
        (func_set*) fbpair->pair2, &enum_value ) ) < 0 )
        return rv2;
      if( !rv2 ) {
        alloc_error_value( (const char*) fbpair->pair1 );
        return UNKNOWN_IDENTIFIER_ERROR;
      }
    }

    delete[] (char*) fbpair->pair1;
    fbpair->pair1 = (binary_pair*) enum_value;
    fbpair->start = number_type_ptr;
    return 0;
  }
  
  if( fbpair->end == NULL && fbpair->pair2 != NULL ) {
    if( ( rv = convert_enumerations( fbpair->pair2 ) ) < 0 )
      return rv;
  } else if( fbpair->end == enumed_type_ptr ) {
    if( !is_basic_enum( (const char*) fbpair->pair2, &enum_value ) ) {
      if( fbpair->start != functs_type_ptr ) {
        error_ptr = fbpair->operation;
        return BAD_ENUM_OPERATION_ERROR;
      }
      
      if( ( rv2 = is_enumerated( (const char*) fbpair->pair2,
        (func_set*) fbpair->pair1, &enum_value ) ) < 0 )
        return rv2;
      if( !rv2 ) {
        alloc_error_value( (const char*) fbpair->pair2 );
        return UNKNOWN_IDENTIFIER_ERROR;
      }
    }
      
    delete[] (char*) fbpair->pair2;
    fbpair->pair2 = (binary_pair*) enum_value;
    fbpair->end = number_type_ptr;
    return 0;
  }
  
  return 0;
}

/*
 *  bpair2fbpair
 *  
 *  Returns 0 if successful, < 0 if error
 *  
 *  The created fbpair must be unlinked with unlink_fbpair( ) before deleting
 *  using the bpair destructor.
 *  There is nothing created if an error is returned but the original bpair
 *  will likely be corrupted.
 *
 *
 *  unlink_fbpair
 *
 *  Call this on a bpair before deleting it if bpair2fbpair( ) was used on it
 *  
 */

void	unlink_fbpair	( binary_pair* );

int bpair2fbpair( binary_pair* bpair )
{
  int            rv;
  func_set* funcset;
  
  if( bpair->start == NULL && bpair->pair1 != NULL ) {
    if( ( rv = bpair2fbpair( bpair->pair1 ) ) < 0 )
      return rv;
  }
  else if( bpair->start != NULL && bpair->pair1 == NULL ) {
    rv = convert_set( bpair->start, bpair->operation-1, funcset );
    if( rv < 0 )
      return rv;

    if( rv == NUMBER_TYPE )
      bpair->start = number_type_ptr;
    else if( rv == ENUMED_TYPE )
      bpair->start = enumed_type_ptr;
    else if( rv == FUNCTION_TYPE )
      bpair->start = functs_type_ptr;
    bpair->pair1 = (binary_pair*) funcset;    
  }
  
  if( bpair->end == NULL && bpair->pair2 != NULL )
    rv = bpair2fbpair( bpair->pair2 );
  else if( bpair->end != NULL && bpair->pair2 == NULL ) {
    rv = convert_set( bpair->operation+1, bpair->end, funcset );

    if( !( rv < 0 ) ) {
      if( rv == NUMBER_TYPE )
        bpair->end = number_type_ptr;
      else if( rv == ENUMED_TYPE )
        bpair->end = enumed_type_ptr;
      else if( rv == FUNCTION_TYPE )
        bpair->end = functs_type_ptr;
      bpair->pair2 = (binary_pair*) funcset;
    }
  }
  else
    return 0;
  
  if( rv < 0 ) {
    if( bpair->start == NULL )
      unlink_fbpair( bpair->pair1 );
    else {
      if( bpair->start == enumed_type_ptr )
        delete[] (char*) bpair->pair1;
      else if( bpair->start == functs_type_ptr )
        delete (func_set*) bpair->pair1;
      bpair->pair1 = NULL;
      bpair->start = NULL;
    }
    return rv;
  }
  return 0;
}

void unlink_fbpair( binary_pair* bpair )
{
  if( bpair->pair1 != NULL ) {
    if( bpair->start == NULL )
      unlink_fbpair( bpair->pair1 );
    else {
      if( bpair->start == enumed_type_ptr )
        delete[] (char*) bpair->pair1;
      else if( bpair->start == functs_type_ptr )
        delete (func_set*) bpair->pair1;
      bpair->pair1 = NULL;
      bpair->start = NULL;
    }
  }

  if( bpair->pair2 != NULL ) {
    if( bpair->end == NULL )
      unlink_fbpair( bpair->pair2 );
    else {
      if( bpair->end == enumed_type_ptr )
        delete[] (char*) bpair->pair2;
      else if( bpair->end == functs_type_ptr )
        delete (func_set*) bpair->pair2;
      bpair->pair2 = NULL;
      bpair->end = NULL;
    }
  }
}

/*
 *  MAIN FUNCTION
 */

int check_condition( const char* condition,
  check_func_list* funcs, identifier_list* idents )
{
  binary_pair* bpair;
  func_set*  funcset;
  int        rv, rv2;
  int        tmp_int;
  bool         found;
  int    check_value;
  double*    results;
  
  rv = smash_operations( bpair, condition );  
  if( rv < 0 )
    return rv;
  
  if( rv == NO_OPERATION )
    return TRUE;
  
  if( first_run ) {
    init_queues( );
    first_run = FALSE;
  }
  
  check_functions = funcs;
  target_list = idents;
  
  if( rv == UNARY_OPERATION ) {
    rv2 = convert_set( condition,
      &condition[ strlen( condition )-1 ], funcset );
    if( rv2 == ENUMED_TYPE ) {
      found = is_basic_enum( (const char*) funcset, &tmp_int );
      delete[] (char*) funcset;
      if( found ) {
        funcset = (func_set*) tmp_int;
        rv2 = NUMBER_TYPE;
      }
      else {
        alloc_error_value( condition );
        rv2 = UNKNOWN_IDENTIFIER_ERROR;
      }
    }
  }
  else if( rv == BPAIR_OPERATION ) {
    if( ( rv2 = bpair2fbpair( bpair ) ) != 0 )
      delete bpair;
    else if( ( rv2 = convert_enumerations( bpair ) ) != 0 ) {
      unlink_fbpair( bpair );
      delete bpair;
    }
  }
  if( rv2 < 0 )
    return rv2;
  
  if( rv == UNARY_OPERATION ) {
    if( rv2 == NUMBER_TYPE )
      check_value = (int) funcset;
    else {
      check_value = operate_funcset( funcset, results, noteql_check, 0.0 );
      if( check_value = check_value != -1 )
        delete[] results;
    }
  }
  else if( rv == BPAIR_OPERATION ) {
    check_value = operate_fbpair( bpair, results, noteql_check, 0.0 );
    if( check_value == -1 )
      check_value = 0;
    else {
      if( check_value != 0 )
        delete[] results;
      check_value = 1;
    }
  }
  return check_value != 0;
}

/*
 *  ERROR HANDLING
 */


/* 
 * alloc_error_value
 * 
 * error_ptr is allocated the required size in chars --remember to deallocate
 * form_error_message( ... ) will automatically deallocate if called right
 */

void alloc_error_value( const char* value, int length )
{
  char* error_val;
  
  if( length == -1 )
    length = strlen( value );
  error_ptr = error_val = new char [ length+1 ];
  strncpy( error_val, value, length );
  error_val[ length ] = '\0';
}

char* point_text( const char* text, const char* position, char* output,
  int outwidth )
{
  char          tmp [ 16 ];
  const char*    s1 = text;
  int         index = position - text;
  int     point_pos = --outwidth*5/6;
  int        length;
  
  if( index > point_pos ) {
    text += index - point_pos;
    index = point_pos;
  }
  *output++ = '|';
  length = strlen( text );
  strncpy( output, text, length > outwidth ? outwidth-3 : length );
  if( length > outwidth )
    strncpy( &output[ outwidth-3 ], "...", 3 );

  if( s1 != text )
    strncpy( output, "...", 3 );
  
  sprintf( tmp, "\n\r|%%%ds\n\r", index+1 );
  sprintf( &output[ length > outwidth ? outwidth : length ], tmp, "^" );
  
  return output-1;
}

char* form_error_message( int error_num, const char* line,
  char* output, int outwidth )
{
  char         buffer [ 256 ];
  char*    error_text;
  
  switch( error_num ) {
    case SYNTAX_ERROR:
      error_text = "Syntax error";
      break;
    case TOO_MANY_TARGETS_ERROR:
      error_text = "Too many arguments in function";
      break;
    case TOO_MANY_VALUES_ERROR:
      error_text = "Too many values for a function";
      break;
    case JOINED_ARG_LIMIT_ERROR:
      error_text = "Too many joined arguments";
      break;
    case JOINED_FUNC_LIMIT_ERROR:
      error_text = "Too many joined functions";
      break;
    case UNEXPECTED_CHAR_ERROR:
      error_text = "Unexpected character";
      break;
    case UNCLOSED_VALUE_ERROR:
      error_text = "Unclosed value";
      break;
    case EMPTY_VALUE_ERROR:
      error_text = "Empty value disallowed";
      break;
    case MISSING_FUNCTION_ERROR:
      error_text = "Operation missing a function";
      break;
    case DOUBLE_TARGET_ERROR:
      error_text = "Duplicate pre-function argument";
      break;
    case UNEXPECTED_TYPE_ERROR:
      error_text = "Unknown type";
      break;
    case UNKNOWN_FUNCTION_ERROR:
      error_text = "Unknown function";
      break;
    case NOT_ENOUGH_TARGETS_ERROR:
      error_text = "Not enough arguments in function";
      break;
    case NOT_ENOUGH_VALUES_ERROR:
      error_text = "Not enough values in function";
      break;
    case UNINDEXED_IDENTIFIER_ERROR:
      error_text = "Unindexed identifier";
      break;
    case UNKNOWN_IDENTIFIER_ERROR:
      error_text = "Unknown identifier";
      break;
    case NO_OPERAND_ERROR:
      error_text = "Missing operand";
      break;
    case UNMATCHED_BRACKET_ERROR:
      error_text = "Unclosed bracket";
      break;
    case MEANINGLESS_BRACKET_ERROR:
      error_text = "Meaningless brackets";
      break;
    case TOO_MANY_CLOSE_BRACKET_ERROR:
      error_text = "Unopened bracket";
      break;
    case DIFFERENT_VALUES_ENUM_ERROR:
      error_text = "Conflicting values in enumeration";
      break;
    case BAD_ENUM_OPERATION_ERROR:
      error_text = "Function dependant enumerations must be in direct comparison with the depending function";
      break;
    case MISSING_OPERATOR_BEFORE_ERROR:
      error_text = "Missing operator before bracket";
      break;
    case MISSING_OPERATOR_AFTER_ERROR:
      error_text = "Missing operator after closing bracket";
      break;
    case UNEXPECTED_PARSE_ERROR:
      error_text = "Unexpected error!  Please report this";
      break;
  }
  
  if( error_ptr < line || error_ptr > line+strlen( line ) ) {
    sprintf( output, "|Error] %s: '%s'\n\r", error_text, error_ptr );
    delete[] error_ptr;
  }
  else {
    sprintf( output, "|Error] %s:\n\r%s", error_text,
      point_text( line, error_ptr, buffer, outwidth ) );
  }
  return output;
}
