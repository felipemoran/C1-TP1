/************************************************************************
 **	Programme exemple pour le TP æcontroleur		       **
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

code int table[11] = { 0x000, 0x001, 0x003, 0x007, 0x00F, 0x01F, 0x03F, 0x07F, 0x0FF, 0x1FF, 0x3FF};
int active_led = 0;
int sense_fwd = 1;
int increment;
int tempo_counter = 0;
int FLAG_tempo = 0;
int test_var = 0;
xdata unsigned int LEDS _at_ 0xFD00;
sbit pin_tx = 0x97;
sbit pin_rx = 0x96;
bit tx_bit, rx_bit;

/*******************************************/
/* declaration des procedures et fonctions */
/*******************************************/

/* Function to create a small delay */
int tempo() {
	tempo_counter++;
	if (tempo_counter==3000) {
		tempo_counter = 0;
		return 1;
	}

	return 0;
}

/* Function to show on the leds a moving pattern */
/* received a key pressed and if it's different than 0xFF, which means no key, the direction changes */
void show_pattern(char clav) {
	if (clav != (char)0xFF) {
		sense_fwd = !sense_fwd;
	}

	if (sense_fwd) {
		increment = 1;
	} else {
		increment = -1;
	}

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

/* Setup Timer 0 */
void init_timer0() {
	TMOD = TMOD|2; // mode 2 timer 0
	TL0  = 0; // init TL0
	TH0  = 0; // tampon pour 192 us
	ET0  = 1; // validation de l'interrupt timer 0
	EA   = 1; // autorisation globale des interupts
	TR0  = 1; // start timer 0
}

/* IRS Timer 0 */
void timer0() interrupt 1 {
	/* tempo_counter++;
	if (tempo_counter >=1302) {
		tempo_counter = 0;
		FLAG_tempo = 1;
	} */
	tx_bit = fsm_tx_get_tx_bit();
	pin_tx = tx_bit;

	rx_bit = pin_rx;
	fsm_rx_set_rx_bit(rx_bit);

	fsm_tx_update();
	fsm_rx_update();
}


/***************************************/
/* main program                        */
/***************************************/
char clav;
char clav_in = 0;
void main (void)  {  /* debut du programme apres le startup  */
	init_timer0();

  	while (1) {
		// clav_out = detect_touche();
		// clav_out = decode_touche(clav_out);

		clav = 5;
		fsm_tx_set_tx_data(clav);
		fsm_tx_start_transmit();

		clav_in = fsm_rx_get_rx_data();

		if (clav_in == (char) 0x05) {
			test_var++;
		}
		// show_pattern(clav);
	}
} 


