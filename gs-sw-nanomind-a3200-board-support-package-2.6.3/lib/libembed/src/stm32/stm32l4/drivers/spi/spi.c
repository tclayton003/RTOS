/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/util/sem.h>
#include <gs/util/check.h>
#include <gs/util/drivers/spi/master.h>
#include <gs/embed/stm32/stm32l4/drivers/spi/spi.h>
#include <gs/embed/stm32/stm32l4/drivers/gpio/gpio.h>
#include <gs/embed/stm32/stm32l4/error.h>
#include <gs/embed/freertos.h>
#include "../dma/dma.h"
#include "../common.h"

#define GS_EMBED_SPI_NUMBER_OF_HANDLES 3
#define GS_EMBED_SPI_MAX_NUMBER_OF_SLAVES 20

typedef struct gs_stm32l4_spi_handle {
    //! STM32 spi_handle
    SPI_HandleTypeDef stm32_spi_handle;
    //! Bool true if active
    bool inuse;
    //! Lock
    gs_sem_t lock;
    //! Semaphore indicating completion of transmit/receive
    gs_sem_t isr_sync_sem;
    //! Transfer status
    volatile gs_error_t transfer_status;
    //! Use polling mode
    bool polling_mode;
} gs_stm32l4_spi_handle_t;

typedef struct gs_stm32l4_spi_slave {
    //! spi_handle
    gs_stm32l4_spi_handle_t * spi_handle;
    //! Bool true if active
    bool inuse;
    //! Slave/Chip select callback
    slave_select_cb_t slave_select_cb;
    //! Slave_id (self)
    uint8_t slave_identifier;
    //! Slave specific configuration
    gs_stm32l4_spi_slave_config_t config;

} gs_stm32l4_spi_slave_t;

static gs_stm32l4_spi_handle_t spi_handles[GS_EMBED_SPI_NUMBER_OF_HANDLES + 1];
static gs_stm32l4_spi_slave_t spi_slaves[GS_EMBED_SPI_MAX_NUMBER_OF_SLAVES];


static DMA_HandleTypeDef handle_dma1_channel4;
static DMA_HandleTypeDef handle_dma1_channel5;
static DMA_HandleTypeDef handle_dma2_channel1;
static DMA_HandleTypeDef handle_dma2_channel2;
static DMA_HandleTypeDef handle_dma2_channel3;
static DMA_HandleTypeDef handle_dma2_channel4;

static uint32_t system_clock_freq_hz;

gs_error_t gs_stm32l4_spi_init(uint8_t device, const gs_stm32_spi_config_t *config)
{
    GS_CHECK_HANDLE(device >= 1);
    GS_CHECK_HANDLE(device <= GS_EMBED_SPI_NUMBER_OF_HANDLES);

    gs_stm32l4_spi_handle_t * handle = &spi_handles[device];
    IRQn_Type SPIx_DMA_TX_IRQn, SPIx_DMA_RX_IRQn;
    DMA_Channel_TypeDef * SPIx_TX_DMA_CHANNEL, *SPIx_RX_DMA_CHANNEL;
    uint32_t SPIx_TX_DMA_REQUEST, SPIx_RX_DMA_REQUEST;

    gs_dma_channel_t dma_channel_tx, dma_channel_rx;

    DMA_HandleTypeDef * hdma_tx;
    DMA_HandleTypeDef * hdma_rx;

    handle->polling_mode = config->polling_mode;

    switch (device) {
        case 1:
            hdma_tx = &handle_dma2_channel4;
            hdma_rx = &handle_dma2_channel3;
            handle->stm32_spi_handle.Instance = SPI1;
            /* Enable I2Cx clock */
            __HAL_RCC_SPI1_CLK_ENABLE();
            /* Enable DMAx clock */
            __HAL_RCC_DMA2_CLK_ENABLE();
            SPIx_DMA_TX_IRQn = DMA2_Channel4_IRQn;
            SPIx_DMA_RX_IRQn = DMA2_Channel3_IRQn;
            SPIx_TX_DMA_CHANNEL = DMA2_Channel4;
            SPIx_RX_DMA_CHANNEL = DMA2_Channel3;
            SPIx_TX_DMA_REQUEST = DMA_REQUEST_4;
            SPIx_RX_DMA_REQUEST = DMA_REQUEST_4;
            dma_channel_tx.dma = 2;
            dma_channel_tx.channel = 4;
            dma_channel_rx.dma = 2;
            dma_channel_rx.channel = 3;
            break;
        case 2:
            hdma_tx = &handle_dma1_channel5;
            hdma_rx = &handle_dma1_channel4;
            handle->stm32_spi_handle.Instance = SPI2;
            /* Enable I2Cx clock */
            __HAL_RCC_SPI2_CLK_ENABLE();
            /* Enable DMAx clock */
            __HAL_RCC_DMA1_CLK_ENABLE();
            SPIx_DMA_TX_IRQn = DMA1_Channel5_IRQn;
            SPIx_DMA_RX_IRQn = DMA1_Channel4_IRQn;
            SPIx_TX_DMA_CHANNEL = DMA1_Channel5;
            SPIx_RX_DMA_CHANNEL = DMA1_Channel4;
            SPIx_TX_DMA_REQUEST = DMA_REQUEST_1;
            SPIx_RX_DMA_REQUEST = DMA_REQUEST_1;
            dma_channel_tx.dma = 1;
            dma_channel_tx.channel = 5;
            dma_channel_rx.dma = 1;
            dma_channel_rx.channel = 4;
            break;
        case 3:
            hdma_tx = &handle_dma2_channel2;
            hdma_rx = &handle_dma2_channel1;
            handle->stm32_spi_handle.Instance = SPI3;
            /* Enable I2Cx clock */
            __HAL_RCC_SPI3_CLK_ENABLE();
            /* Enable DMAx clock */
            __HAL_RCC_DMA2_CLK_ENABLE();
            SPIx_DMA_TX_IRQn = DMA2_Channel2_IRQn;
            SPIx_DMA_RX_IRQn = DMA2_Channel1_IRQn;
            SPIx_TX_DMA_CHANNEL = DMA2_Channel2;
            SPIx_RX_DMA_CHANNEL = DMA2_Channel1;
            SPIx_TX_DMA_REQUEST = DMA_REQUEST_3;
            SPIx_RX_DMA_REQUEST = DMA_REQUEST_3;
            dma_channel_tx.dma = 2;
            dma_channel_tx.channel = 2;
            dma_channel_rx.dma = 2;
            dma_channel_rx.channel = 1;
            break;
        default:
            return GS_ERROR_ARG;
    }

    handle->stm32_spi_handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    handle->stm32_spi_handle.Init.Direction = SPI_DIRECTION_2LINES;
    handle->stm32_spi_handle.Init.CLKPhase = SPI_PHASE_1EDGE;
    handle->stm32_spi_handle.Init.CLKPolarity = SPI_POLARITY_LOW;
    handle->stm32_spi_handle.Init.DataSize = SPI_DATASIZE_8BIT;
    handle->stm32_spi_handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
    handle->stm32_spi_handle.Init.TIMode = SPI_TIMODE_DISABLE;
    handle->stm32_spi_handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    handle->stm32_spi_handle.Init.CRCPolynomial = 7;
    handle->stm32_spi_handle.Init.CRCLength = SPI_CRC_LENGTH_8BIT;
    handle->stm32_spi_handle.Init.NSS = SPI_NSS_SOFT;
    handle->stm32_spi_handle.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    handle->stm32_spi_handle.Init.Mode = SPI_MODE_MASTER;

    HAL_StatusTypeDef res_hal = HAL_SPI_Init(&handle->stm32_spi_handle);
    if (res_hal != HAL_OK) {
        return gs_hal_status_to_error(res_hal);
    }

    gs_error_t res = gs_sem_create(1, &handle->lock);
    if (res != GS_OK) {
        return res;
    }
    if (!handle->polling_mode) {
        res = gs_sem_create(0, &handle->isr_sync_sem);
        if (res != GS_OK) {
            return res;
        }
        __HAL_LINKDMA(&handle->stm32_spi_handle, hdmatx, *hdma_tx);
        __HAL_LINKDMA(&handle->stm32_spi_handle, hdmarx, *hdma_rx);

        res = gs_dma_assign_handle_to_channel(dma_channel_tx, handle->stm32_spi_handle.hdmatx);

        if (res != GS_OK) {
            return GS_ERROR_HANDLE;
        }
        if (gs_dma_assign_handle_to_channel(dma_channel_rx, handle->stm32_spi_handle.hdmarx) != GS_OK) {
            return GS_ERROR_HANDLE;
        }

        /* Configure the DMA handler for Transmission process */
        hdma_tx->Instance = SPIx_TX_DMA_CHANNEL;
        hdma_tx->Init.Request = SPIx_TX_DMA_REQUEST;
        hdma_tx->Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_tx->Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_tx->Init.MemInc = DMA_MINC_ENABLE;
        hdma_tx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_tx->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_tx->Init.Mode = DMA_NORMAL;
        hdma_tx->Init.Priority = DMA_PRIORITY_LOW;

        res_hal = HAL_DMA_Init(hdma_tx);
        if (res_hal != HAL_OK) {
            return gs_hal_status_to_error(res_hal);
        }

        /* Configure the DMA handler for Transmission process */
        hdma_rx->Instance = SPIx_RX_DMA_CHANNEL;
        hdma_rx->Init.Request = SPIx_RX_DMA_REQUEST;
        hdma_rx->Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_rx->Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_rx->Init.MemInc = DMA_MINC_ENABLE;
        hdma_rx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_rx->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_rx->Init.Mode = DMA_NORMAL;
        hdma_rx->Init.Priority = DMA_PRIORITY_HIGH;

        res_hal = HAL_DMA_Init(hdma_rx);
        if (res_hal != HAL_OK) {
            return gs_hal_status_to_error(res_hal);
        }

        /* Configure the NVIC for DMA */

        /* NVIC configuration for DMA transfer complete interrupt (I2Cx_TX) */
        HAL_NVIC_SetPriority(SPIx_DMA_TX_IRQn, GS_STM32L4_DEFAULT_ISR_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(SPIx_DMA_TX_IRQn);

        /* NVIC configuration for DMA transfer complete interrupt (I2Cx_RX) */
        HAL_NVIC_SetPriority(SPIx_DMA_RX_IRQn, GS_STM32L4_DEFAULT_ISR_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(SPIx_DMA_RX_IRQn);
    }


    handle->inuse = true;

    system_clock_freq_hz = HAL_RCC_GetPCLK1Freq();

    return GS_OK;
}

/**
   Gets baudrate prescaler value according to wanted baudrate.
   Note, that it is flooring, meaning taking the lower BPS when desired BPS is in between to levels
   @param[in] bps Desired baudrate
   @return_gs_error_t
 */
static inline uint32_t get_baudrate_prescaler(uint32_t bps)
{
    uint16_t i;
    for (i = 2; i < 256; i *= 2) {
        if (system_clock_freq_hz / i <= bps) {
            break;
        }
    }

    switch (i) {
        case 2:
            return SPI_BAUDRATEPRESCALER_2;
        case 4:
            return SPI_BAUDRATEPRESCALER_4;
        case 8:
            return SPI_BAUDRATEPRESCALER_8;
        case 16:
            return SPI_BAUDRATEPRESCALER_16;
        case 32:
            return SPI_BAUDRATEPRESCALER_32;
        case 64:
            return SPI_BAUDRATEPRESCALER_64;
        case 128:
            return SPI_BAUDRATEPRESCALER_128;
        default:
            return SPI_BAUDRATEPRESCALER_256;
    }
}

/**
   Configures SPI device to specific slave
   @param[in] slave
   @return_gs_error_t
 */
static gs_error_t gs_configure_spi_device(gs_stm32l4_spi_slave_t * slave, bool disable_during_config)
{
    uint32_t cntr_reg_1 = READ_REG(slave->spi_handle->stm32_spi_handle.Instance->CR1);

    /*Disable SPI module before changing CPOL/CPHA*/
    if (disable_during_config) {
        cntr_reg_1 &= ~(SPI_CR1_SPE_Msk);
        WRITE_REG(slave->spi_handle->stm32_spi_handle.Instance->CR1, cntr_reg_1);
    }

    uint32_t prescaler = get_baudrate_prescaler(slave->config.config.bps);
    if (slave->config.config.bits != 8) {
        return GS_ERROR_NOT_IMPLEMENTED;
    }
    uint32_t msb;
    if (slave->config.config.data_order_msb) {
        msb = SPI_FIRSTBIT_MSB;
    } else {
        msb = SPI_FIRSTBIT_LSB;
    }
    uint32_t cpol_cpha = 0;
    switch (slave->config.config.mode) {
        case GS_SPI_MODE_CPOL0_CPHA0:
            cpol_cpha = SPI_POLARITY_LOW | SPI_PHASE_1EDGE;
            break;
        case GS_SPI_MODE_CPOL1_CPHA0:
            cpol_cpha = SPI_POLARITY_HIGH | SPI_PHASE_1EDGE;
            break;
        case GS_SPI_MODE_CPOL0_CPHA1:
            cpol_cpha = SPI_POLARITY_LOW | SPI_PHASE_2EDGE;
            break;
        case GS_SPI_MODE_CPOL1_CPHA1:
            cpol_cpha = SPI_POLARITY_HIGH | SPI_PHASE_2EDGE;
            break;
    }

    cntr_reg_1 &= ~(SPI_POLARITY_HIGH | SPI_PHASE_2EDGE);
    uint32_t new_reg = ((~SPI_CR1_LSBFIRST_Msk | ~SPI_CR1_BR_Msk) & (~SPI_CR1_BR_Msk) & cntr_reg_1) | msb | prescaler
                    | cpol_cpha;

    WRITE_REG(slave->spi_handle->stm32_spi_handle.Instance->CR1, new_reg);

    /*Re-enable SPI module*/
    if (disable_during_config) {
        new_reg |= (SPI_CR1_SPE_Msk);
        WRITE_REG(slave->spi_handle->stm32_spi_handle.Instance->CR1, new_reg);
    }
    return GS_OK;
}

/**
   Configures SPI slave
   Allocates place in instance array, configures GPIO for slave select and configures SPI device
   @param[in] slave Unique identifier (handle)
   @param[in] slave_conf Slave configuration
   @return_gs_error_t
 */
gs_error_t gs_stm32l4_spi_master_configure_slave(uint8_t slave, const gs_stm32l4_spi_slave_config_t * slave_conf)
{
    GS_CHECK_HANDLE(slave < GS_EMBED_SPI_MAX_NUMBER_OF_SLAVES);
    gs_stm32l4_spi_slave_t * slave_instance = &spi_slaves[slave];
    gs_error_t res = GS_ERROR_HANDLE;
    if (!slave_instance->inuse) {
        memcpy(&slave_instance->config, slave_conf, sizeof(gs_stm32l4_spi_slave_config_t));
        slave_instance->slave_identifier = slave;
        slave_instance->slave_select_cb = slave_conf->slave_select_cb;
        if (slave_instance->config.spi_device <= GS_EMBED_SPI_NUMBER_OF_HANDLES) {
            gs_stm32l4_spi_handle_t * spi_handle = &spi_handles[slave_instance->config.spi_device];
            if (spi_handle) {
                if (spi_handle->inuse) {
                    slave_instance->spi_handle = spi_handle;
                    res = gs_configure_spi_device(slave_instance, true);
                    if (res == GS_OK) {

                        if (slave_conf->slave_select_cb == NULL) {
                            gs_stm32l4_gpio_conf_t conf;
                            conf.mode = GPIO_MODE_OUTPUT_PP;
                            conf.pull = GPIO_PULLUP;
                            conf.speed = GPIO_SPEED_FREQ_HIGH;
                            res = gs_stm32l4_gpio_init(slave_instance->config.slave_select_pin, &conf);
                            if (res == GS_OK) {
                                gs_gpio_set_nc(slave_instance->config.slave_select_pin, true);
                                slave_instance->inuse = true;
                            }
                        } else {
                            slave_instance->inuse = true;
                        }
                    }
                }
            }
        }
    } else {
        res = GS_ERROR_IN_USE;
    }
    return res;
}

/**
   Finds GomSpace handle according to STM handle
   @param[in] spi_handle STM SPI handle
   @return GomSpace handle
 */
static inline gs_stm32l4_spi_handle_t * gs_spi_find_handle(SPI_HandleTypeDef * spi_handle)
{
    for (unsigned int i = 1; i < (GS_EMBED_SPI_NUMBER_OF_HANDLES + 1); i++) {
        if (&spi_handles[i].stm32_spi_handle == spi_handle) {
            return &spi_handles[i];
        }
    }
    return NULL;
}

/**
   Finds SPI slave handle according to slave identifier
   @param[in] slave Identifier assigned in slave configure
   @return SPI slave handle
 */
static inline gs_stm32l4_spi_slave_t * gs_spi_find_slave(uint8_t slave)
{
    if (slave < GS_EMBED_SPI_MAX_NUMBER_OF_SLAVES) {
        gs_stm32l4_spi_slave_t * slave_found = &spi_slaves[slave];
        if (slave_found != NULL) {
            if (slave_found->inuse) {
                return slave_found;
            }
        }
    }
    return NULL;
}

/**
   Selects slave
   Both configuring SPI device and pulling slave select pin low
   Note it remembers last slave, so it is only configuring SPI device if it is a new slave
   @param[in] slave Slave handle
   @return_gs_error_t
 */
static inline gs_error_t gs_spi_select_slave(gs_stm32l4_spi_slave_t * slave)
{
    static gs_stm32l4_spi_slave_t * last_slave;
    static gs_spi_mode_t last_mode_of_spi_device[GS_EMBED_SPI_NUMBER_OF_HANDLES] = { 0 };

    if (last_slave != slave) {
        bool disable_during_config = slave->config.config.mode != last_mode_of_spi_device[slave->config.spi_device];
        last_mode_of_spi_device[slave->config.spi_device] = slave->config.config.mode;
        gs_error_t res = gs_configure_spi_device(slave, disable_during_config);
        if (res != GS_OK) {
            return res;
        }
        last_slave = slave;
    }
    if (slave->slave_select_cb != NULL) {
        slave->slave_select_cb(slave->slave_identifier, true);
    } else {
        gs_gpio_set_nc(slave->config.slave_select_pin, false);
    }
    return GS_OK;
}

/**
   Deselects slave
   Only pulling slave select pin high
   @param[in] slave Slave handle
   @return_gs_error_t
 */
static inline void gs_spi_deselect_slave(gs_stm32l4_spi_slave_t * slave)
{
	if (slave->slave_select_cb != NULL) {
		slave->slave_select_cb(slave->slave_identifier, false);
	} else {
		gs_gpio_set_nc(slave->config.slave_select_pin, true);
	}
}

/**
   Implementation of function declared in libutil <gs/util/drivers/spi/master.h>
 */
gs_error_t gs_spi_master_transaction(uint8_t slave, const void * tx, void * rx, size_t size, int timeout_ms)
{
    gs_spi_master_trans_t t = {tx, rx, size};

    return gs_spi_master_transactions(slave, &t, 1, timeout_ms);
}


gs_error_t gs_spi_master_transactions(uint8_t slave, gs_spi_master_trans_t *trans, size_t count, int timeout_ms)
{
    gs_stm32l4_spi_slave_t * slave_handle = gs_spi_find_slave(slave);
    GS_CHECK_HANDLE(slave_handle != NULL);
    gs_error_t res = gs_sem_wait(slave_handle->spi_handle->lock, timeout_ms);
    if (res == GS_OK) {
        res = gs_spi_select_slave(slave_handle);
        if (res == GS_OK) {
            for (unsigned int t = 0; t < count; t++)
            {
                const bool tx_op = (trans[t].tx && trans[t].size);
                const bool rx_op = (trans[t].rx && trans[t].size);
                if (!(tx_op || rx_op))
                {
                    res = GS_ERROR_ARG;
                    break;
                }
                if (slave_handle->spi_handle->polling_mode) {
                    if (tx_op && rx_op) {
                        res = gs_hal_status_to_error(HAL_SPI_TransmitReceive(&slave_handle->spi_handle->stm32_spi_handle,
                                                                             (uint8_t*) trans[t].tx,
                                                                             (uint8_t *) trans[t].rx,
                                                                             trans[t].size,
                                                                             timeout_ms));
                    } else if (tx_op && !rx_op) {
                        res = gs_hal_status_to_error(HAL_SPI_Transmit(&slave_handle->spi_handle->stm32_spi_handle,
                                                                      (uint8_t*) trans[t].tx,
                                                                      trans[t].size,
                                                                      timeout_ms));

                    } else {
                        res = gs_hal_status_to_error(HAL_SPI_Receive(&slave_handle->spi_handle->stm32_spi_handle,
                                                                     (uint8_t*) trans[t].rx,
                                                                     trans[t].size,
                                                                     timeout_ms));
                    }


                } else {
                    /* Makes sure, that semaphore is taken */
                    while(gs_sem_wait(slave_handle->spi_handle->isr_sync_sem, 0) == GS_OK) {}

                    slave_handle->spi_handle->transfer_status = GS_OK;
                    if (tx_op && rx_op) {
                        res = gs_hal_status_to_error(HAL_SPI_TransmitReceive_DMA(&slave_handle->spi_handle->stm32_spi_handle,
                                                                                (uint8_t*) trans[t].tx,
                                                                                (uint8_t *) trans[t].rx,
                                                                                trans[t].size));
                    } else if (tx_op && !rx_op) {
                        res = gs_hal_status_to_error(HAL_SPI_Transmit_DMA(&slave_handle->spi_handle->stm32_spi_handle,
                                                                        (uint8_t*) trans[t].tx,
                                                                        trans[t].size));

                    } else {
                        res = gs_hal_status_to_error(HAL_SPI_Receive_DMA(&slave_handle->spi_handle->stm32_spi_handle,
                                                                        (uint8_t*) trans[t].rx,
                                                                        trans[t].size));
                    }

                    /* If transmit/receive operation failed - Bail out and return the error code */
                    if (res != GS_OK)
                        break;

                    /* Wait for transfer completion or timeout */
                    if ((gs_sem_wait(slave_handle->spi_handle->isr_sync_sem, timeout_ms) != GS_OK))
                    {
                        slave_handle->spi_handle->transfer_status = GS_ERROR_TIMEOUT;
                        HAL_DMA_Abort(slave_handle->spi_handle->stm32_spi_handle.hdmarx);
                        HAL_DMA_Abort(slave_handle->spi_handle->stm32_spi_handle.hdmatx);
                    }
                    res = slave_handle->spi_handle->transfer_status;
                }

                /* In case a transfer fails, there is no need to continue with the rest - so bail out and return error */
                if (res != GS_OK)
                    break;
            }
            gs_spi_deselect_slave(slave_handle);
        }
        gs_sem_post(slave_handle->spi_handle->lock);
    }
    return res;
}

/*  Callbacks for SPI complete which are called from the DMA ISR */

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    gs_stm32l4_spi_handle_t * handle = gs_spi_find_handle(hspi);
    handle->transfer_status = GS_OK;
    gs_context_switch_t ctx_switch;
    gs_sem_post_isr(handle->isr_sync_sem, &ctx_switch);
    portEND_SWITCHING_ISR(ctx_switch.task_woken);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    gs_stm32l4_spi_handle_t * handle = gs_spi_find_handle(hspi);
    handle->transfer_status = GS_ERROR_IO;
    gs_context_switch_t ctx_switch;
    gs_sem_post_isr(handle->isr_sync_sem, &ctx_switch);
    portEND_SWITCHING_ISR(ctx_switch.task_woken);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    gs_stm32l4_spi_handle_t * handle = gs_spi_find_handle(hspi);
    handle->transfer_status = GS_OK;
    gs_context_switch_t ctx_switch;
    gs_sem_post_isr(handle->isr_sync_sem, &ctx_switch);
    portEND_SWITCHING_ISR(ctx_switch.task_woken);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    gs_stm32l4_spi_handle_t * handle = gs_spi_find_handle(hspi);
    handle->transfer_status = GS_OK;
    gs_context_switch_t ctx_switch;
    gs_sem_post_isr(handle->isr_sync_sem, &ctx_switch);
    portEND_SWITCHING_ISR(ctx_switch.task_woken);
}
