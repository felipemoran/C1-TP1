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
	int  fsm_state;      // current fsm state
	int  nb_bit;         // counter of received bits
	int  logic_counter;  // counter of interruption cycles
	char data_is_ready;  // flag for signaling data ready to be read
	char data_ready;     // received char. Populated only after the data is ready
	char data_partial;   // received char. Populated during reception
} fsm_rx_vars_t;

/*****************************************/
/* variables                             */
/*****************************************/

fsm_rx_vars_t fsm_rx_vars;  // instance of declared struct
bit  fsm_rx_bit;            // received bit
char moved_bit;             // shifted received bit, temporary

/*****************************************/
/* prototypes                            */
/*****************************************/

void fsm_rx_execute_state(void);
void fsm_rx_change_state(void);

/*****************************************/
/* public                                */
/*****************************************/

/**
 * \fn void fsm_rx_init()
 * \brief Init the fsm for reception. Sets all internal variables to 0
 */
void fsm_rx_init() {
	fsm_rx_vars.fsm_state = RX_STATE_S0;
	fsm_rx_vars.nb_bit = 0;
	fsm_rx_vars.logic_counter = 0;
	fsm_rx_vars.data_is_ready = 0;
	fsm_rx_vars.data_ready = 0;
	fsm_rx_vars.data_partial = 0;
}

/**
 * \fn void fsm_rx_update()
 * \brief Public function to update the state machine. Internally it calls the respective functions for executing state action and changing to the next one if necessary
 */
void fsm_rx_update() {
	fsm_rx_execute_state();
	fsm_rx_change_state();
}

/**
 * \fn char fsm_rx_get_rx_data()
 * \brief Function that gets the received data.
 *
 * \return Received char. If no char is ready, it returns 0xFF. Each received char is return one time only
 */
char fsm_rx_get_rx_data() {
	if (fsm_rx_vars.data_is_ready == (char) 1) {
		fsm_rx_vars.data_is_ready = 0;
		return fsm_rx_vars.data_ready;
	}

	return 0xFF;
}

/**
 * \fn static Str_err_e str_destroy (Str_t ** self)
 * \brief Function to write the registered input to the internal fsm variable that keeps track of the input
 *
 * \param rx_bit Value of the rx pin read at interrupt routine
 */
void fsm_rx_set_rx_bit(bit rx_bit) {
	fsm_rx_bit = rx_bit;
}

/*****************************************/
/* private                               */
/*****************************************/

/**
 * \fn void fsm_rx_execute_state()
 * \brief Function that executes the action corresponding to the current state
 */
void fsm_rx_execute_state() {
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

			// processing of incoming bit 
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

/**
 * \fn void fsm_rx_change_state()
 * \brief Function that changes to another state if necessary conditions are met
 */
void fsm_rx_change_state() {
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
