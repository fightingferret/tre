typedef class Extra_Variable
{
  public:
  char*        name;
  arg_enum     type;
  void*       value;
  void*        next;
  
  Extra_Variable( char* var_name, arg_enum var_type ) {
    name = alloc_string( var_name, MEM_PROG );
    type = var_type;
    value = NULL;
    next = NULL;
  }

  ~Extra_Variable(  ) {
    free_string( name, MEM_PROG )
    type = NONE;
    value = NULL;
    next = NULL;
  }
} extra_variable;



class Program_Runtime_Data
{
  
  private:
  
  void* first_extra;
  
  public:
  
  void new_variable( ) {
    
    
  }
  
  
  Program_Runtime_Data( ) {
    first_extra = NULL;
    
  }
  
  virtual ~Program_Runtime_Data( ) {
    
    
  }
}

void Program_Runtime_Data :: Add_Variable( const char* name, arg_enum type )
{
  
  extra_variable  extra;
  
  extra = new extra_variable( name, type );
  
  if( first_extra == NULL )
    first_extra = extra;
  else {
    void*             ptr;
    for( ptr = first_extra; ptr->next != NULL; ptr = ptr->next ) ;
    
    ptr->next = extra;
  }
}


void Program_Runtime_Data :: Remove_Variable( const char* name, arg_enum type )
{
  void*             ptr;
  extra_variable  extra;
  
  extra = new extra_variable( name, type );
  
  if( first_extra == NULL )
    ptr = first_extra;
  else
    for( ptr = first_extra; ptr->next != NULL; ptr = ptr->next );
  
}


