#include "sys/types.h"
#include <ctype.h>
#include "stdlib.h"
#include "stdio.h"
#include "define.h"
#include "struct.h"

/* Global */
auction_array  auction_list;

/* Prototypes */
void  transfer_buyer   ( auction_data* );
void  transfer_file    ( pfile_data*, thing_array*, int );
void  transfer_file    ( pfile_data*, obj_data*, int );
void  return_seller    ( auction_data* );
bool  no_auction       ( obj_data* );
bool  stolen_auction   ( char_data*, obj_array* );
void  display_auction  ( player_data* );
bool  can_auction      ( char_data*, obj_array* );

/* Constants */
const char* undo_msg = "An auction daemon stamps up to you and hands\
 you %s. He mutters something about making up your mind while tapping\
 his forehead and storms off.";

const char* corpse_msg = "An auction daemon runs up to you.  You hand him\
 %s and a silver coin.  He looks at the corpse, looks at you, rips the\
 corpse apart, eats it, smiles happily and disappears with your silver coin.";

const char* no_auction_msg = "An auction daemon runs up to you.  You hand\
 him a silver coin and attempt to hand him %s, but he quickly refuses with\
 some mumble about items banned by the gods.  He then disappears in cloud\
 of smoke.  Only afterwards do you realize your silver coin went with him.";

const char* no_drop_msg = "An auction daemon runs up to you.  You hand\
 him a silver coin and try to hand him %s but you can't let go of it.\
 The patiently waiting auction daemon laughs rudely and disappears in\
 in cloud of smoke with your silver coin.";

const char* distinct_msg = "Due to idiosyncrasies of the accountant daemons\
 you may only auction lots of items which contain at most 5 distinct types\
 of items and distinct is unfortunately defined by them and not by what you see.";


/* auction command */
void do_auction( char_data* ch, char* argument )
{
  char                 arg  [ MAX_INPUT_LENGTH ];
  char                 tmp  [ EIGHT_LINES ];
  char                 buf  [ EIGHT_LINES ];
  auction_data*    auction;
  player_data*          pc;
  thing_array*       array;
  obj_data*          obj;
  int                  bid;
  int                 slot;
  int                count; 
  int                flags  = 0;

  if( is_mob( ch ) )
    return;

  pc = player( ch );

  if( has_permission( ch, PERM_PLAYERS )
    && !get_flags( ch, argument, &flags, "c", "Auction" ) )
    return;
 
  if( !strcasecmp( argument, "on" ) || !strcasecmp( argument, "off" ) ) {
    send( ch, "See help iflag for turning on and off auction.\n\r" );
    return;
  }
  
  if( is_set( &flags, 0 ) ) {
    if( auction_list == 0 ) {
      send( ch, "The auction block is empty.\n\r" );
      return;
    }

    sprintf( tmp, "++ Auction Block cleared by %s. ++", ch->real_name( ) );
    info( tmp, LEVEL_APPRENTICE, tmp, 1, IFLAG_AUCTION, ch );
    send( ch, "-- Auction block cleared --\n\r" );

    for( int i = 0; i < auction_list; i++ ) 
      return_seller( auction_list[i] );
    delete_list( auction_list );

    return;
  }

  if( *argument == '\0' ) {
    display_auction( pc );
    return;
  }

  if( ch->in_room == NULL
    || !is_set( &ch->in_room->room_flags, RFLAG_AUCTION_HOUSE ) ) {
    send( ch, "You must be at an auction house to do that.\n\r");
    return;
  }

  argument = one_argument( argument, arg );
  
  if( !strcasecmp( arg, "undo" ) )
  {
    for( int i = auction_list - 1; i >= 0; i-- )
      if( ( auction = auction_list[i] ) != NULL &&
        ( *argument == '\0' || atoi( argument ) == auction->slot ) ) {
        if( auction->seller != ch->pcdata->pfile )
          if( *argument == '\0' )
            continue;
          else {
            send( ch, "That lot does not belong to you.\n\r" );
            return;
          }

        if( auction->time < 45 && ( auction->buyer != NULL
          || auction->deleted ) ) {
          send( ch, "That item has been on the auction block too long\
 to remove it.\n\r" );
          return;
        }

        auction_list -= auction;

        sprintf( tmp, "%s has removed item #%d, %s from the auction block.",
          ch->real_name( ), auction->slot, list_name( NULL ,
          &auction->contents, TRUE, buf ) );
        info( tmp, LEVEL_APPRENTICE, tmp, IFLAG_AUCTION, 2, ch );

        fsend( ch, undo_msg, list_name( NULL, &auction->contents, TRUE, buf ) );
        
        for( int j = auction->contents - 1; j >= 0; j-- )
          if( ( obj = object( auction->contents[j] ) ) != NULL ) {
            obj = (obj_data*) obj->From( obj->number );
            set_owner( obj, ch, NULL );
            obj->To( &ch->contents );
            consolidate( obj );
          }
        
        delete auction;
        return;
      }
    
    if( *argument != '\0' )
      send( ch, "There is no such auction.\n\r" );
    else
      send( ch, "You have no items on the auction block.\n\r" );
    return;
  }
    
  count = 0;
  for( int i = 0; i < auction_list; i++ ) 
    if( auction_list[i]->seller == ch->pcdata->pfile && ++count == 3 ) {
      send( ch,
        "You may only have 3 lots on the auction block at once.\n\r" );
      return;
    }

  if( *argument == '\0' ) {
    send( ch, "What do you want the minimum bid to be?\n\r" );
    return;
  }

  if( ( bid = atoi( argument ) ) < 1 ) {
    send( ch, "Minimum bid must be at least 1 cp.\n\r" );
    return;
  }

  if( bid > 100000 ) {
    send( ch, "The minimum bid cannot be greater than 100 pp.\n\r" );
    return;
  }

  if( ( array = several_things( ch, arg,
    "auction", &ch->contents ) ) == NULL )
    return;

  if( !can_auction( ch, (obj_array*) array ) ) {
    delete array;
    return;
    }


  /* FIND FIRST OPEN SLOT */

  slot = 1;
  for( int i = 0; ; ) {
    if( i == auction_list )
      break;
    if( auction_list[i]->slot == slot ) {
      slot++;
      i = 0;
    }
    else
      i++;
  }

  /* AUCTION ITEM */

  fsend( ch, "A auction daemon runs up to you.  You hand him %s and a\
 silver coin and he sprints off to the auction block.",
    list_name( ch, array, FALSE, buf ) );
  
  sprintf( tmp, "%s has placed %s on the auction block.",
    ch->real_name( ), list_name( NULL, array, TRUE, buf ) );
  info( tmp, LEVEL_APPRENTICE, tmp, IFLAG_AUCTION, 1, ch );

  auction          = new auction_data;
  auction->seller  = ch->pcdata->pfile;
  auction->buyer   = NULL;
  auction->bid     = bid;
  auction->time    = 50;
  auction->slot    = slot;

  for( int i = 0; i < *array; i++ ) {
    obj = (obj_data*) array->list[i];
    obj = (obj_data*) obj->From( obj->selected );
    set_owner( obj, NULL, ch );
    obj->To( &auction->contents );
  }
  delete array;

  auction_list += auction;
}


/*
 *   CAN AUCTION
 */


bool can_auction( char_data* ch, obj_array* array )
{
  obj_data* obj;
  int  distinct;
    
  for( int i = *array - 1; i >= 0; i-- )
    if( ( obj = object( array->list[i] ) ) != NULL )
      if( obj->pIndexData->item_type == ITEM_MONEY )
        *array -= obj;
  
  if( *array == 0 )
  {
    send( ch, "You can't auction money!\n\r" );
    return FALSE;
  }
  
  if( ( *array ) > 5 )
  {
    rehash( ch, *( (thing_array*) array ) );
    
    distinct = 0;
    for( int i = 0; i < *array; i++ )
      if( array->list[i]->shown > 0 )
        distinct++;

    if( distinct > 5 ) {
      fsend( ch, distinct_msg );
      return FALSE;
    }
  }

  if( !remove_silver( ch ) ) {
    send( ch, "To auction you need a silver coin to bribe the delivery daemon.\n\r" );
    return FALSE;
  }

  for( int i = 0; i < *array; i++ ) {
    obj = object( array->list[i] );
 
    if( no_auction( obj ) ) {
      fsend( ch, no_auction_msg, obj );
      return FALSE;
    }

    if( is_set( obj->extra_flags, OFLAG_NODROP ) ) {
      send( ch, no_drop_msg, obj );
      return FALSE;
    }
 
    if(  obj->pIndexData->item_type == ITEM_CORPSE ) {
      fsend( ch, corpse_msg, obj );
      obj->Extract( obj->selected );
      return FALSE;
    }
  }

  return !stolen_auction( ch, array );
}


bool no_auction( obj_data* obj )
{
  obj_data* content;

  if( is_set( obj->pIndexData->extra_flags, OFLAG_NO_AUCTION ) ) 
    return TRUE;

  for( int i = 0; i < obj->contents; i++ )
    if( ( content = object( obj->contents[i] ) ) == NULL 
      || no_auction( content ) )
      return TRUE;

  return FALSE;
}

const char* stolen_msg = "An auction daemon runs up to you. You hand\
 him %s and a silver coin.  He stops and looks at %s closely and then declares\
 %s stolen property and disappears with a mutter about returning %s to the\
 true owner.";

const char* owner_msg = "An auction daemon runs up to you and hands\
 you %s.  He bows deeply and then blinks out of existence.";


bool stolen_auction( char_data* ch, obj_array* array )
{
  player_data*      player;
  obj_data*            obj;
  obj_array*  stolen_array;
  obj_array*  return_array;
  pfile_data*   arry_owner;
  char                 buf  [ EIGHT_LINES ];
  
  
  stolen_array = new obj_array;
  
  for( int i = 0; i < *array; i++ )
    if( ( obj = object( array->list[i] ) ) != NULL )
      if( !obj->Belongs( ch ) )
      {
        obj = (obj_data*) obj->From( obj->selected );
        *stolen_array += obj;
      }
  
  if( stolen_array->size == 0 )
  {
    delete stolen_array;
    return FALSE;
  }
  
  fsend( ch, stolen_msg, list_name( ch, (thing_array*) array, FALSE, buf ),
    list_name( ch, (thing_array*) stolen_array, TRUE, buf ),
    stolen_array->size > 1 ? "them" : "it",
    stolen_array->size > 1 ? "them" : "it" );
  
  while( *stolen_array > 0 )
  {
    arry_owner = NULL;

    return_array = new obj_array;

    for( int i = *stolen_array - 1; i >= 0; i-- ) {
      obj = (*stolen_array)[i];

      if( arry_owner == NULL )
        arry_owner = obj->owner;
      else
      if( obj->owner != arry_owner )
        continue;
        
      *stolen_array -= obj;
      *return_array += obj;
    }
    
    if( ( player = find_player( arry_owner ) ) != NULL ) {
      fsend( player, owner_msg,
        list_name( ch, (thing_array*) return_array, TRUE, buf ) );
        
      for( int i = *return_array - 1; i >= 0; i-- ) {
        (*return_array)[i]->To( &player->contents );
        consolidate( (*return_array)[i] );
      }
    }
    else { 
      transfer_file( arry_owner, (thing_array*) return_array, 0 );
    }
    delete return_array;
  }
  
  delete stolen_array;
  
  return TRUE;
}


/*
 *   DISPLAY
 */


void display_auction( player_data* pc )
{
  char               tmp  [ FOUR_LINES ];
  char          obj_name  [ TWO_LINES ];
  char         condition  [ 50 ];
  char             buyer  [ 20 ];
  char               age  [ 20 ];
  auction_data*  auction;
  obj_data*          obj;
  bool             first;

  if( is_empty( auction_list ) ) {
    send( pc, "There are no items being auctioned.\n\r" );
    return;
    }

  page( pc, "Bank Account: %d cp\n\r\n\r", pc->bank );
  page_centered( pc, "+++ The Auction Block +++" );
  page( pc, "\n\r" );
  sprintf( tmp, "%4s %3s %-42s %4s %4s %s %s %7s\n\r",
    "Slot", "Tme", "Item", "Buyr", "Use?", "Age", "Cnd", "Min Bid" );
  page_underlined( pc, tmp );
 
  for( int i = 0; i < auction_list; i++ ) {
    auction = auction_list[i];
    rehash( pc, auction->contents );
    first = TRUE;
    for( int j = 0; j < auction->contents; j++ ) {
      obj = object( auction->contents[j] );
      if( obj->shown > 0 ) {
        condition_abbrev( condition, obj, pc );
        age_abbrev( age, obj, pc );
        
        strcpy( obj_name, obj->Seen_Name( NULL, obj->shown, TRUE ) );
        truncate( obj_name, 42 );
        
        if( first ) {
          first = FALSE;
          memcpy( buyer, auction->buyer == NULL ? " -- "
            : auction->buyer->name, 4 );
          buyer[4] = '\0';
          sprintf( tmp, "%-4d %-3d %-42s %4s %4s %s %s %7d\n\r",
            auction->slot, auction->time,
            obj_name, buyer,
            can_use( pc, obj->pIndexData, obj ) ? "yes" : "no",
            age, condition, auction->minimum_bid( ) );
          }
        else {
          sprintf( tmp, "         %-42s      %4s %s %s\n\r",
            obj_name, can_use( pc, obj->pIndexData, obj ) ? "yes" : "no",
            age, condition );
          }
        page( pc, tmp );
        }
      }
    }
}


const char* insurance_msg = "An auction daemon runs up to you and removes %d\
 cps from you to put in your bank account as insurance.  He skips away\
 happily, obviously keeping some for himself.";


/*
 *   BID
 */


void do_bid( char_data* ch, char* argument )
{
  char               arg  [ MAX_INPUT_LENGTH ];
  char               tmp  [ EIGHT_LINES ];
  char               buf  [ EIGHT_LINES ];
  player_data*        pc;
  auction_data*  auction  = NULL;
  content_array*   array;
  int                bid;
  int            min_bid;
  int               slot;
  int              proxy;
  int               debt;

  if( is_mob( ch ) )
    return;
/*
  if( ch->in_room == NULL
    || !is_set( &ch->in_room->room_flags, RFLAG_AUCTION_HOUSE ) )
  {
    send( ch, "There is no auction house here.\n\r" );    
    return;
  }
*/
  pc = player( ch );

  if( *argument == '\0' ) {
    display_auction( pc );
    page( pc, "\n\rUsage: Bid <slot> <price> [proxy]\n\r" );
    return;
  }

  argument = one_argument( argument, arg );
  slot     = atoi( arg );

  for( int i = 0; i < auction_list; i++ ) 
    if( ( auction = auction_list[i] )->slot == slot )
      break;
        
  if( auction == NULL ) {
    send( ch,
      "There is no lot with slot %d on the auction block.\n\r",
      slot );
    return;
    }

  if( auction->seller == ch->pcdata->pfile ) {
    send( ch, "You can't bid on your own item!\n\r" );
    return;
    }

  argument = one_argument( argument, arg );

  bid     = atoi( arg );
  proxy   = atoi( argument );
  min_bid = auction->minimum_bid( );

  bid = max( bid, min( min_bid, proxy ) );

  if( bid < min_bid ) {
    fsend( ch, "The minimum bid for %s is %d.\n\r",    
      list_name( ch, &auction->contents, TRUE, buf ), min_bid );
    return;
    }

  if( bid > 3*min_bid && bid > 500 ) {
    send( ch, "To protect you from yourself you may not bid more than the\
 greater\n\rof 3 times the current minimum bid and 500 cp.\n\r" );
    return;
    }

  if( *argument != '\0' && proxy <= bid ) {
    send( ch, "A proxy only makes sense if greater than the bid.\n\r" );
    return;
    }
  
  /* PUIOK 7/3/2000 - auto coins -> bank transfer */
  if( ( debt = max( bid, proxy ) - free_balance( pc, auction ) ) > 0 ) {
    int remove_amount = debt + max( 10, (int) ( debt * 0.02 ) );
    remove_amount = remove_amount + ( 10 - remove_amount % 10 );

    if( !remove_coins( ch, remove_amount, NULL ) ) {
      send( ch, "The bid is not accepted due to insufficent funds.\n\r" );
      return;
    }
    
    sprintf( buf, insurance_msg, remove_amount );
    fsend( ch, buf );
    send( ch, "\n\r" );
    
    pc->bank += debt;
  }

  array = &auction->contents;

  if( auction->buyer == ch->pcdata->pfile ) {
    bid = max( bid, proxy );
    if( auction->proxy > 0 ) 
      fsend( ch, "You change the proxy on %s from %d to %d cp.",
         list_name( NULL, array, TRUE, buf ), auction->proxy, bid );
    else
      fsend( ch, "You add a proxy on %s of %d cp.",
         list_name( NULL ,array, TRUE, buf ), bid );
    auction->proxy = bid;
    return;
    }

  bid = max( bid, min( auction->proxy, proxy ) );

  if( bid < auction->proxy  ) {
    sprintf( tmp,
      "You bid %d cp for %s, but it is immediately matched by %s.\n\r",
      bid, list_name( NULL, array, TRUE, buf ), auction->buyer->name );
    fsend( ch, tmp );
    sprintf( tmp, "%s bids %d cp on item #%d, %s.",
      ch->real_name( ), bid, auction->slot,
      list_name( NULL, array, TRUE, buf ) );
    info( tmp, LEVEL_APPRENTICE, tmp, IFLAG_AUCTION, 3, ch );
    sprintf( tmp, "The bid is matched by %s.", auction->buyer->name );
    info( tmp, LEVEL_APPRENTICE, tmp, IFLAG_AUCTION, 3, ch );
    auction->bid = bid;
    add_time( auction );
    return;
    } 

  if( proxy > bid ) 
    fsend( ch, "You bid %d cp for %s and will automatically match bids on it up to %d cp.\n\r", bid, list_name( NULL, array, TRUE, buf ) , proxy );
  else
    send( ch, "You bid %d cp for %s.\n\r",
      bid, list_name( NULL, array, TRUE, buf ) );

  sprintf( tmp, "%s bids %d cp on item #%d, %s.",
    ch->real_name( ), bid, auction->slot,
    list_name( NULL, array, TRUE, buf ) );
  info( tmp, LEVEL_APPRENTICE, tmp, IFLAG_AUCTION, 3, ch );

  auction->bid     = bid;
  auction->buyer   = ch->pcdata->pfile;
  auction->deleted = FALSE;
  auction->proxy   = proxy;

  add_time( auction );
}


const char* delivery_msg = "A daemon runs up and hands you %s.  He mumbles\
 something about %d cp and raiding your bank account and sprints off.";

const char* return_msg = "A daemon runs up and shoves %s to you. \
 He then marches off without a word.";

const char* floor_msg = "A daemon runs up to you and attempts to hand %s to you\
 but realizes you unable to carry %s.  He snickers rudely and drops the\
 delivery on the floor and sprints off.";
 

void auction_update( )
{
  auction_data*  auction;

  for( int i = 0; i < auction_list; i++ ) {
    auction = auction_list[i];
    if( auction == NULL)
      continue;
    
    auction->time -= 1;
    
    if( auction->time == 0 ) {
      auction_list -= auction;
      i--;
      
      if( auction->buyer == NULL ) {
        return_seller( auction );
      }
      else {
        transfer_buyer( auction );
      }
      delete auction;
    }
  }
}


int free_balance( player_data* player, auction_data* replace )
{
  int              credit  = player->bank;
  auction_data*   auction;

  for( int i = 0; i < auction_list; i++ ) {
    auction = auction_list[i];
    if( auction->buyer == player->pcdata->pfile && auction != replace ) 
      credit -= max( auction->bid, auction->proxy );
  }

  return credit;
}


void auction_message( char_data* ch )
{
  char   tmp   [ TWO_LINES ];
  int      i;

  if( ( i = auction_list.size ) != 0 ) {
    sprintf( tmp, "There %s %s item%s on the auction block.",
      i == 1 ? "is" : "are", number_word( i ),
      i == 1 ? "" : "s" ); 
    send_centered( ch, tmp );
  }
}


/*
 *   TRANSFERING OF OBJECT/MONEY
 */

void transfer_file( pfile_data* pfile, thing_array* array, int amount )
{
  link_data*       link;
  player_data*   player;
  obj_data*         obj;
  int           connect;

  for( link = link_list; link != NULL; link = link->next )
  {
    player = link->player;
 
    if( player != NULL && player->pcdata->pfile == pfile ) {
      for( int i = *array - 1; i >= 0; i-- )
        if( ( obj = object( array->list[i] ) ) != NULL ) {
          obj = (obj_data*) obj->From( obj->number );
          set_owner( obj, player, NULL );
          obj->To( &player->contents );
          consolidate( obj );
        }
      
      if( !add_bank( player, amount ) )
        return;
      
      connect         = link->connected;
      link->connected = CON_PLAYING;
      write( player );
      link->connected = connect;
      return;
    }
  }
 
  link = new link_data;

  if( !load_char( link, pfile->name, PLAYER_DIR ) ) {
    bug( "Transfer_File: Non-existent player file." ); 
    if( obj != NULL )
      obj->Extract( );
    return;
  }          

  player           = link->player;
  link->connected  = CON_PLAYING;
  
  if( !add_bank( player, amount ) )
    return;

  for( int i = *array - 1; i >= 0; i-- )
    if( ( obj = object( array->list[i] ) ) != NULL ) {
      obj = (obj_data*) obj->From( obj->number );
      set_owner( obj, player, NULL );
      obj->To( &player->locker );
      consolidate( obj );
    }

  write( player );
  player->Extract( );

  delete link;
}

void transfer_file( pfile_data* pfile, obj_data* obj, int amount )
{
  link_data*       link;
  player_data*   player;
  int           connect;

  for( link = link_list; link != NULL; link = link->next ) {
    player = link->player;
 
    if( player != NULL && player->pcdata->pfile == pfile ) {
      if( obj != NULL ) {
        set_owner( obj, player, NULL );
        obj->To( &player->contents );
        consolidate( obj );
      }
      
      player->bank   += amount;
      
      connect         = link->connected;
      link->connected = CON_PLAYING;
      write( player );
      link->connected = connect;
      return;
    }
  }
 
  link = new link_data;

  if( !load_char( link, pfile->name, PLAYER_DIR ) ) {
    bug( "Transfer_File: Non-existent player file." ); 
    if( obj != NULL )
      obj->Extract( );
    return;
  }          

  player           = link->player;
  link->connected  = CON_PLAYING;
  player->bank    += amount;

  if( obj != NULL ) {
    set_owner( obj, player, NULL );
    obj->To( &player->locker );
    consolidate( obj );
  }

  write( player );
  player->Extract( );

  delete link;
}

void mail_delivery( pfile_data* pfile, const char* title, const char* message )
{
  note_data* mail;
  link_data* link;
  char_data*   ch;
  int      clones;
  
  mail              = new note_data;
  mail->title       = alloc_string( title, MEM_NOTE );
  mail->message     = alloc_string( message, MEM_NOTE );
  mail->from        = alloc_string( "Auction Daemon", MEM_NOTE );
  mail->noteboard   = NOTE_PRIVATE;
  
  mail->date = current_time;
  
  append( pfile->mail, mail );
  save_mail( pfile );
    
  for( link = link_list, clones = 0; clones < player_list; ) {
    if( link == NULL ) {
      ch = player_list[ clones++ ];
      if( !ch->In_Game( ) || ch->link == NULL
        || !is_set( &ch->status, STAT_CLONED ) )
        continue;
    }
    else {
      if( link->connected != CON_PLAYING ) {
        link = link->next;
        continue;
      }
      ch = link->player;
      link = link->next;
    }
    if( ch->pcdata->pfile == pfile ) {
      send( ch,
        "A mail daemon runs up and hands you a letter from the auction daemon.\n\r" );
      break;
    }
  }
}

void return_seller( auction_data* auction )
{
  char             tmp  [ EIGHT_LINES ];
  char             buf  [ EIGHT_LINES ];
  player_data*      pc;
  thing_array*   array  = &auction->contents;
  obj_data*        obj;
  bool       to_locker;

  if( auction->seller == NULL ) {
    sprintf( tmp, "Lot #%d, %s returned to the estate of a deceased character.",
      auction->slot, list_name( NULL, array, TRUE, buf ) );
    info( tmp, LEVEL_APPRENTICE, tmp, 3, IFLAG_AUCTION );
    extract( auction->contents );
    return;
  }

  pc = find_player( auction->seller );

  sprintf( tmp, "Lot #%d, %s, returned to %s.", auction->slot,
    list_name( NULL, array, TRUE, buf ), auction->seller->name );
  info( tmp, LEVEL_APPRENTICE, tmp, IFLAG_AUCTION, 3, pc );
 
  /* added obj name to mail message - zemus */
  char tmpname[ MAX_STRING_LENGTH ];
  sprintf( tmpname, "%s", list_name( NULL, array, TRUE, buf ) );


  if( pc == NULL ) {
    transfer_file( auction->seller, array, 0 );
  }
  else {
    to_locker = pc->in_room == NULL
      || !is_set( &pc->in_room->room_flags, RFLAG_AUCTION_HOUSE )
      || pc->in_room->vnum == ROOM_LIMBO;
      
    if( !to_locker && pc->link != NULL )
        fsend( pc, return_msg, list_name( pc, array, TRUE, buf ) );
    
    for( int i = *array - 1; i >= 0; i-- )
      if( ( obj = object( array->list[i] ) ) != NULL ) {
        obj = (obj_data*) obj->From( obj->number );
        set_owner( obj, pc, NULL );
        obj->To( to_locker ? &pc->locker : &pc->contents );
        consolidate( obj );
      }
  }
  
  if( pc == NULL || to_locker ) {
    /* obj name - zemus */
    char tmpmsg[ MAX_STRING_LENGTH ];
    sprintf( tmpmsg, "Your auction(%s) did not receive any bids and has been\
 returned to you in your bank account.\n\r\n\rAuction Daemon\n\r", tmpname );
      mail_delivery( auction->seller, "Auction Returned", tmpmsg );
  }
}


void transfer_buyer( auction_data* auction ) 
{
  char              tmp  [ EIGHT_LINES ];
  char              buf  [ EIGHT_LINES ];
  player_data*   player;
  obj_data*         obj;
  thing_array*    array = &auction->contents;
  bool        to_locker;
  bool         to_floor;
  bool         singular;

  if( auction->seller != NULL ) {
    player = find_player( auction->seller );
    if( player != NULL ) 
      player->bank += 19*auction->bid/20;
    else
      transfer_file( auction->seller, (obj_data*) NULL, 19*auction->bid/20 );
  }

  if( auction->buyer == NULL ) {
    sprintf( tmp, "Item #%d, %s, sold to the estate of a deceased character.",
      auction->slot, list_name( NULL, array, TRUE, buf ) );
    info( tmp, LEVEL_APPRENTICE, tmp, 2, IFLAG_AUCTION );
    extract( auction->contents );
    return;
  }

  player = find_player( auction->buyer );

  sprintf( tmp, "Item #%d, %s, sold to %s for %d cp.", auction->slot,
    list_name( NULL, array, TRUE, buf ), auction->buyer->name, auction->bid );
  info( tmp, LEVEL_APPRENTICE, tmp, IFLAG_AUCTION, 2, player );
  

  /* line added by Zemus for obj specific auction mail */
  char tmpname[ MAX_STRING_LENGTH ];
  sprintf( tmpname, "%s", list_name( NULL, array, TRUE, buf ) );


  singular = !( *array > 1 || ( *array > 0 && array->list[0]->number > 1 ) );
  
  if( player == NULL ) {
    transfer_file( auction->buyer, array, -auction->bid );
  }
  else
  {
    player->bank -= auction->bid;
    
    to_locker = player->in_room == NULL
      || !is_set( &player->in_room->room_flags, RFLAG_AUCTION_HOUSE )
      || player->in_room->vnum == ROOM_LIMBO;
    
    if( !to_locker ) {
/*      to_floor = !can_carry( player, *array, FALSE );*/
      to_floor = FALSE;
    
      if( player->link != NULL )
        if( to_floor ) {
          fsend( player, floor_msg, list_name( NULL, array, TRUE, buf ), singular ? "it" : "them" );
        }
        else
          fsend( player, delivery_msg, list_name( NULL, array, TRUE, buf ), auction->bid );
    }  
    
    for( int i = *array - 1; i >= 0; i--)
      if( ( obj = object( array->list[i] ) ) != NULL ) {
        obj = (obj_data*) obj->From( obj->number );
        set_owner( obj, player, NULL );
        obj->To( to_locker ? &player->locker : to_floor
          ? &player->in_room->contents : &player->contents );
        consolidate( obj );
      }
  }
  

/* tmpname added below for obj name in mail - zemus */
char tmpmsg[ MAX_STRING_LENGTH ];
  if( player == NULL || to_locker )
  {
    if( !singular ) {
      sprintf( tmpmsg, "The items you have purchased(%s) have been placed\
 in your bank account.\n\r\n\rAuction Daemon\n\r", tmpname );
      mail_delivery( auction->buyer, "Auction Delivered", tmpmsg );
      }
    else {
      sprintf( tmpmsg, "The item you have purchased(%s) has been placed in\
 your bank account.\n\r\n\rAuction Daemon\n\r", tmpname );
      mail_delivery( auction->buyer, "Auction Delivered", tmpmsg );
      }
  }
}


void clear_auction( pfile_data* pfile )
{
  auction_data* auction;

  for( int i = 0; i < auction_list; i++) {
    if( (auction = auction_list[i]) == NULL )
    continue;
    
    if( auction->seller == pfile )
      auction->seller = NULL;
    if( auction->buyer == pfile ) {
      auction->buyer   = NULL;
      auction->deleted = TRUE;
    }
  }
  return;
}
