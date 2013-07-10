
#define MAX_DESCDATA_INDEX 4500


#define DD_PURGABLE     0
#define DD_PROTECTED    1


extern note_data*   descdata_list  [ MAX_DESCDATA_INDEX ];


void     save_ddatabase    ( void );
void     load_ddatabase    ( void );
