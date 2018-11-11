/*----------------------------------------------------------------------*/
/*                                                                      */
/*      Source file:            code1.C                                 */
/*      Package:                basic                                   */
/*      Software version:       1.0                                     */
/*      Date:                   18/01/2017                               */
/*      Authors:                A. Elouardi	*/
/*		      based on	Jens-Ulf Pehrs                          		*/
/*            Philips Semiconductors RHW-PCALH/AI			    		*/
/*                                                                      */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*      INCLUDES      D E F I N I T I O N S                             */
/************************************************************************/
#include <reg592.h>
#include <stdio.h>
#include <intrins.h>
#include "candefs.h"
#include "FonctionsBases2.h"
#include "libtp2.h"




/************************************************************************/
/************************************************************************/
/*							FONCTIONS									*/
/************************************************************************/
/************************************************************************/

/************************************************************************/
/*......................................................................*/
/*		Fonctions pour la configuration et l'initialisation de CAN		*/
/*......................................................................*/
/************************************************************************/

/************************************************************************/
/*      procedure:      can_config                                      */
/*      input:          ---                                             */
/*      output:         ---                                             */
/*      calls:          ---                                             */
/************************************************************************/

//La reference de tension n'est pas active ... à voir !
unsigned char    can_config ()
{   unsigned char retour=0;
    EA = FALSE;
    can_com=0;
    do    {
        CANADR = CONTROL;
        CANDAT = RESET_REQUEST;
    }   while  ( !(CANDAT & RESET_REQUEST) );
    CANADR = AUTO_ACC_CODE;
    CANDAT = ACCEPT_CODE;
    CANDAT = ACCEPT_MASK;
    CANDAT = BUS_TIM_0;
    CANDAT = BUS_TIM_1;
    CANDAT = OUT_CONTROL;
    CANCON = DIFFERENTIAL;
    do    {
        CANADR = CONTROL;
        CANDAT = 0;
    }   while  ( CANDAT & RESET_REQUEST );
    CANDAT = CAN_INT_ENABLE;

    if  ( CAN_ERROR_STAT )    
	        retour=1; //printf("erreur_stat\n");
    PS1 = TRUE;
    ES1 = TRUE;
    EA  = TRUE;
	return retour;
}

/************************************************************************/
/*      procedure:      can_contr_init                                  */
/*      input:          ---                                             */
/*      output:         ---                                             */
/*      calls:          led_init, can_config, slio_init                 */
/************************************************************************/

void can_contr_init()
{
    can_config ();
}


/************************************************************************/
/*      procedure:      CANInitialisation                               */
/*      input:          ---                                             */
/*      output:         ---                                             */
/*      calls:          can_contr_init					                */
/************************************************************************/
 
void CANInitialisation()
{
	can_contr_init();
    can_init_request=FALSE;
	int_save = CANCON;
	can_tx_ready=1;
}





/************************************************************************/
/*......................................................................*/
/*			Fonction en cas de problème sur le CAN						*/
/*......................................................................*/
/************************************************************************/



/************************************************************************/
/*      procedure:      can_int_error                                   */
/*      input:          ---                                             */
/*      output:         ---                                             */
/*      calls:          ---                                             */
/*----------------------------------------------------------------------*/
/*      running on interrupt level, called from "can_int ()"            */
/************************************************************************/

void    can_int_error ()
{
    if  ( CAN_ERROR_STAT )  {
        can_event       = TRUE;
        if  ( CAN_BUS_STATUS )
            can_system_error = TRUE;
    }
    else  {
        can_event   = FALSE;
    }
}

/************************************************************************/
/*......................................................................*/
/*			Fonctions pour la réception de messages sur le CAN			*/
/*......................................................................*/
/************************************************************************/

/************************************************************************/
/*      procedure:      can_buf_full                                    */
/*      input:          ---                                             */
/*      output:         ---                                             */
/*      calls:          ---                                             */
/*----------------------------------------------------------------------*/
/*      running on interrupt level, called from "can_int ()"            */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*                                                                      */
/************************************************************************/

void    can_buf_full ()
{
    
//    CANSTA = (char *)tab_rx;
    CANSTA = (unsigned char)(can_buffrx);
	CANADR = DMA_EN|AUTO_RX_BUFFER;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
    if  ( overrun_int )
        CANCON = RELEASE_RX_BUF | OVERRUN_CLEAR;
    else
        CANCON = RELEASE_RX_BUF;

	can_event   = TRUE;
    can_rx_ready =TRUE;
}

/************************************************************************/
/*......................................................................*/
/*			Fonctions pour l'émission de messages sur le CAN			*/
/*......................................................................*/
/************************************************************************/

/************************************************************************/
/*      procedure:      send_message                                    */
/*      input:          data_table         				*/
/*      output:         ---                                             */
/*      calls:                                                          */
/************************************************************************/

unsigned char send_message ( unsigned char *tabtx ){

  int_save    = CANCON;
  if  ( CAN_TX_BUFFER )   {
    CANSTA = (char)tabtx;
    CANADR = DMA_EN|AUTO_TX_BUFFER;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
    can_event = TRUE;
    can_tx_ready = FALSE;
    can_tx_sending = TRUE; 
    CANCON = TRANSMIT_REQUEST|DIFFERENTIAL;        		/* start CAN transmit to SLIO */
	return 1;
  }
  else   {                                    			/* error exit, if transmit    */

  //ATTENTION ne pas faire ca car cela risque de planter
  // il faut rien faire dans ce cas et prévenir qu'il n'y  pas 
  //d'�mission...... et emettre à nouveau après.
    CANCON           = TRANSMIT_ABORT|DIFFERENTIAL;     /*    not possible            */
    can_system_error = TRUE;
    can_init_request = TRUE;
    can_partner      = FALSE;
    can_tx_ready=can_tx_sending=FALSE;
	return 0;
    };
}

/************************************************************************/
/*      procedure:      can_tx_complete                                 */
/*      input:          ---                                             */
/*      output:         ---                                             */
/*      calls:          ---                                             */
/*----------------------------------------------------------------------*/
/*      running on interrupt level, called from "can_int ()"            */
/************************************************************************/

void    can_tx_complete ()
{
    can_event = TRUE;
	can_tx_ready = TRUE;
}


/************************************************************************/
/*......................................................................*/
/*			Fonction gérant les interruptions sur le CAN				*/
/*......................................................................*/
/************************************************************************/
/*      procedure:      can_int                                         */
/*      input:          ---                                             */
/*      output:         ---                                             */
/*      calls:          ---                                             */
/*----------------------------------------------------------------------*/
/*      running on interrupt level, called from "can_int ()"            */
/************************************************************************/

void    can_int (void)  interrupt 5
{
    int_save = CANCON;
 
    if  ( error_int ){			// due � une erreur
		can_int_error ();
	}
    if  ( receive_int )    							// due � la r�ception d'un message
	{
		// on stocke la valeur du tps lors de la r�ception du message NON
		    CANSTA = (unsigned char)(can_buffrx);
			CANADR = DMA_EN|AUTO_RX_BUFFER;
			_nop_();
			_nop_();
			_nop_();
			_nop_();
    		if  ( overrun_int )
        		CANCON = RELEASE_RX_BUF | OVERRUN_CLEAR;
    		else
        		CANCON = RELEASE_RX_BUF;
			can_event   = TRUE;
    		can_rx_ready =TRUE;

	}
    if  ( transmit_int && CAN_TX_COMPLETE ){// due � une transm- correcte et buffer d'emission vide
  		can_tx_complete ();
	}
}


/************************************************************************/
/*      procedure:      timer setup and ISR                             */
/*      input:          ---                                             */
/*      output:         ---                                             */
/*      calls:          ---                                             */
/************************************************************************/

int tempo_counter = 0;     // counter for soft delay
int FLAG_tempo = 0;        // flag to signal pattern change

/**
 * \fn void init_timer0()
 * \brief Function to setup Timer 0
 */
void init_timer0() {
	TMOD = TMOD|2; // mode 2 timer 0
	TL0  = 0; // init TL0
	TH0  = 0; // tampon pour 192 us
	ET0  = 1; // validation de l'interrupt timer 0
	EA   = 1; // autorisation globale des interupts
	TR0  = 1; // start timer 0
}

/**
 * \fn void init_timer0()
 * \brief ISR of Timer 0
 */
void timer0() interrupt 1 {
	// Counting 250 ms with the timer
	tempo_counter++;
	if (tempo_counter >=4166) {
		tempo_counter = 0;
		FLAG_tempo = 1;
	}
}

/************************************************************************************************************************************************/

/************************************************************************/
/*      procedure:      main                                            */
/*      input:          ---                                             */
/*      output:         ---                                             */
/*      calls:          CANInitialisation, LireBufferReception          */
/*                      CANEmissionMessage, CANREceptionMessage         */
/************************************************************************/

unsigned int can_id = 0x490;

unsigned short address; // address sent on CAN message
unsigned char dlc; // dlc from CAN message
unsigned int counter; // data counter, used to get all data from message according to dlc
unsigned char can_data[8]; // data from CAN message
unsigned int led_pattern; // pattern to be shown
xdata unsigned int LED _at_ 0xFD00;   // access to LED hardware
unsigned char tx_data[] = {'C','E','S','T',' ','D','U','R',' ','C','H','O','I','S','I','R',' ','U','N',' ','M','E','S','S','A','G','E'}; // data to be sent via CAN bus
unsigned char tx_message[10]; // CAN message to be sent. Contains only part of tx_data

unsigned int char_counter; // counter used on send_data to iterate through tx_data
unsigned int frame_data_length; // converted in send_data to the dlc


/************************************************************************/
/*......................................................................*/
/*			Function that sends tx_data on multiple messages            */
/*......................................................................*/
/************************************************************************/
/*      procedure:      send_data                                       */
/*      input:          ---                                             */
/*      output:         ---                                             */
/*      calls:          send_message                                    */
/*----------------------------------------------------------------------*/
/*      called from main loop                                           */
/************************************************************************/
void send_data() {
	FLAG_tempo = 0; // reset timer flag
	
	frame_data_length = 0; // reset
	for (char_counter=0; char_counter < sizeof(tx_data); char_counter++) { // iterate through tx_data
		tx_message[2 + char_counter%8] = tx_data[char_counter]; // write bytes to message buffer
		frame_data_length++; // increment dlc counter 
		
		
		if ((char_counter+1)%8 == 0 || char_counter == sizeof(tx_data)-1) { // send message once max length has been achieved (8 data bytes) or there's no more data
			tx_message[0] = can_id >> 3;             // set can_id msb
			tx_message[1] = (can_id & 0x07) << 5;    // set can_id lsb
			tx_message[1] |= (char_counter/8) << 5;  // set sub id of message, used when data to be sent is too bg for a single message
			tx_message[1] |= frame_data_length;      // set dlc
			send_message(tx_message);                // send frame
			frame_data_length = 0;                   // reset counter
		}
	} 
}


/***************************   MAIN      *********************************/
void main() {
	//Initialisation du CAN
	CANInitialisation();
	// initialise timer 0, used to send CAN messages each 800 ms
	init_timer0();

	printf("initialisation du systeme\n");
	
	while(1) {   
		/**** erreur donc demande d'initialisation *****/    
		if  ( can_system_error ){
			printf("CAN ERREUR\n");
			can_init_request = TRUE;
		}
		/**** besoin d'une initialisation *****/
		if  ( can_init_request ){
			printf("initialisation CAN\n");
			CANInitialisation();
			can_init_request=FALSE;
		}
		
		if ( can_rx_ready ) {
			can_rx_ready=0;

			// read address from incoming message
			address = (((int)can_buffrx[0]) << 3) + ((can_buffrx[1] & 0xE0) >> 5);
			
			// discard messages from address 0x0370
			if (address != 0x0370) {
				// msg decode formating
				if (address == 0x0310) {
					printf("\n");
				}
				
				// read dlc
				dlc = (can_buffrx[1] & 0xF);
				
				// read data (length: dlc)
				for (counter=0; counter<dlc; counter++) {
					can_data[counter] = can_buffrx[2+counter];
				}
				
				// clear the rest of the buffer can_data
				for (counter=dlc; counter<8; counter++) {
					can_data[counter] = 0;
				}
				
				// set the led patter according to received data
				led_pattern = ((unsigned int)can_data[0] << 8) + (unsigned int)can_data[1];
				LED = led_pattern;
				
				// printf("address: 0x%X    dlc: 0x%X   led: 0x%X  data: ", address, (unsigned int)dlc, led_pattern);
				
				for (counter=0; counter<dlc; counter++) {
					// print received message 
					printf("%c", can_data[counter]);
				}
				
				printf(" ");
			}
		
		}
		
		// When 800 ms have passed the flag FLAG_tempo changes state and a new data trasmission is made
		if(can_tx_ready && FLAG_tempo){
			send_data();  	   		        
		}
	}
}

