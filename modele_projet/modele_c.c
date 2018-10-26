/************************************************************************
 **	Programme exemple pour le TP �controleur		       **
 **                                                                    **
 ************************************************************************/
 
#include <reg592.h>                /* registres specifiques au 592 */
#include <stdio.h>  
#include "libtp2.h" 
#include "fsm_tx.h"
#include "fsm_rx.h"


/*****************************************/
/* declaration des variables et tableaux */
/*****************************************/

// LED DISPLAY
code int table[11] = { 0x000, 0x001, 0x003, 0x007, 0x00F, 0x01F, 0x03F, 0x07F, 0x0FF, 0x1FF, 0x3FF}; // table of patterns 
int active_led = 0;        // current selected pattern
int sense_fwd = 1;         // sense of movement of led patterns
int increment;             // next cycle increment direction
int tempo_counter = 0;     // counter for soft delay
int FLAG_tempo = 0;        // flag to signal pattern change
int test_var = 0;          // debug only
xdata unsigned int LEDS _at_ 0xFD00;   // access to hardware

// Data tranmission
sbit pin_tx = 0x97;
sbit pin_rx = 0x96;
bit tx_bit, rx_bit;

/*******************************************/
/* declaration des procedures et fonctions */
/*******************************************/

/**
 * \fn int tempo()
 * \brief Function to create a small delay
 * 
 * \return 0 if the delay has not elased yet, 1 if it has already
 */
void tempo() {
	tempo_counter++;
	if (tempo_counter==3000) {
		tempo_counter = 0;
		return 1;
	}

	return 0;
}

/**
 * \fn void show_pattern(char clav)
 * \brief unction to show on the leds a moving pattern
 * 
 * \param clav char indicating the key pressed. If it's different than 0xFF, which means no key, the direction changes
 */
void show_pattern(char clav) {
	// If clav different from 0xFF, change sense
	if (clav != (char)0xFF) {
		sense_fwd = !sense_fwd;
	}

	// Change patter increment according to current sense
	if (sense_fwd) {
		increment = 1;
	} else {
		increment = -1;
	}

	// If the delay has elapsed, change the pattern and show it 
	if (FLAG_tempo) {
		FLAG_tempo = 0;
		active_led = active_led + increment;
		if (active_led < 0) {
			active_led = 10;
		}
		active_led = active_led % 11;

		LEDS = table[active_led];
	}
}

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
	if (tempo_counter >=1302) {
		tempo_counter = 0;
		FLAG_tempo = 1;
	}

	// Getting TX bit from tx fsm and writing to physical pin
	tx_bit = fsm_tx_get_tx_bit();
	pin_tx = tx_bit;

	// Reading physical RX pin and writing it to the rx fsm
	rx_bit = pin_rx;
	fsm_rx_set_rx_bit(rx_bit);

	// Update both fsm
	fsm_tx_update();
	fsm_rx_update();
}


/***************************************/
/* main program                        */
/***************************************/
char clav;
char clav_in = 0;
void main (void)  {  /* debut du programme apres le startup  */
	// setup timer 0 and make it work
	init_timer0();

  	while (1) {
		// detect pressed key
		clav_out = detect_touche();
		clav_out = decode_touche(clav_out);

		// set next value to transmit the one we just read
		fsm_tx_set_tx_data(clav);
		fsm_tx_start_transmit();

		// read received data
		clav_in = fsm_rx_get_rx_data();

		// if received data is difference than 0xFF, stop execution (debug)
		if (clav_in != (char) 0xFF) {
			test_var++;
		}

		// show led pattern in a non blocking way  
		show_pattern(clav);
	}
} 


