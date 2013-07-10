class Help_Data
{
 public:
  char*      name;
  char*      text;
  char*  immortal;
  int       level  [ 2 ];
  int    category;

  Help_Data   ( );
  ~Help_Data  ( );

  friend char* name( help_data* help ) {
    return help->name;
    }
};
