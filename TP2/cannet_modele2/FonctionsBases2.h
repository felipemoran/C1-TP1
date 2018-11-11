//                  _,;;.,_
//         ,-;;,_  ;,',,,'''@
//      ,;;``  `'\\|//``-:;,.
//    @`         ;^^^;      `'@
//               :@ @:
//               \ u /
//      ,=,------)^^^(------,=,
//      '-'-----/=====\-----'-'
//              \_____/
//          '`\ /_____\
//          `\ \\_____/_
//            \//_____\/|
//            |        ||
//            |CAN-NET |'
//            :________:`
//
//	Beware the canner !!


/*----------------------------------------------------------------------*/
/*      bit variables and arrays of bit variables                       */
/*----------------------------------------------------------------------*/

bdata unsigned char    int_save ; 		/* interrupt bits in CAN contr. */
sbit    receive_int     = int_save^0;
sbit    transmit_int    = int_save^1;
sbit    error_int       = int_save^2;
sbit    overrun_int     = int_save^3;

bit     can_init_request;               /* TRUE: new CAN init necessary     */
bit     can_system_error;               /* TRUE: CAN-bus failure detected   */
bit     time_over;                      /* TRUE: started time interval over */


bdata unsigned char can_com ;
sbit     can_partner= can_com^0;		/* TRUE: another CAN partner on bus */
sbit     can_event=can_com^1;			/* TRUE: CAN interrupt arrived      */
sbit     can_rx_ready=can_com^2;
sbit 	 can_tx_ready=can_com^3;
sbit 	 can_tx_sending=can_com^4;


/*----------------------------------------------------------------------*/
/*      byte variables and arrays of byte variables                     */
/*----------------------------------------------------------------------*/

//idata unsigned char tab_rx[10],tab_canrx[10];
#define BUFFER_RX_SIZE   20
unsigned char can_buffrx[BUFFER_RX_SIZE];
