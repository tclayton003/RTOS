#ifndef TWIM_H_
#define TWIM_H_

#include <avr32/io.h>
#include <stdint.h>

//! Status Clear Register Mask for No Acknowledgements
#define AVR32_TWIM_SCR_NAK_MASK (AVR32_TWIM_SCR_ANAK_MASK \
		| AVR32_TWIM_SCR_DNAK_MASK)
//! Status Register Mask for No Acknowledgements
#define AVR32_TWIM_SR_NAK_MASK  (AVR32_TWIM_SR_ANAK_MASK  \
		| AVR32_TWIM_SR_DNAK_MASK)
//! Interrupt Enable Register Mask for No Acknowledgements
#define AVR32_TWIM_IER_NAK_MASK (AVR32_TWIM_IER_ANAK_MASK \
		| AVR32_TWIM_IER_DNAK_MASK)
//! Frequently used Interrupt Enable Register Mask
#define AVR32_TWIM_IER_STD_MASK (AVR32_TWIM_IER_NAK_MASK  \
		| AVR32_TWIM_IER_ARBLST_MASK)
//! Frequently used Status Clear Register Mask
#define AVR32_TWIM_SR_STD_MASK  (AVR32_TWIM_SR_NAK_MASK  \
		| AVR32_TWIM_SR_ARBLST_MASK)

/** Version of error codes defined in twim_transfer_status_t */
#define TWIM_TRANSFER_STATUS_CODES_VERSION 2

/** Status Codes for TWIM Transfer */
typedef enum twim_transfer_status {
	TWI_SUCCESS = 0,               //!< TWI Transaction Success
	TWI_INVALID_ARGUMENT = -1,     //!< Invalid Argument Passed
	TWI_ARBITRATION_LOST = -2,     //!< Bus Arbitration Lost
	TWI_NO_CHIP_FOUND = -3,        //!< Slave Not Found
	TWI_RECEIVE_NACK = -4,         //!< Addres/Data No Acknowledgement Received
	TWI_SEND_NACK = -5,            //!< Data No Acknowledgement Send
	TWI_INVALID_CLOCK_DIV = -6,    //!< Invalid Clock Divider Value
	TWI_TIMEOUT = -7,              //!< Timeout on transfer
	TWI_TIMEOUT_LOCK = -8,         //!< Timeout on getting device lock
	TWI_INSERT_QUEUE_FAILED = -9,  //!< Failed to insert command in queue
        TWI_NOT_INITIALIZED = -10,     //!< TWIM module not initialized
} twim_transfer_status_t;

typedef struct {
	uint32_t chip;		//! TWI chip address to communicate with
	uint8_t * buffer;	//! Where to find the data
	uint32_t length;	//! How many bytes do we want to transfer
}
twim_transfer_t;

/** Minimum timeout on TWIM transfers */
#define MIN_TWIM_TIMEOUT_TICKS 100

/**
 * Reset twi master
 * @param handle twi master handle id 0,1,2
 */
void twim_reset(int handle);

/**
 * Disable twim interrupts
 * @param handlw twim master handle id 0,1,2
 */
void twim_disable_interrupt(int handle);

/**
 * Print twi master status
 * @param handle twi master handle id 0,1,2
 */
void twim_status(int handle);

/**
 * Initialize twi master
 * @param handle twi master handle id 0,1,2
 * @param pba_hz speed of the pba bus
 * @param speed speed of the i2c bus in khz
 * @return 0 if ok -1 if otherwise
 */
int twim_init(int handle, int pba_hz, int speed);

/**
 * Perform a twi master transfer using the PDC with timeout
 * @param handle twi master handle id 0,1,2
 * @param tx pointer to tx object
 * @param rx pointer to rx object
 * @param timeout Timeout in tick. A minimum of 100 ticks is enforced.
 * @return transfer status
 */
twim_transfer_status_t twim_pdc_transfer_with_timeout(int handle, volatile twim_transfer_t *tx, volatile twim_transfer_t *rx, int timeout);

/**
 * Perform a twi master transfer using the PDC
 * @param handle twi master handle id 0,1,2
 * @param tx pointer to tx object
 * @param rx pointer to rx object
 * @return transfer status
 */
static inline twim_transfer_status_t twim_pdc_transfer(int handle, volatile twim_transfer_t *tx, volatile twim_transfer_t *rx)
{
    return twim_pdc_transfer_with_timeout(handle, tx, rx, MIN_TWIM_TIMEOUT_TICKS);
}

#endif // _TWI_H_
