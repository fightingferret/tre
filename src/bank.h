bool add_bank( char_data* ch, int amount );
bool add_bank( player_data* pc, int amount );

/*
class Locker_Data : public Thing_Data
{
 public:
  char_data*  owner;

  virtual int type( ) { return LOCKER_DATA; }

  Locker_Data( char_data* ch ) {
    owner = ch;
    return;
    }
};
*/
