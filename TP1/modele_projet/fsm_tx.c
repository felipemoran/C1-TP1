#include "fsm_tx.h"

/*****************************************/
/* defines                               */
/*****************************************/

#define TX_STATE_S0 0
#define TX_STATE_S1 1
#define TX_STATE_S2 2
#define TX_STATE_S3 3
#define TX_STATE_S4 4


/*****************************************/
/* typedefs                              */
/*****************************************/

typedef struct {
	int  fsm_state;      // current fsm state
	int  nb_bit;         // counter of received bits
	int  logic_counter;  // counter of interruption cycles
	char transmit;       // flag used to start a transmission
	char data_next;      // next char to be transmitted
	char data_sending;   // current char being transmitted
} fsm_tx_vars_t;
	

/*****************************************/
/* variables                             */
/*****************************************/

fsm_tx_vars_t fsm_tx_vars;  // instance of above struce
bit fsm_tx_bit;             // next bit to be transmitted

/*****************************************/
/* prototypes                            */
/*****************************************/

void fsm_tx_execute_state(void);
void fsm_tx_change_state(void);

/*****************************************/
/* public                                */
/*****************************************/

/**
 * \fn void fsm_tx_init()
 * \brief Init the fsm for transmission. Sets all internal variables to 0
 */
void fsm_tx_init() {
	fsm_tx_vars.fsm_state = TX_STATE_S0;
	fsm_tx_vars.nb_bit = 0;
	fsm_tx_vars.logic_counter = 0;
	fsm_tx_vars.transmit = 0;
	fsm_tx_vars.data_next = 0;
	fsm_tx_vars.data_sending = 0;
}

/**
 * \fn svoid fsm_tx_update()
 * \brief Public function to update the state machine. Internally it calls the respective functions for executing state action and changing to the next one if necessary
 */
void fsm_tx_update() {
	fsm_tx_execute_state();
	fsm_tx_change_state();
}

/**
 * \fn void fsm_tx_set_tx_data(char tx_data_input)
 * \brief Function that sets the next byte to be sent
 *
 * \param tx_data_input Char to be sent on next transmit cycle. If this function is called multiple times before the tranmission begins only the last value will be transmitted
 */
void fsm_tx_set_tx_data(char tx_data_input) {
	fsm_tx_vars.data_next = tx_data_input;
}

/**
 * \fn void fsm_tx_start_transmit()
 * \brief Function to signal start of transmission
 */
void fsm_tx_start_transmit() {
	fsm_tx_vars.transmit = (char) 1;
}

/**
 * \fn void fsm_tx_get_tx_bit()
 * \brief Function that gets the value of the tx pin of the fsm
 *
 * \return bit fsm internal value of the tx pin
 */
bit fsm_tx_get_tx_bit() {
	return fsm_tx_bit;
}

/*****************************************/
/* private                               */
/*****************************************/

/**
 * \fn void fsm_tx_execute_state()
 * \brief Function that executes the action corresponding to the current state
 */
void fsm_tx_execute_state() {
	switch(fsm_tx_vars.fsm_state) {
		case TX_STATE_S0:
			fsm_tx_bit = 1;
		break;

		case TX_STATE_S1:
			fsm_tx_bit = 0;
			fsm_tx_vars.nb_bit = 0;
			fsm_tx_vars.logic_counter = 0;
		break;

		case TX_STATE_S2:
			fsm_tx_vars.logic_counter++;
		break;

		case TX_STATE_S3:
			fsm_tx_vars.logic_counter = 0;
			if (fsm_tx_vars.nb_bit < 8) {
					fsm_tx_bit = fsm_tx_vars.data_sending & 0x1;
					fsm_tx_vars.data_sending = fsm_tx_vars.data_sending >> 1;
			} else {
				fsm_tx_bit = 1;
			}
			fsm_tx_vars.nb_bit++;
		break;

		case TX_STATE_S4:
			fsm_tx_vars.transmit = 0;
		break;
	}
}

/**
 * \fn void fsm_rx_change_state()
 * \brief Function that changes to another state if necessary conditions are met
 */
void fsm_tx_change_state() {
	switch(fsm_tx_vars.fsm_state) {
		case TX_STATE_S0:
			if (fsm_tx_vars.transmit == (char) 1) {
				fsm_tx_vars.fsm_state = TX_STATE_S1;
				fsm_tx_vars.data_sending = fsm_tx_vars.data_next;
			}
		break;
		case TX_STATE_S1:
			fsm_tx_vars.fsm_state = TX_STATE_S2;
		break;
		case TX_STATE_S2:
			if (fsm_tx_vars.logic_counter < 12) {
				break;
			}

			if (fsm_tx_vars.nb_bit < 9) {
				fsm_tx_vars.fsm_state = TX_STATE_S3;
			} else {
				fsm_tx_vars.fsm_state = TX_STATE_S4;
			}
		break;
		case TX_STATE_S3:
			fsm_tx_vars.fsm_state = TX_STATE_S2;
		break;
		case TX_STATE_S4:
			fsm_tx_vars.fsm_state = TX_STATE_S0;
		break;
	}
}
