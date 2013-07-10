enum {
  JOINED_ARG_LIMIT_ERROR        = -3000,
  JOINED_FUNC_LIMIT_ERROR       = -3001,
  TOO_MANY_TARGETS_ERROR        = -3002,
  TOO_MANY_VALUES_ERROR         = -3003,
  NOT_ENOUGH_TARGETS_ERROR      = -3004,
  NOT_ENOUGH_VALUES_ERROR       = -3005,
  UNINDEXED_IDENTIFIER_ERROR    = -3006,
  UNKNOWN_IDENTIFIER_ERROR      = -3007,
  UNKNOWN_FUNCTION_ERROR        = -3008,
};

enum {
  UNEXPECTED_PARSE_ERROR        = -10009,
  SYNTAX_ERROR                  = -4000,
  UNEXPECTED_CHAR_ERROR         = -4001,
  UNEXPECTED_TYPE_ERROR         = -4002,
  UNCLOSED_VALUE_ERROR          = -4003,
  EMPTY_VALUE_ERROR             = -4004,
  MISSING_FUNCTION_ERROR        = -4005,
  DOUBLE_TARGET_ERROR           = -4006
};

enum {
  NO_OPERAND_ERROR              = -6000,
  UNMATCHED_BRACKET_ERROR       = -6001,
  MEANINGLESS_BRACKET_ERROR     = -6002,
  TOO_MANY_CLOSE_BRACKET_ERROR  = -6003,
  MISSING_OPERATOR_BEFORE_ERROR = -6004,
  MISSING_OPERATOR_AFTER_ERROR  = -6005
};

enum {
  DIFFERENT_VALUES_ENUM_ERROR   = -5000,
  BAD_ENUM_OPERATION_ERROR      = -5001
};

typedef visible_data* target_type;
typedef const char*    value_type;     /* DO NOT CHANGE */

typedef double check_func ( target_type*, value_type* );

typedef class Enum_List {
 public:
  const char*  name;
  int         value;
} enum_list;

typedef class Identifier_List {
 public:
  const char*      name;
  unsigned char   shrtn;
  target_type    target;
} identifier_list;

typedef class Check_Func_List {
 public:
  const char*    name;
  const char*   shrtn;
  check_func*    func;
  int            argc;
  int            valc;
  enum_list* enum_set;
} check_func_list;

int			check_condition				( const char*, check_func_list*,
                                          identifier_list* );
char*		form_error_message			( int, const char*, char*, int = 78 );

/* N.B: form_error_message( ... ) MUST be called if check_condition returns
 * a negative number.  This is to deallocate the possible error message.
 */
