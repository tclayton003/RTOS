/*****************************************************************************
 *
 * \file
 *
 * \brief TWIM driver for AVR32 UC3.
 *
 * This file defines a useful set of functions for TWIM on AVR32 devices.
 *
 *****************************************************************************/
/**
 * Copyright (c) 2010-2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *	this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *	this list of conditions and the following disclaimer in the documentation
 *	and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *	from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *	Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include <inttypes.h>
#include <stdio.h>

#include <gs/util/drivers/i2c/common.h>
#define LOG_DEFAULT gs_i2c_log

#include "twim.h"
#include "pdca.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include <gs/util/check.h>
#include <gs/util/minmax.h>

#define TWIM_ISR_LEVEL 0

#define MAX_TWIM_TRANSFER_SIZE 255

/* Forward declaration */
__attribute__((__naked__)) void twim0_ISR();
__attribute__((__naked__)) void twim1_ISR();
#if (AVR32_TWIM_NUM >= 3)
__attribute__((__naked__)) void twim2_ISR();
#endif

static struct {
    xSemaphoreHandle lock;
    xSemaphoreHandle isr_wait;
    xQueueHandle cmd_queue;
    volatile avr32_twim_t *dev;
    volatile twim_transfer_status_t transfer_status;
    __int_handler irq_handler;
    int irq;
    int pid_rx;
    int pid_tx;
    int pdc_chan_rx;
    int pdc_chan_tx;
} twim[] = {
    {.dev = &AVR32_TWIM0, .irq_handler = twim0_ISR, .irq = AVR32_TWIM0_IRQ, .pid_rx = AVR32_PDCA_PID_TWIM0_RX, .pid_tx = AVR32_PDCA_PID_TWIM0_TX, .pdc_chan_tx = 9, .pdc_chan_rx = 12},
    {.dev = &AVR32_TWIM1, .irq_handler = twim1_ISR, .irq = AVR32_TWIM1_IRQ, .pid_rx = AVR32_PDCA_PID_TWIM1_RX, .pid_tx = AVR32_PDCA_PID_TWIM1_TX, .pdc_chan_tx = 10, .pdc_chan_rx = 13},
#if (AVR32_TWIM_NUM >= 3)
    {.dev = &AVR32_TWIM2, .irq_handler = twim2_ISR, .irq = AVR32_TWIM2_IRQ, .pid_rx = AVR32_PDCA_PID_TWIM2_RX, .pid_tx = AVR32_PDCA_PID_TWIM2_TX, .pdc_chan_tx = 11, .pdc_chan_rx = 14},
#endif
};

#if (0)
#define TWIM_LOG_INTERRUPT_EVENTS  10
static unsigned int dsr_count = 0;
static unsigned int last_dsr_count = 0;
static uint32_t status_reg[TWIM_LOG_INTERRUPT_EVENTS];
#endif

GS_CHECK_STATIC_ASSERT(GS_ARRAY_SIZE(twim) == AVR32_TWIM_NUM, mismatch_in_configured_and_actual_twim_devices);

void twim_reset(int handle) {
    twim_disable_interrupt(handle);
    twim[handle].dev->cr = AVR32_TWIM_CR_MEN_MASK;
    twim[handle].dev->cr = AVR32_TWIM_CR_SWRST_MASK;
    twim[handle].dev->cr = AVR32_TWIM_CR_MEN_MASK;
}

void twim_disable_interrupt(int handle) {
    // Clear commands, transmission isn't restarted when any errors are cleared
    twim[handle].dev->ncmdr = 0;
    twim[handle].dev->cmdr = 0;
    twim[handle].dev->idr = ~0UL;
    twim[handle].dev->scr = ~0UL;
}

void twim_status(int handle) {
    printf("TWIM%i status register: %08X\r\n", handle, (unsigned int)twim[handle].dev->sr);
    printf("TWIM%i cmd register:	%08X\r\n", handle, (unsigned int)twim[handle].dev->cmdr);
    printf("TWIM%i ier: %08X\r\n", handle, (unsigned int)twim[handle].dev->ier);
    printf("TWIM%i imr register:	%08X\r\n", handle, (unsigned int)twim[handle].dev->imr);

    volatile avr32_pdca_channel_t *pdca_channel_tx = pdca_get_handler(twim[handle].pdc_chan_tx);
    printf("PDCA%i tx SR:			%08X\r\n", twim[handle].pdc_chan_tx, (unsigned int)pdca_channel_tx->sr);
    printf("PDCA%i tx ISR:			%08X\r\n", twim[handle].pdc_chan_tx, (unsigned int)pdca_channel_tx->isr);
    printf("PDCA%i tx IMR:			%08X\r\n", twim[handle].pdc_chan_tx, (unsigned int)pdca_channel_tx->imr);

    volatile avr32_pdca_channel_t *pdca_channel_rx = pdca_get_handler(twim[handle].pdc_chan_rx);
    printf("PDCA%i rx SR:			%08X\r\n", twim[handle].pdc_chan_rx, (unsigned int)pdca_channel_rx->sr);
    printf("PDCA%i rx ISR:			%08X\r\n", twim[handle].pdc_chan_rx, (unsigned int)pdca_channel_rx->isr);
    printf("PDCA%i rx IMR:			%08X\r\n", twim[handle].pdc_chan_rx, (unsigned int)pdca_channel_rx->imr);
}

static inline int twim_lock(int handle, int timeout) {
    if (xSemaphoreTake(twim[handle].lock, timeout) == pdFALSE) {
        return -1;
    }
    //log_trace("TWIM[%d] locked (%p)", handle, xTaskGetCurrentTaskHandle());
    return 0;
}

static inline void twim_unlock(int handle) {
    //log_trace("TWIM[%d] unlocked (%p)", handle, xTaskGetCurrentTaskHandle());
    xSemaphoreGive(twim[handle].lock);
}

__attribute__((__noinline__)) portBASE_TYPE twim_DSR(int handle) {

    portBASE_TYPE xTaskWoken = pdFALSE;

    volatile avr32_twim_t *twim_inst = twim[handle].dev;

    // get masked status register value
    const uint32_t status = twim_inst->sr;

#if (TWIM_LOG_INTERRUPT_EVENTS)
    status_reg[dsr_count % TWIM_LOG_INTERRUPT_EVENTS] = status;
    ++dsr_count;
#endif

    /* NACK */
    if (status & AVR32_TWIM_SR_NAK_MASK) {
        twim[handle].transfer_status = TWI_RECEIVE_NACK;
        // Don't acknowledge error, stay in 'error' state to prevent further transfer on the bus
        twim_inst->idr = AVR32_TWIM_IER_NAK_MASK | AVR32_TWIM_IDR_CRDY_MASK;
    }

    /* ARBITRATION LOST */
    if (status & AVR32_TWIM_SR_ARBLST_MASK) {
        twim[handle].transfer_status = TWI_ARBITRATION_LOST;
        // Don't acknowledge error, stay in 'error' state to prevent further transfer on the bus
        twim_inst->idr = AVR32_TWIM_IER_ARBLST_MASK | AVR32_TWIM_IDR_CRDY_MASK;
    }

    /* Ready for command */
    if (status & AVR32_TWIM_SR_CRDY_MASK) {
        uint32_t cmd;
        if(xQueueReceiveFromISR(twim[handle].cmd_queue, &cmd, NULL)){
            /* Put next command in next cmd register */
            twim[handle].dev->ncmdr = cmd;
        }
        if (uxQueueMessagesWaitingFromISR(twim[handle].cmd_queue) == 0) {
            /* Disable the command ready interrupt */
            twim_inst->idr = AVR32_TWIM_IDR_CRDY_MASK;
        }
    }

    /* We are idle now */
    if (status & AVR32_TWIM_SR_IDLE_MASK) {
        /* disable all interrupts and signal task - we are done */
        twim_inst->idr = ~0UL;
        xSemaphoreGiveFromISR(twim[handle].isr_wait, &xTaskWoken);
    }

    return xTaskWoken;
}

__attribute__((__naked__)) void twim0_ISR() {
    portENTER_SWITCHING_ISR();
    twim_DSR(0);
    portEXIT_SWITCHING_ISR();
}

__attribute__((__naked__)) void twim1_ISR() {
    portENTER_SWITCHING_ISR();
    twim_DSR(1);
    portEXIT_SWITCHING_ISR();
}

#if (AVR32_TWIM_NUM >= 3)
__attribute__((__naked__)) void twim2_ISR() {
    portENTER_SWITCHING_ISR();
    twim_DSR(2);
    portEXIT_SWITCHING_ISR();
}
#endif

static void twim_busready_wait(int handle, int timeout) {

    /* Remember when we started waiting */
    portTickType start = xTaskGetTickCount();

    while (1) {

        /* Check if we are idle */
        unsigned long sr = twim[handle].dev->sr;
        if (sr & AVR32_TWIM_SR_IDLE_MASK)
            break;

        /* Check if we have been waiting for too long */
        if (((int)xTaskGetTickCount() - (int)((unsigned)start + (unsigned)timeout)) > 0) {
            twim[handle].transfer_status = TWI_TIMEOUT;
            log_error("TWIM[%d] BUSFREE TIMEOUT %lx %lx %lx", handle, sr, twim[handle].dev->cmdr, twim[handle].dev->cr);
            twim_reset(handle);
            break;
        }

        vTaskDelay(1);
        //cpu_relax();
    }
}

/**
 * Initialize twi master
 * @param handle twi master handle id 0,1,2
 * @param pba_hz speed of the pba bus
 * @param speed speed of the i2c bus in khz
 * @return 0 if ok -1 if otherwise
 */
int twim_init(int handle, int pba_hz, int speed) {

    /* Reset the hardware */
    twim_reset(handle);

    /* Select the speed */
    uint32_t f_prescaled;
    uint8_t cwgr_exp = 0;
    f_prescaled = (pba_hz / speed / 2);

    /* f_prescaled must fit in 8 bits, cwgr_exp must fit in 3 bits */
    while ((f_prescaled > 0xFF) && (cwgr_exp <= 0x7)) {
        cwgr_exp++;
        f_prescaled /= 2;
    }

    /* Check if speed could be obtained */
    if (cwgr_exp > 0x7) {
        log_error("TWIM[%d] Invalid I2C speed", handle);
        return -1;
    }

    uint32_t setup_hold_tm = 250 * (pba_hz/1000000) / 1000; // Min 250ns according to I2C spec.
    uint32_t clock_high = f_prescaled/2;
    uint32_t clock_low = f_prescaled - clock_high - (2*setup_hold_tm);
    uint32_t stasto = 5000 * (pba_hz/1000000) / 1000; // Min ~5us (4.7us) according to I2C spec.

    /* set clock waveform generator register */
    twim[handle].dev->cwgr = (clock_low << AVR32_TWIM_CWGR_LOW_OFFSET)
            | (clock_high << AVR32_TWIM_CWGR_HIGH_OFFSET)
            | (cwgr_exp << AVR32_TWIM_CWGR_EXP_OFFSET)
            | (setup_hold_tm << AVR32_TWIM_CWGR_DATA_OFFSET)
            | (stasto << AVR32_TWIM_CWGR_STASTO_OFFSET);

#if 0
    int f_twim = pba_hz / pow(2, cwgr_exp + 1);
    log_debug("F_TWIM %d %p", f_twim, twim);
    log_debug("t_low %u, t 1/%u Hz, %f us", twim->CWGR.low, f_twim / twim->CWGR.low, 1000000.0 / (f_twim / twim->CWGR.low));
    log_debug("t_high %u, t 1/%u Hz, %f us", twim->CWGR.high, f_twim / twim->CWGR.high, 1000000.0 / (f_twim / twim->CWGR.high));
    log_debug("t_stasto %u, t 1/%u Hz, %f us", twim->CWGR.stasto, f_twim / twim->CWGR.stasto, 1000000.0 / (f_twim / twim->CWGR.stasto));
    log_debug("t_data %u, t 1/%u Hz, %f us", twim->CWGR.data, f_twim / twim->CWGR.data, 1000000.0 / (f_twim / twim->CWGR.data));
#endif

    /* Create synchronization semaphore */
    twim[handle].isr_wait = xSemaphoreCreateBinary();
    if (twim[handle].isr_wait == NULL) {
        log_error("TWIM[%d] Could not create isr_wait sem", handle);
        return -1;
    }

    twim[handle].lock = xSemaphoreCreateMutex();
    if (twim[handle].lock == NULL) {
        log_error("TWIM[%d] Could not create lock", handle);
        return -1;
    }

    twim[handle].cmd_queue = xQueueCreate(64, sizeof(uint32_t));
    if (twim[handle].cmd_queue == NULL) {
        log_error("TWIM[%d] Could not create cmd queue", handle);
        return -1;
    }

    /* Register ISR */
    irq_register_handler(twim[handle].irq_handler, twim[handle].irq, TWIM_ISR_LEVEL);

    /* TX PDC */
    pdca_channel_options_t pdcopt_tx = { .pid = twim[handle].pid_tx, .transfer_size = PDCA_TRANSFER_SIZE_BYTE, };
    pdca_init_channel(twim[handle].pdc_chan_tx, &pdcopt_tx);

    /* RX PDC */
    pdca_channel_options_t pdcopt_rx = { .pid = twim[handle].pid_rx, .transfer_size = PDCA_TRANSFER_SIZE_BYTE, };
    pdca_init_channel(twim[handle].pdc_chan_rx, &pdcopt_rx);

    return 0;
}

/**
   Calculates the length of a cmd and down counts the total length

   @param total_length
   @return length of command
*/
static inline uint8_t get_length(uint32_t * total_length)
{
    uint8_t return_length;
    if (*total_length > MAX_TWIM_TRANSFER_SIZE) {
        return_length = MAX_TWIM_TRANSFER_SIZE;
    } else {
        return_length = *total_length;
    }
    *total_length -= return_length;
    return return_length;
}

int twim_pdc_transfer_with_timeout(int handle, volatile twim_transfer_t *tx, volatile twim_transfer_t *rx, int timeout)
{
    bool do_tx, do_rx;
    uint32_t remaining_tx_length, remaining_rx_length;
    if(tx && tx->length) {
        do_tx = true;
        remaining_tx_length = tx->length;
    } else {
        do_tx = false;
        remaining_tx_length = 0;
    }
    if(rx && rx->length) {
        do_rx = true;
        remaining_rx_length = rx->length;
    } else {
        do_rx = false;
        remaining_rx_length = 0;
    }
    if (!(do_rx || do_tx)) {
        return TWI_INVALID_ARGUMENT;
    }
    timeout = gs_max(timeout, MIN_TWIM_TIMEOUT_TICKS);

    log_info("TWIM[%d] tx: %"PRIu32", rx: %"PRIu32", timeout ticks: %d", handle, remaining_tx_length, remaining_rx_length, timeout);

    if(twim[handle].cmd_queue == NULL) {
        return TWI_NOT_INITIALIZED;
    }
    if (twim_lock(handle, timeout) < 0) {
        log_warning("TWIM[%d] timeout on getting lock, timeout ticks: %d, task: %p", handle, timeout, xTaskGetCurrentTaskHandle());
        return TWI_TIMEOUT_LOCK;
    }

    twim_busready_wait(handle, timeout);

    // Clear the interrupt flags
    twim[handle].dev->idr = ~0UL;
    // Clear the status flags
    twim[handle].dev->scr = ~0UL;
    // Initialize bus transfer status
    twim[handle].transfer_status = TWI_SUCCESS;

    xSemaphoreTake(twim[handle].isr_wait, 0);

    {
        uint32_t cmd_length;
        uint32_t reg_val;
        uint32_t tx_condition = AVR32_TWIM_CMDR_START_MASK;
        uint32_t rx_condition = AVR32_TWIM_CMDR_START_MASK;
        uint32_t cmd_count = 0;
        uint32_t cmdr = 0;
        uint32_t interrupts = (AVR32_TWIM_IER_NAK_MASK | AVR32_TWIM_IER_ARBLST_MASK | AVR32_TWIM_IER_IDLE_MASK);

        /* The command register can only send MAX_TWIM_TRANSFER_SIZE bytes at a time.
           This loop breaks the actual transfer down to commands of at most MAX_TWIM_TRANSFER_SIZE bytes and queues them */
        while(1) {
            if (remaining_tx_length) {
                /* Do a tx command */
                cmd_length = get_length(&remaining_tx_length);
                if (!remaining_tx_length) {
                    if (remaining_rx_length) {
                        /* Do not put in a stop condition if there is rx to be done */
                        tx_condition |= 0;
                    } else {
                        /* Put in a stop condition on last command if no rx is to be done */
                        tx_condition |= AVR32_TWIM_CMDR_STOP_MASK;
                    }
                }
                /* Make the cmd register value */
                reg_val = (tx->chip << AVR32_TWIM_CMDR_SADR_OFFSET) | (cmd_length << AVR32_TWIM_CMDR_NBYTES_OFFSET) | AVR32_TWIM_CMDR_VALID_MASK | tx_condition;
                tx_condition = 0;

            } else if (remaining_rx_length) {
                /* Do a rx command */
                cmd_length = get_length(&remaining_rx_length);
                if (!remaining_rx_length) {
                    /* Put in stop condition if last command */
                    rx_condition |= AVR32_TWIM_CMDR_STOP_MASK;
                } else {
                    /* Acknowledge last byte of command, if there is following rx commands */
                    rx_condition |= AVR32_TWIM_CMDR_ACKLAST_MASK;
                }
                /* Make the cmd register value */
                reg_val = (rx->chip << AVR32_TWIM_CMDR_SADR_OFFSET) | (cmd_length << AVR32_TWIM_CMDR_NBYTES_OFFSET) | AVR32_TWIM_CMDR_VALID_MASK | AVR32_TWIM_CMDR_READ_MASK | rx_condition;
                rx_condition = 0;
            } else {
                break;
            }

            /* Enqueue the cmd, which on interrupt (CRDY) will be dequeued and send */
            if (cmd_count == 0) {
                cmdr = reg_val;
            } else {
                if (xQueueSendToBack(twim[handle].cmd_queue, &reg_val, 0) != pdTRUE) {
                    /* Empty queue */
                    xQueueReset(twim[handle].cmd_queue);
                    twim_unlock(handle);
                    return TWI_INSERT_QUEUE_FAILED;
                }
                interrupts |= AVR32_TWIM_IER_CRDY_MASK;
            }
            ++cmd_count;
        }

        /* Enter critical section while starting the transfer and setting up PDCA (DMA) */
        portENTER_CRITICAL();

        if (do_tx) {
            pdca_load_channel(twim[handle].pdc_chan_tx, tx->buffer, tx->length);
            pdca_enable(twim[handle].pdc_chan_tx);
        }
        if (do_rx) {
            pdca_load_channel(twim[handle].pdc_chan_rx, rx->buffer, rx->length);
            pdca_enable(twim[handle].pdc_chan_rx);
        }

        twim[handle].dev->cmdr = cmdr;
        twim[handle].dev->ier = interrupts;

        portEXIT_CRITICAL();
    }

    /* Wait for hardware to complete */
    twim_transfer_status_t transfer_status;
    if (xSemaphoreTake(twim[handle].isr_wait, timeout) == pdFALSE) {
        transfer_status = TWI_TIMEOUT;
        log_error("TWIM[%d] TIMEOUT ST REG: %lx CMD REG: %lx, t-status: %d",
                  handle, twim[handle].dev->sr, twim[handle].dev->cmdr, twim[handle].transfer_status);
    } else {
        transfer_status = twim[handle].transfer_status;
    }

    pdca_disable(twim[handle].pdc_chan_tx);
    pdca_disable(twim[handle].pdc_chan_rx);

    /* On error: Empty queue and reset */
    if (transfer_status != TWI_SUCCESS) {
        xQueueReset(twim[handle].cmd_queue);
        twim_reset(handle);
    }

    log_debug("TWIM[%d] done, t-status: %d, SR REG: 0x%lx, addr: %"PRIu32"/%"PRIu32,
              handle, transfer_status, twim[handle].dev->sr, tx->chip, rx->chip);

#if (TWIM_LOG_INTERRUPT_EVENTS)
    while (last_dsr_count != dsr_count) {
        log_info("TWIM[%d] %u: 0x%"PRIx32"", handle, last_dsr_count, status_reg[last_dsr_count % TWIM_LOG_INTERRUPT_EVENTS]);
        ++last_dsr_count;
    }
#endif

    twim_unlock(handle);

    return transfer_status;
}
