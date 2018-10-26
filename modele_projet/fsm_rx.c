#include "fsm_rx.h"

/*****************************************/
/* defines                               */
/*****************************************/

#define RX_STATE_S0 0
#define RX_STATE_S1 1
#define RX_STATE_S2 2
#define RX_STATE_S3 3
#define RX_STATE_S4 4
#define RX_STATE_S5 5


/*****************************************/
/* typedefs                              */
/*****************************************/

typedef struct {
	int  fsm_state;
	int  nb_bit;
	int  logic_counter;
	char data_is_ready;
	char data_ready;
	char data_partial;
} fsm_rx_vars_t;

/*****************************************/
/* variables                             */
/*****************************************/

fsm_rx_vars_t fsm_rx_vars;
bit  fsm_rx_bit;

/*****************************************/
/* prototypes                            */
/*****************************************/

void fsm_rx_execute_state(void);
void fsm_rx_change_state(void);

/*****************************************/
/* public                                */
/*****************************************/

void fsm_rx_init() {
	fsm_rx_vars.fsm_state = RX_STATE_S0;
	fsm_rx_vars.nb_bit = 0;
	fsm_rx_vars.logic_counter = 0;
	fsm_rx_vars.data_is_ready = 0;
	fsm_rx_vars.data_ready = 0;
	fsm_rx_vars.data_partial = 0;
}

void fsm_rx_update() {
	fsm_rx_execute_state();
	fsm_rx_change_state();
}

char fsm_rx_get_rx_data() {
	if (fsm_rx_vars.data_is_ready == (char) 1) {
		fsm_rx_vars.data_is_ready = 0;
		return fsm_rx_vars.data_ready;
	}

	return 0xFF;
}

void fsm_rx_set_rx_bit(bit rx_bit) {
	fsm_rx_bit = rx_bit;
}

/*****************************************/
/* private                               */
/*****************************************/
char moved_bit;
void fsm_rx_execute_state(void) {
	switch(fsm_rx_vars.fsm_state) {
		case RX_STATE_S0:
			// do nothing
		break;

		case RX_STATE_S1:
			fsm_rx_vars.logic_counter++;
		break;

		case RX_STATE_S2:
			fsm_rx_vars.logic_counter = 0;
			fsm_rx_vars.nb_bit = 0;
		break;

		case RX_STATE_S3:
			fsm_rx_vars.logic_counter++;
		break;

		case RX_STATE_S4:
			fsm_rx_vars.nb_bit++;
			fsm_rx_vars.logic_counter = 0;
			fsm_rx_vars.data_partial = (fsm_rx_vars.data_partial >> 1) & 0x7F;
			moved_bit = (((char) fsm_rx_bit) << 7);
			fsm_rx_vars.data_partial = fsm_rx_vars.data_partial | (((char) fsm_rx_bit) << 7);
		break;

		case RX_STATE_S5:
			fsm_rx_vars.data_ready = fsm_rx_vars.data_partial;
			fsm_rx_vars.data_is_ready = (char) 1;
		break;
	}
}


void fsm_rx_change_state(void) {
	switch(fsm_rx_vars.fsm_state) {
		case RX_STATE_S0:
			if (fsm_rx_bit == (bit) 0) {
				fsm_rx_vars.fsm_state = RX_STATE_S1;
			}
		break;
		case RX_STATE_S1:
			if (fsm_rx_vars.logic_counter == 5) {
				fsm_rx_vars.fsm_state = RX_STATE_S2;
			}
		break;
		case RX_STATE_S2:
			fsm_rx_vars.fsm_state = RX_STATE_S3;
		break;
		case RX_STATE_S3:
			if (fsm_rx_vars.logic_counter < 12) {
				break;
			}

			if (fsm_rx_vars.nb_bit < 8) {
				fsm_rx_vars.fsm_state = RX_STATE_S4;
			} else {
				fsm_rx_vars.fsm_state = RX_STATE_S5;
			}
		break;
		case RX_STATE_S4:
			fsm_rx_vars.fsm_state = RX_STATE_S3;
		break;
		case RX_STATE_S5:
			fsm_rx_vars.fsm_state = RX_STATE_S0;
		break;
	}
}
