/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/embed/asf/drivers/i2c/i2c.h>
#include <gs/embed/freertos.h>

#include <avr32/io.h>
#include <pdca.h>
#include <twim.h>
#include <gs/asf/avr32/asf.h>
#include <gs/util/drivers/i2c/slave.h>
#include <gs/util/log.h>
#include <gs/util/error.h>
#include <gs/util/stdio.h>
#include <gs/util/check.h>

// change default log group
#define LOG_DEFAULT gs_i2c_log

static struct twi_handle_s {
    volatile avr32_twis_t *twis;
    volatile avr32_twim_t *twim;
} twi_device[AVR32_TWIM_NUM];

//! Define all error conditions
#define AVR32_TWIS_SR_ERROR_MASK (AVR32_TWIS_SR_BUSERR_MASK             \
                                  | AVR32_TWIS_SR_SMBPECERR_MASK        \
                                  | AVR32_TWIS_SR_SMBTOUT_MASK          \
                                  | AVR32_TWIS_SR_ORUN_MASK             \
                                  | AVR32_TWIS_SR_URUN_MASK)

#define PDC_CHAN 0
#define PDC_IRQ AVR32_PDCA_IRQ_0

//! Pointer to the TWIS instance physical address
static volatile avr32_twis_t *twis_inst_slave;

static gs_i2c_slave_receive_t rx_callback;
static gs_i2c_slave_get_rx_buf_t get_buf_callback;
static uint8_t i2c_slave_handle;
static uint8_t * rx_buf;
static uint32_t slave_rx_buffer_size;

__attribute__((__noinline__)) portBASE_TYPE i2c_DSR(void) {

    /* FreeRTOS context switch */
    gs_context_switch_t cswitch = GS_CONTEXT_SWITCH_INIT;

    /* Get status and interrupt mask register values */
    unsigned long status  = twis_inst_slave->sr;

    log_debug_isr("S%lx", status);

    /* Check for bus error */
    if (status & AVR32_TWIS_SR_ERROR_MASK) {
        log_warning_isr("TWIS BUS ERROR %lx %lx", twis_inst_slave->cr, status);
        twis_inst_slave->scr = AVR32_TWIS_SR_ERROR_MASK;
        if (rx_buf != NULL)
            pdca_load_channel(PDC_CHAN, rx_buf, slave_rx_buffer_size);
    }

    /* First byte */
    if (status & AVR32_TWIS_SR_RXRDY_MASK) {

        /* Allocate new frame */
        if (rx_buf == NULL)
            rx_buf = get_buf_callback(i2c_slave_handle);

        /* Setup PDC */
        if (rx_buf != NULL) {
            pdca_load_channel(PDC_CHAN, rx_buf, slave_rx_buffer_size);
            pdca_enable(PDC_CHAN);
            pdca_enable_interrupt_transfer_complete(PDC_CHAN);
            twis_inst_slave->idr = AVR32_TWIS_IDR_RXRDY_MASK;
            twis_inst_slave->scr = AVR32_TWIS_IDR_RXRDY_MASK;

            /* Out of memory, dump byte */
        } else {
            (void) twis_inst_slave->rhr;
        }

    }

    /* Check if the transmission complete or repeated start flags raised */
    if (status & (AVR32_TWIS_SR_TCOMP_MASK | AVR32_TWIS_SR_REP_MASK)) {

        pdca_disable(PDC_CHAN);
        pdca_disable_interrupt_transfer_complete(PDC_CHAN);

        /* Clear spurious bytes received between stop and start */
        while(twis_inst_slave->sr & AVR32_TWIS_IER_RXRDY_MASK) {
            (void) twis_inst_slave->rhr;
            /* log_info("Spurious data %"PRIx32, twis_inst_slave->rhr); */
        }

        /* Now re-enable first byte ISR */
        twis_inst_slave->ier = AVR32_TWIS_IER_RXRDY_MASK;
        /* Deliver frame to I2C interface */
        if (rx_buf != NULL) {
            uint16_t rx_len = slave_rx_buffer_size - pdca_get_load_size(PDC_CHAN);
            if (rx_callback != NULL) {
                rx_callback(i2c_slave_handle, rx_buf, rx_len, &cswitch);
            } else {
                log_error_isr("Invalid rx callback function %p", rx_callback);
            }
            rx_buf = NULL;
        }
        /* Clear transmit complete and repeated start flags */
        twis_inst_slave->scr = AVR32_TWIS_SCR_TCOMP_MASK | AVR32_TWIS_SCR_REP_MASK;

    }

    /* Detect slave address match interrupt */
    if (status & AVR32_TWIS_SR_SAM_MASK) {
        /* If slave has been set in transmit mode */
        if (status & AVR32_TWIS_SR_TRA_MASK) {
            /* Slave transmit not supported, reset TWIS */
            log_error_isr("TWIS TX mode, RESET %lx %lx", twis_inst_slave->cr, twis_inst_slave->sr);

            twis_inst_slave->idr = ~0UL; /* disable interrupts */
            twis_inst_slave->scr = ~0UL; /* clear status flags */

            /* Store settings */
            unsigned long cr = twis_inst_slave->cr;
            unsigned long tr = twis_inst_slave->tr;

            /* Reset */
            twis_inst_slave->cr = AVR32_TWIS_CR_SWRST_MASK;
            twis_inst_slave->scr = ~0UL; /* clear status flags */

            /* Recall settings */
            twis_inst_slave->tr = tr;
            twis_inst_slave->cr = cr;

            /* Enable interrupts */
            twis_inst_slave->ier = AVR32_TWIS_SR_ERROR_MASK |
                    AVR32_TWIS_SR_TCOMP_MASK |
                    AVR32_TWIS_SR_REP_MASK |
                    AVR32_TWIS_SR_RXRDY_MASK |
                    AVR32_TWIS_SR_SAM_MASK;
        } else {
            /* Clear slave address match flag */
            twis_inst_slave->scr = AVR32_TWIS_SR_SAM_MASK;
        }
    }

    return cswitch.task_woken;
}

__attribute__((__naked__)) void i2c_ISR() {
    portENTER_SWITCHING_ISR();
    i2c_DSR();
    portEXIT_SWITCHING_ISR();
}

__attribute__((__noinline__)) portBASE_TYPE pdca_DSR() {

    volatile avr32_pdca_channel_t *pdca_channel = &AVR32_PDCA.channel[PDC_CHAN];

    /* Handle transfer complete interrupt */
    if (pdca_channel->isr & AVR32_PDCA_TRC_MASK) {
        /* Disable TRC interrupt */
        pdca_disable_interrupt_transfer_complete(PDC_CHAN);

        /* Handle not finished reception */
        if(!(twis_inst_slave->sr & AVR32_TWIS_SR_TCOMP_MASK)) {

            /* This is an error scenario - TWIS may have missed a stop condition */
            log_error_isr("PDCA TRC, TWIS RESET %lx %lx", twis_inst_slave->cr, twis_inst_slave->sr);

            /* Reset TWIS */
            twis_inst_slave->idr = ~0UL; /* disable interrupts */
            twis_inst_slave->scr = ~0UL; /* clear status flags */

            /* Store settings */
            unsigned long cr = twis_inst_slave->cr;
            unsigned long tr = twis_inst_slave->tr;

            /* Reset */
            twis_inst_slave->cr = AVR32_TWIS_CR_SWRST_MASK;
            twis_inst_slave->scr = ~0UL; /* clear status flags */

            /* Recall settings */
            twis_inst_slave->tr = tr;
            twis_inst_slave->cr = cr;

            /* Enable interrupts */
            twis_inst_slave->ier = AVR32_TWIS_SR_ERROR_MASK |
                    AVR32_TWIS_SR_TCOMP_MASK |
                    AVR32_TWIS_SR_REP_MASK |
                    AVR32_TWIS_SR_RXRDY_MASK |
                    AVR32_TWIS_SR_SAM_MASK;
        }
    }

    return pdFALSE;
}

__attribute__((__naked__)) void pdca_ISR() {
    portENTER_SWITCHING_ISR();
    pdca_DSR();
    portEXIT_SWITCHING_ISR();
}

static int twis_init(volatile avr32_twis_t *twis, uint8_t handle, uint32_t pba_hz, uint32_t speed, uint8_t chip) {

    twis->idr = ~0UL;
    twis->scr = ~0UL;
    twis->cr = AVR32_TWIS_CR_SWRST_MASK;
    twis->scr = ~0UL;

    const pdca_channel_options_t pdcopt[] = {
        {
            .pid = AVR32_PDCA_PID_TWIS0_RX,
            .transfer_size = PDCA_TRANSFER_SIZE_BYTE,
        },
        {
            .pid = AVR32_PDCA_PID_TWIS1_RX,
            .transfer_size = PDCA_TRANSFER_SIZE_BYTE,
        },
#if (AVR32_TWIM_NUM >= 3)
        {
            .pid = AVR32_PDCA_PID_TWIS2_RX,
            .transfer_size = PDCA_TRANSFER_SIZE_BYTE,
        }
#endif
    };
    pdca_init_channel(PDC_CHAN, &pdcopt[handle]);
    const uint32_t i2c_irq[] = {
        AVR32_TWIS0_IRQ,
        AVR32_TWIS1_IRQ,
#if (AVR32_TWIM_NUM >= 3)
        AVR32_TWIS2_IRQ,
#endif
    };
    irq_register_handler(i2c_ISR, i2c_irq[handle], 0);
    irq_register_handler(pdca_ISR, PDC_IRQ, 0);

    twis_inst_slave = twis;

    /* Ensure that DMA transfer complete interrupt is initially disabled (is enabled on first rx byte) */
    pdca_disable_interrupt_transfer_complete(PDC_CHAN);

    // Enable the TWI Slave Module and allow for clock stretching
    twis->cr = AVR32_TWIS_CR_SEN_MASK | AVR32_TWIS_CR_SMATCH_MASK | AVR32_TWIS_CR_STREN_MASK;

    /* Clock stretch after slave address match */
    twis->cr |= AVR32_TWIS_CR_SOAM_MASK;

    // Set slave address
    twis->cr |= (chip << AVR32_TWIS_CR_ADR_OFFSET);

    // Calculate the wait time from clk falling edge to
    // let the slave control the bus
    uint8_t setup_time = (pba_hz / speed) / 32;
    twis->tr = (setup_time << AVR32_TWIS_TR_SUDAT_OFFSET);

    /* Start the show */
    twis->ier = AVR32_TWIS_SR_ERROR_MASK | AVR32_TWIS_SR_TCOMP_MASK | AVR32_TWIS_SR_REP_MASK | AVR32_TWIS_SR_RXRDY_MASK | AVR32_TWIS_SR_SAM_MASK;

    return 0;
}

void twis_reset(uint8_t handle)
{
    volatile avr32_twis_t *twis;

    if (handle == 0) {
        twis = &AVR32_TWIS0;
    } else if (handle == 1) {
        twis = &AVR32_TWIS1;
#if (AVR32_TWIM_NUM >= 3)
    } else if (handle == 2) {
        twis = &AVR32_TWIS2;
#endif
    } else {
        return;
    }

    twis->idr = ~0UL;
    twis->scr = ~0UL;
    twis->cr = AVR32_TWIS_CR_SWRST_MASK;
    twis->scr = ~0UL;
}

gs_error_t gs_avr_i2c_init(uint8_t handle, gs_avr32_i2c_mode_t mode, uint8_t addr, uint32_t bps)
{
    static bool multimaster_inuse = false;

    gs_log_group_register(LOG_DEFAULT);

    if (mode != GS_AVR_I2C_MASTER && mode != GS_AVR_I2C_MULTIMASTER)
        return GS_ERROR_ARG;

    if (handle > AVR32_TWIM_NUM)
        return GS_ERROR_HANDLE;

    if (handle == 0) {
        twi_device[handle].twis = &AVR32_TWIS0;
        twi_device[handle].twim = &AVR32_TWIM0;
    } else if (handle == 1) {
        twi_device[handle].twis = &AVR32_TWIS1;
        twi_device[handle].twim = &AVR32_TWIM1;
#if (AVR32_TWIM_NUM >= 3)
    } else if (handle == 2) {
        twi_device[handle].twis = &AVR32_TWIS2;
        twi_device[handle].twim = &AVR32_TWIM2;
#endif
    }

    /* Slave */
    if (mode == GS_AVR_I2C_MULTIMASTER) {
        if (!multimaster_inuse) {
            twis_init(twi_device[handle].twis, handle, sysclk_get_peripheral_bus_hz(twi_device[handle].twis), bps, addr);
            multimaster_inuse = true; // This driver only supports one multimaster device, this could however be improved
        } else {
            return GS_ERROR_IN_USE;
        }
    }

    /* Master */
    if (twim_init(handle, sysclk_get_peripheral_bus_hz(twi_device[handle].twim), bps) == 0) {
        return GS_OK;
    }
    return GS_ERROR_ARG;
}

static inline gs_error_t gs_twi_convert_error(int twi_error_code)
{
    switch (twi_error_code) {
        case TWI_SUCCESS:
            return GS_OK;
        case TWI_TIMEOUT:
            return GS_ERROR_TIMEOUT;
        case TWI_INVALID_ARGUMENT:
            return GS_ERROR_ARG;
        default:
            return GS_ERROR_IO;
    }
}

gs_error_t gs_i2c_master_transaction(uint8_t handle, uint8_t addr, const void * tx, size_t txlen, void * rx, size_t rxlen, int timeout_ms)
{
    /* TX */
    twim_transfer_t twim_tx;
    twim_tx.chip = addr;
    twim_tx.buffer = (void *)tx;
    twim_tx.length = txlen;

    /* RX */
    twim_transfer_t twim_rx;
    twim_rx.chip = addr;
    twim_rx.buffer = rx;
    twim_rx.length = rxlen;

    return gs_twi_convert_error(twim_pdc_transfer(handle, &twim_tx, &twim_rx));
}

gs_error_t gs_i2c_slave_set_rx(uint8_t handle, gs_i2c_slave_receive_t rx)
{// This driver only supports one multimaster device
    rx_callback = rx;
    return GS_OK;
}

gs_error_t gs_i2c_slave_set_get_rx_buf(uint8_t device, gs_i2c_slave_get_rx_buf_t get_rx_buf, size_t buf_length)
{
    // This driver only supports one multimaster device
    slave_rx_buffer_size = buf_length;
    get_buf_callback = get_rx_buf;
    return GS_OK;
}

gs_error_t gs_i2c_slave_start(uint8_t handle)
{// This driver only supports one multimaster device
    i2c_slave_handle = handle;
    return GS_OK;
}

gs_error_t gs_i2c_slave_set_response(uint8_t device, const uint8_t * tx, size_t size)
{
    return GS_ERROR_NOT_IMPLEMENTED;
}
