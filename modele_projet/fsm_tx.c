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
	int fsm_state;
	int nb_bit;
	int logic_counter;
	char transmit;
	char data_next;
	char data_sending;
} fsm_tx_vars_t;
	

/*****************************************/
/* variables                             */
/*****************************************/

fsm_tx_vars_t fsm_tx_vars;
bit fsm_tx_bit;

/*****************************************/
/* prototypes                            */
/*****************************************/

void fsm_tx_execute_state(void);
void fsm_tx_change_state(void);

/*****************************************/
/* public                                */
/*****************************************/

void fsm_tx_init() {
	fsm_tx_vars.fsm_state = TX_STATE_S0;
	fsm_tx_vars.nb_bit = 0;
	fsm_tx_vars.logic_counter = 0;
	fsm_tx_vars.transmit = 0;
	fsm_tx_vars.data_next = 0;
	fsm_tx_vars.data_sending = 0;
}

void fsm_tx_update() {
	fsm_tx_execute_state();
	fsm_tx_change_state();
}

void fsm_tx_set_tx_data(char tx_data_input) {
	fsm_tx_vars.data_next = tx_data_input;
}

void fsm_tx_start_transmit() {
	fsm_tx_vars.transmit = (char) 1;
}

bit fsm_tx_get_tx_bit() {
	return fsm_tx_bit;
}

/*****************************************/
/* private                               */
/*****************************************/

void fsm_tx_execute_state(void) {
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


void fsm_tx_change_state(void) {
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
