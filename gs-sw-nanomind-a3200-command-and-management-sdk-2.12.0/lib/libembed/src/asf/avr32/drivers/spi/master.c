/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/embed/asf/drivers/spi/master.h>
#include <gs/embed/freertos.h>

#include <gpio.h>

#include <pdca.h>
#include <gs/asf/avr32/asf.h>
#include <gs/util/log.h>
#include <gs/util/check.h>
#include <gs/util/minmax.h>

// change default log group
#define LOG_DEFAULT gs_spi_log

#define SPI_TIMEOUT 15000
#define div_ceil(a, b)	(((a) + (b) - 1) / (b))

struct {
    volatile avr32_spi_t * base;        /*! The SPI hardware base address */
    xSemaphoreHandle lock;              /*! Lock that is used internally in the SPI driver */
    xSemaphoreHandle sem_isr;           /*! Semaphore for the ISR signal */
    int pdc_channel_rx;                 /*! PDC channel to use for RX */
    int pdc_channel_tx;                 /*! PDC channel to use for TX */
} spidev[] = {
    {.base = &AVR32_SPI0, .pdc_channel_rx = 8, .pdc_channel_tx = 7, .sem_isr = 0, .lock = 0},
#if (AVR32_SPI_NUM >= 2)
    {.base = &AVR32_SPI1, .pdc_channel_rx = 6, .pdc_channel_tx = 5, .sem_isr = 0, .lock = 0},
#endif
};

static gs_error_t gs_spi_setup_chip(gs_spi_chip_t * chip);
static gs_error_t __attribute__ ((warn_unused_result)) gs_spi_select_chip(gs_spi_chip_t * chip, int timeout_ms);
static gs_error_t gs_spi_unselect_chip(gs_spi_chip_t * chip);

GS_CHECK_STATIC_ASSERT(GS_ARRAY_SIZE(spidev) == AVR32_SPI_NUM, mismatch_in_configured_and_actual_spi_devices);

gs_error_t gs_spi_init_device(uint8_t handle, bool decode)
{
    gs_log_group_register(LOG_DEFAULT);

    GS_CHECK_HANDLE(handle < GS_ARRAY_SIZE(spidev));

    /* Create PDCA sync semaphore */
    if (spidev[handle].sem_isr == NULL) {
        spidev[handle].sem_isr = xSemaphoreCreateBinary();
    }

    /* Create device mutex */
    if (spidev[handle].lock == NULL) {
        spidev[handle].lock = xSemaphoreCreateMutex();
    }

    volatile avr32_spi_t * spi = spidev[handle].base;

    /* Setup clock */
    sysclk_enable_peripheral_clock(spi);

    /* Reset hardware */
    spi->cr = AVR32_SPI_CR_SWRST_MASK;

    /* Setup master mode */
    spi->MR.mstr = 1;
    spi->MR.modfdis = 1;
    spi->MR.llb = 0;
    spi->MR.pcs = (1 << AVR32_SPI_MR_PCS_SIZE) - 1;
    spi->MR.ps = 0;
    spi->MR.pcsdec = decode;
    spi->MR.dlybcs = 0;

    /* Enable hardware */
    // cppcheck-suppress redundantAssignment
    spi->cr = AVR32_SPI_CR_SPIEN_MASK;

    return GS_OK;
}

static gs_error_t gs_spi_setup_chip(gs_spi_chip_t * chip)
{
    GS_CHECK_HANDLE(chip != NULL);
    GS_CHECK_ARG(chip->spi_mode <= 3);
    GS_CHECK_ARG((chip->bits >= 8) && (chip->bits <= 16));

    volatile avr32_spi_t * spi = spidev[chip->handle].base;

    // Calculate divider
    uint32_t baudDiv = div_ceil(sysclk_get_peripheral_bus_hz(spi), chip->baudrate);
    if (baudDiv < 1 || baudDiv > 255) {
        log_error("gs_spi_setup_chip: invalid baudrate/bps: %"PRIu32", clk: %"PRIu32", divide: %"PRIu32,
                  chip->baudrate, sysclk_get_peripheral_bus_hz(spi), baudDiv);
        return GS_ERROR_ARG;
    }

    // check decode versus chip-select
    uint8_t reg;
    if (spi->MR.pcsdec) {
        if (chip->chipselect > 14) {
            log_error("SPI (decode) invalid chip-select: %u, handle: %d", chip->chipselect, chip->handle);
            return GS_ERROR_HANDLE;
        }
        reg = chip->chipselect / 4;
    } else {
        if (chip->chipselect > 3) {
            log_error("SPI invalid chip-select: %u, handle: %d", chip->chipselect, chip->handle);
            return GS_ERROR_HANDLE;
        }
        reg = chip->chipselect;
    }

    /* Will use CSR0 struct, these are the same for CSR0 to CSR3. */
    volatile avr32_spi_csr0_t * csr;
    switch (reg) {
        case 0: csr = (avr32_spi_csr0_t *) &spi->CSR0; break;
        case 1: csr = (avr32_spi_csr0_t *) &spi->CSR1; break;
        case 2: csr = (avr32_spi_csr0_t *) &spi->CSR2; break;
        case 3: csr = (avr32_spi_csr0_t *) &spi->CSR3; break;
        default: return GS_ERROR_ARG;
    }

    /* Setup chip select register */
    csr->cpol   = chip->spi_mode >> 1;
    csr->ncpha  = (chip->spi_mode & 0x1) ^ 0x1;
    csr->csaat  = chip->stay_act;
    csr->bits   = chip->bits - 8;
    csr->scbr   = baudDiv;
    csr->dlybs  = chip->spck_delay;
    csr->dlybct = chip->trans_delay;

    if (chip->use_gpio_cs) {
        gpio_configure_pin((chip->gpio_chipselect.port << 5 | chip->gpio_chipselect.pin), GPIO_DIR_OUTPUT | GPIO_PULL_UP | GPIO_INIT_HIGH);
    }


    return GS_OK;
}

__attribute__((always_inline)) static inline gs_error_t gs_spi_write_internal(volatile avr32_spi_t * spi, gs_spi_chip_t * chip, uint16_t data)
{
    unsigned int timeout = SPI_TIMEOUT;

    while (!(spi->sr & AVR32_SPI_SR_TDRE_MASK)) {
        if (!timeout--) {
            log_error("SPI Error write. chip-select: %u, handle: %d", chip->chipselect, chip->handle);
            return GS_ERROR_TIMEOUT;
        }
    }

    spi->tdr = data << AVR32_SPI_TDR_TD_OFFSET;

    return GS_OK;
}

__attribute__((always_inline)) static inline gs_error_t gs_spi_read_internal(volatile avr32_spi_t * spi, gs_spi_chip_t * chip, uint16_t * data)
{
    unsigned int timeout = SPI_TIMEOUT;

    /* Start transfer */
    spi->tdr = 0x0000 << AVR32_SPI_TDR_TD_OFFSET;

    /* Poll for data ready */
    while (!(spi->sr & AVR32_SPI_SR_RDRF_MASK)) {
        if (!timeout--) {
            log_error("SPI read poll timeout. chip-select: %u, handle: %d", chip->chipselect, chip->handle);
            *data = 0x0000;
            return GS_ERROR_TIMEOUT;
        }
    }

    *data = spi->rdr >> AVR32_SPI_RDR_RD_OFFSET;

    return GS_OK;
}

__attribute__((always_inline)) static inline gs_error_t gs_spi_write_read_internal(volatile avr32_spi_t * spi, gs_spi_chip_t * chip, uint16_t tx_data, uint16_t * rx_data)
{
    unsigned int timeout = SPI_TIMEOUT;

    /* Start transfer */
    spi->tdr = tx_data << AVR32_SPI_TDR_TD_OFFSET;

    /* Poll for data ready */
    while (!(spi->sr & AVR32_SPI_SR_RDRF_MASK)) {
        if (!timeout--) {
            *rx_data = 0x0000;
            log_error("SPI read poll timeout. chip-select: %u, handle: %d", chip->chipselect, chip->handle);
            return GS_ERROR_TIMEOUT;
        }
    }

    *rx_data = spi->rdr >> AVR32_SPI_RDR_RD_OFFSET;

    return GS_OK;
}

static gs_error_t gs_spi_select_chip(gs_spi_chip_t * chip, int timeout_ms)
{
    GS_CHECK_HANDLE(chip != NULL);

    volatile avr32_spi_t * spi = spidev[chip->handle].base;

    /* Lock spi device */
    const TickType_t ticks = timeout_ms / portTICK_RATE_MS;
    if (xSemaphoreTake(spidev[chip->handle].lock, ticks) == pdFALSE) {
        log_error("SPI lock timeout. chip-select: %u, handle: %d", 
                chip->chipselect, chip->handle);
        return GS_ERROR_TIMEOUT;
    }

    const bool decode = spi->MR.pcsdec;
    gs_error_t error = GS_OK;
    if (decode) {
        // reconfigure chip, as same csr registers may be used with different configuration
    	error = gs_spi_setup_chip(chip);
    } else if (chip->chipselect > 3) {
    	error = GS_ERROR_ARG;
    }

    if (error) {
        xSemaphoreGive(spidev[chip->handle].lock);
        return error;
    }

    /**
     * Select chip:
     * The following code is a snippet from the atmel asf SPI driver
     * It's copied here in order to avoide the SPI locking already present in the ASF driver
     */

    /* Assert all lines; no peripheral is selected. */
    spi->mr |= AVR32_SPI_MR_PCS_MASK;

    if (decode) {
        spi->mr &= ~AVR32_SPI_MR_PCS_MASK | (chip->chipselect << AVR32_SPI_MR_PCS_OFFSET);
    } else {
        spi->mr &= ~(1 << (AVR32_SPI_MR_PCS_OFFSET + chip->chipselect));
    }
    if (chip->use_gpio_cs) {
        gs_gpio_set(chip->gpio_chipselect, false);
    }
    return GS_OK;
}

static gs_error_t gs_spi_unselect_chip(gs_spi_chip_t * chip)
{
    /**
     * Unselect chip:
     * The following code is a snippet from the atmel asf SPI driver
     * It's copied here in order to avoide the SPI locking already present in the ASF driver
     */
    volatile avr32_spi_t * spi = spidev[chip->handle].base;

    unsigned int timeout = SPI_TIMEOUT;

    while (!(spi->sr & AVR32_SPI_SR_TXEMPTY_MASK)) {
        if (!timeout--) {
            log_error("SPI tx empty timeout. chip-select: %u, handle: %d", chip->chipselect, chip->handle);
            xSemaphoreGive(spidev[chip->handle].lock);
            if (chip->use_gpio_cs) {
                gs_gpio_set(chip->gpio_chipselect, true);
            }
            return GS_ERROR_TIMEOUT;
        }
    }

    /* Assert all lines; no peripheral is selected. */
    spi->mr |= AVR32_SPI_MR_PCS_MASK;

    /* Last transfer, so de-assert the current NPCS if CSAAT is set. */
    spi->cr = AVR32_SPI_CR_LASTXFER_MASK;

    if (chip->use_gpio_cs) {
        gs_gpio_set(chip->gpio_chipselect, true);
    }
    xSemaphoreGive(spidev[chip->handle].lock);

    return GS_OK;
}

typedef struct {
    bool configured;
    gs_spi_chip_t chip;
} spi_slave_t;

#define MAX_SPI_SLAVES   10
static spi_slave_t slaves[MAX_SPI_SLAVES];

gs_error_t gs_spi_asf_master_configure_slave(uint8_t slave, const gs_spi_chip_t * chip)
{
    GS_CHECK_HANDLE(slave < MAX_SPI_SLAVES);
    GS_CHECK_ARG(chip != NULL);

    spi_slave_t * pslave = &slaves[slave];
    memset(pslave, 0, sizeof(*pslave));
    pslave->chip = *chip;

    gs_error_t error = gs_spi_setup_chip(&pslave->chip);
    if (error) {
        return error;
    }

    pslave->configured = true;
    return GS_OK;
}

bool gs_spi_master_is_slave_configured(uint8_t slave)
{
    if (slave < MAX_SPI_SLAVES) {
        return slaves[slave].configured;
    }
    return false;
}

gs_error_t gs_spi_master_transaction(uint8_t slave, const void * tx_in, void * rx_in, size_t size, int timeout_ms)
{
    gs_spi_master_trans_t t = {tx_in, rx_in, size};

    return gs_spi_master_transactions(slave, &t, 1, timeout_ms);
}

#define MIN_TIMEOUT 400

gs_error_t gs_spi_master_transactions(uint8_t slave, gs_spi_master_trans_t *trans, size_t count, int timeout_ms)
{
    GS_CHECK_HANDLE(slave < MAX_SPI_SLAVES);
    GS_CHECK_ARG(trans != NULL);
    GS_CHECK_ARG(count > 0);

    volatile int flush_register_dummy;

    spi_slave_t * pslave = &slaves[slave];
    GS_CHECK_HANDLE(pslave->configured);
    volatile avr32_spi_t *spi = spidev[pslave->chip.handle].base;
    gs_error_t ret = gs_spi_select_chip(&pslave->chip, gs_max(timeout_ms, MIN_TIMEOUT));
    if (ret == GS_OK)
    {
        for (unsigned int t = 0; t < count; t++)
        {
            const uint8_t * tx = trans[t].tx;
            uint8_t * rx = trans[t].rx;

            unsigned int i = trans[t].size;

            if (tx && !rx) {
                while(i-- && ret == GS_OK)
                {
                    ret = gs_spi_write_internal(spi, &pslave->chip, (uint16_t)*tx++);
                }
            } else if (rx && !tx) {
                /* Make sure TX is finished */
                int timeout = SPI_TIMEOUT;
                while(!(spi->sr & AVR32_SPI_SR_TXEMPTY_MASK)) {
                    timeout--;
                    if (timeout <= 0) {
                        return GS_ERROR_TIMEOUT;
                    }
                }
                uint16_t data;
                flush_register_dummy = spi->rdr; // Flushing rx data register
                while(i-- && ret == GS_OK)
                {
                    ret = gs_spi_read_internal(spi, &pslave->chip, &data);
                    *rx++ = (uint8_t) data;
                }
            } else if (tx && rx) {
                uint16_t data;
                // cppcheck-suppress unreadVariable - flushing
                flush_register_dummy = spi->rdr; // Flushing rx data register
                while(i-- && ret == GS_OK)
                {
                    ret = gs_spi_write_read_internal(spi, &pslave->chip, (uint16_t)*tx++, &data);
                    *rx++ = (uint8_t) data;
                }
            }
        }
        gs_spi_unselect_chip(&pslave->chip);
    }

    return ret;
}
