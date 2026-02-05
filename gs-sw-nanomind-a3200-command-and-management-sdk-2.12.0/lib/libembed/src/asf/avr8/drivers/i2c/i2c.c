/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/embed/asf/drivers/i2c/i2c.h>
#include <gs/util/drivers/i2c/slave.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/twi.h>

#include <gs/embed/freertos.h>
#include <gs/util/error.h>
#include <gs/util/log.h>
#include <gs/util/stdio.h>
#include <gs/util/string.h>

/* Bit rate register  */
#define CPU_CLK 8000000
#define BITRATE(speed)	((CPU_CLK / (2 * (uint32_t)speed) - 8) / (1 << ((TWSR & (_BV(TWPS0) | _BV(TWPS1))) << 1))) // (1 << (n << 1)) is 4^n


static gs_i2c_slave_receive_t rx_callback = NULL;
static gs_i2c_slave_get_rx_buf_t get_buf_callback = NULL;
static uint8_t i2c_slave_handle;
static uint8_t * rx_buf = NULL;
static const uint8_t *tx_buf = NULL;
static uint16_t rx_len;
static uint16_t tx_len;
static uint8_t tx_addr;

static volatile uint16_t tx_cnt;

static int device_mode;

static uint16_t i2c_speed;
static uint8_t i2c_addr;

static int i2c_busy = 0;

/* Last TX time */
static portTickType last_tx = 0;


gs_error_t gs_i2c_slave_set_rx(uint8_t handle, gs_i2c_slave_receive_t rx)
{// This driver only supports one multimaster or slave device
    rx_callback = rx;
    return GS_OK;
}

gs_error_t gs_i2c_slave_set_get_rx_buf(uint8_t device, gs_i2c_slave_get_rx_buf_t get_rx_buf, size_t buf_length)
{// This driver only supports one multimaster or slave device
    get_buf_callback = get_rx_buf;
    return GS_OK;
}

gs_error_t gs_i2c_slave_start(uint8_t handle)
{// This driver only supports one multimaster or slave device
    i2c_slave_handle = handle;
    return GS_OK;
}

/**
 * I2C init
 * This is the AVR8 TWI driver,
 * currently the CPU speed and bus speed is hardcoded!
 * @param handle unused
 * @param mode I2C device mode. Must be either I2C_MASTER or I2C_SLAVE
 * @param addr i2c address
 * @param speed speed in Kb/s
 * @param queue_len_tx Keep these very short, not very much RAM on AVR8
 * @param queue_len_rx Keep these very short, not very much RAM on AVR8
 * @return
 */
gs_error_t gs_avr_i2c_init(uint8_t handle, gs_avr32_i2c_mode_t mode, uint8_t addr, uint32_t bps) {

    /* Validate mode */
    if (mode != GS_AVR_I2C_MULTIMASTER && mode != GS_AVR_I2C_SLAVE)
        return GS_ERROR_NOT_SUPPORTED;
    //
    //    /* Initialise message queues */
    //    if (mode == GS_AVR_I2C_MULTIMASTER) {
    //        tx_queue = xQueueCreate(queue_len_tx, sizeof(i2c_frame_t *));
    //        if (tx_queue == 0)
    //            return GS_ERROR_ALLOC;
    //    }

    /* Set device mode */
    device_mode = mode;

    /* Setup hardware */
    i2c_speed = bps;
    i2c_addr = addr;

    TWCR = 0; // Reset TWI hardware
    TWBR = BITRATE(i2c_speed); // Set bit rate register
    TWAR = i2c_addr << 1; // Set slave address in the two wire address register
    TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE); // Enable acknowledge, twi and interrupts

    return GS_OK;

}

/**
 * Send I2C frame via the selected device
 *
 * @param handle Handle to the device, unused on AVR8
 * @param frame Pointer to I2C frame, should contain destination address as first byte!
 * @param timeout Ticks to wait
 * @return Error code as per error.h
 */
//int i2c_send(uint8_t handle, i2c_frame_t * frame, uint16_t timeout) {
//
//	if ((frame->len == 0) || (frame->len > I2C_MTU))
//		return E_INVALID_BUF_SIZE;
//
//	/* If device is in slave mode, it can only reply to master queries */
//	if (device_mode == I2C_SLAVE) {
//		printf("Device is I2C slave, so it cannot initiate a I2C frame\r\n");
//		return E_INVALID_PARAM;
//	}
//
//	/* Store pointer to frame and start transmitting */
//	/* Note: Blocking send is not supported! */
//	if (xQueueSendToBack(tx_queue, &frame, 0) != pdTRUE) {
//		printf("QUEUE send fail\r\n");
//		return E_NO_BUFFER;
//	}
//
//	/* If bus is idle, send start condition */
//	portENTER_CRITICAL();
//	if (i2c_busy == 0) {
//		//printf("TX-USR\r\n");
//		last_tx = xTaskGetTickCount() + 1;
//		TWCR |= _BV(TWSTA);
//		i2c_busy = 1;
//	}
//	portEXIT_CRITICAL();
//
//	return E_NO_ERR;
//
//}
gs_error_t gs_i2c_master_transaction(uint8_t handle, uint8_t addr, const void * tx, size_t txlen, void * rx, size_t rxlen, int timeout_ms)
{
    /*
     * IMPLEMENT LOCK
     */
    if ((txlen == 0) || (txlen > I2C_MTU))
        return GS_ERROR_ARG;

    if (rxlen != 0) /* No master rx supported */
        return GS_ERROR_NOT_SUPPORTED;

    /* If device is in slave mode, it can only reply to master queries */
    if (device_mode == GS_AVR_I2C_SLAVE) {
        printf("Device is I2C slave, so it cannot do master transaction\r\n");
        return GS_ERROR_NOT_SUPPORTED;
    }

    tx_buf = tx;
    tx_len = txlen;
    tx_addr = addr;
    /* If bus is idle, send start condition */
    portENTER_CRITICAL();
    if (i2c_busy == 0) {
        //printf("TX-USR\r\n");
        last_tx = xTaskGetTickCount() + 1;
        TWCR |= _BV(TWSTA);
        i2c_busy = 1;
    }
    portEXIT_CRITICAL();

    /*
     * WAIT FOR A SEMAPHORE
     */

    return GS_OK;

}
//
///**
// * receive I2C frame from selected device
// *
// * @param handle Handle to the device
// * @param frame Pointer to I2C frame (free this when done!!!)
// * @param timeout Number of ticks to wait for a frame
// * @return Returns error code: E_NO_ERR if a frame is received, or E_TIMEOUT if timed out, E_NO_DEVICE if handle is not a valid device
// */
//int i2c_receive(uint8_t handle, i2c_frame_t ** frame, uint16_t timeout) {
//
//	if (xQueueReceive(rx_queue, frame, timeout) == pdFALSE)
//		return E_TIMEOUT;
//
//	return E_NO_ERR;
//
//}

gs_error_t gs_i2c_slave_set_response(uint8_t bus, const uint8_t * tx, size_t size)
{
    if (bus != i2c_slave_handle) {
        return GS_ERROR_HANDLE;
    }
    tx_buf = tx;
    tx_len = size;
    return GS_OK;
}

/**
 * Try to recover from bus error by checking timeout
 * This function is externally and weakly linked inside the idle() loop
 */
void i2c_hw_poll(void) {
    if ((last_tx != 0) && ((last_tx + 10) < xTaskGetTickCount())) {
        last_tx = 0;

        /* Free current frame */
        if (tx_buf != NULL) {
            tx_buf = NULL;
        }

        rx_len = 0;

        TWCR = 0; // Reset TWI hardware
        TWBR = BITRATE(i2c_speed); // Set bit rate register
        TWAR = i2c_addr << 1; // Set slave address in the two wire address register	
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE); // Enable acknowledge, twi and interrupts
        i2c_busy = 0;
        //driver_debug(DEBUG_I2C, "I2C HW TIMEOUT\r\n");
    }
}

/* Two wire interface ISR */
void TWI_vect(void) __attribute__((signal));
void TWI_vect(void) {

    static uint8_t status;
    static portBASE_TYPE xTaskWoken;
    static uint8_t flag;

    status = TWSR & 0xF8;
    xTaskWoken = pdFALSE;
    flag = 0;

    //driver_debug(DEBUG_I2C, "S%x\n\r", status);
    last_tx = 0;

    /* The I2C driver is one _big_ state-machine */
    switch (status) {

	/**
	 * SLAVE TRANSMIT EVENTS
	 */

	case TW_ST_SLA_ACK:								// 0xA8 SLA+R received, ACK returned
	case TW_ST_ARB_LOST_SLA_ACK:					// 0xB0 Arbitration lost in SLA+RW, SLA+R received, ACK returned
	case TW_ST_DATA_ACK:							// 0xB8 Data transmitted, ACK received

            /* First check if the previous frame was completed */
            if (tx_buf == NULL) {
                //driver_debug(DEBUG_I2C, "NO NEXT ELEMENT\r\n");
                TWCR |= _BV(TWSTO);

                /* Reset counter */
                tx_cnt = 0;
                break;

            }

            /* Send next byte */
            if (tx_cnt < tx_len) {
                TWDR = tx_buf[tx_cnt++];

		/* Warn about too long reads */
            } else {
                //driver_debug(DEBUG_I2C, "Too long I2C read\r\n");
            }

            break;

	case TW_ST_DATA_NACK:							// 0xC0 Data transmitted, NACK received
	case TW_ST_LAST_DATA:							// 0xC8 Last data byte transmitted, ACK received

            /* Frame completed, free buffer */
            tx_buf = NULL;
            break;

            /**
             * SLAVE RECEIVE EVENTS
             */

            /* Beginning of new RX Frame */
	case TW_SR_SLA_ACK: 							// 0x60 SLA+W received, ACK returned
	case TW_SR_ARB_LOST_SLA_ACK:					// 0x68 Arbitration lost (in master mode) and addressed as slave, ACK returned

            /* Don't do nothing */
            i2c_busy = 1;
            break;

            /* DATA received */
	case TW_SR_DATA_ACK: 							// 0x80 Data received, ACK returned
	case TW_SR_DATA_NACK: 							// 0x88 Data received, NACK returned

            i2c_busy = 1;

            /* Get buffer */
            if (rx_buf == NULL) {
                rx_buf = get_buf_callback(i2c_slave_handle);
                if (rx_buf == NULL) {
                    //driver_debug(DEBUG_I2C, "Failed to get buffer\r\n");
                    break;
                }
                rx_len = 0;
            }

            /* Store data */
            if (rx_len < I2C_MTU)
                rx_buf[rx_len++] = TWDR;

            break;

            /* End of frame */
	case TW_SR_STOP: 								// 0xA0 Stop condition received or repeated start

            i2c_busy = 0;

            /* Break if no RX frame */
            if (rx_buf == NULL)
                break;

            /* Deliver frame */
            if (rx_callback != NULL) {
                gs_context_switch_t cswitch = GS_CONTEXT_SWITCH_INIT;
                rx_callback(i2c_slave_handle, rx_buf, rx_len, &cswitch);
            }

            /* Prepare reply or next frame */
            if (device_mode == GS_AVR_I2C_SLAVE) {
                tx_cnt = 0;
            } else {
                /* Try new transmission */
                //			if (uxQueueMessagesWaitingFromISR(tx_queue) > 0 || rx_buf != NULL) {
                //				driver_debug(DEBUG_I2C, "CONTINUE\r\n");
                //				flag |= _BV(TWSTA);
                //				i2c_busy = 1;
                //			}

            }

            /* We are done with this buffer */
            rx_buf = NULL;

            break;

            /**
             * MASTER TRANSMIT EVENTS
             */

            /* Frame START has been signalled */
	case TW_START: 									// 0x08 START has been transmitted
	case TW_REP_START: 								// 0x10 Repeated START has been transmitted

            i2c_busy = 1;

            /* First check if the previous frame was completed */
            if (tx_buf == NULL) {
                /* Reset counter */
                tx_cnt = 0;
                break;
            }

            /* Send destination addr */
            TWDR = tx_addr << 1;

            break;

            /* Data ACK */
	case TW_MT_SLA_ACK: 							// 0x18 SLA+W has been transmitted, ACK received
	case TW_MT_DATA_ACK: 							// 0x28 Data byte has been transmitted,  ACK received

            i2c_busy = 1;

            /* If there is data left in the tx_buf */
            if (tx_buf != NULL && tx_cnt < tx_len) {
                TWDR = *(tx_buf + tx_cnt);
                tx_cnt++;
                break;
            }

            /* Deliberate falltrough to stop */

            /* Data abort from slave (usually because full buffer, should never happen) */
	case TW_MT_SLA_NACK: 							// 0x20 SLA+W transmitted, NACK returned
	case TW_MT_DATA_NACK: 							// 0x30 Data transmitted, NACK returned

            /* Clear TX frame */
            if (tx_buf != NULL) {
                tx_buf = NULL;
            }
            /* Set STOP condition */
            flag |= _BV(TWSTO);
            i2c_busy = 0;
            break;

	case TW_MT_ARB_LOST: 							// 0x38 Arbitration lost, return to slave mode.

            /* Retry the transmission by resetting tx coutner and setting start flag */
            tx_cnt = 0;
            flag |= _BV(TWSTA);
            i2c_busy = 1;

            break;

            /**
             * MASTER RECEIVE EVENTS
             */

            /* The AVR never receives data from a slave device, so these events counts as error */

	case TW_MR_SLA_ACK:								// 0x40 SLA+R transmitted, ACK received
	case TW_MR_SLA_NACK:							// 0x48 SLA+R transmitted, NACK received
	case TW_MR_DATA_ACK:							// 0x50 Data received, ACK returned
	case TW_MR_DATA_NACK:							// 0x58 Data received, NACK returned

            /**
             * TWI ERROR EVENTS
             */
	case TW_BUS_ERROR: 								// 0x00 Bus error
	default:
            if (tx_buf != NULL) {
                tx_buf = NULL;
            }

            rx_len = 0;

            TWCR = 0; // Reset TWI hardware
            TWBR = BITRATE(i2c_speed); // Set bit rate register
            TWAR = i2c_addr << 1; // Set slave address in the two wire address register
            TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE); // Enable acknowledge, twi and interrupts
            i2c_busy = 0;
            //driver_debug(DEBUG_I2C, "I2C ERR %hhx\r\n", status);
            break;
    }

    /* Release the I2C hardware by writing logic one to TWINT. */
    TWCR = flag | _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT);

    /* Perhaps do context switch */
    if (xTaskWoken == pdTRUE)
        taskYIELD();
}

