/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/util/thread.h>
#include <gs/util/time.h>
#include <gs/util/sem.h>
#include <gs/util/check.h>
#include <gs/util/drivers/i2c/master.h>
#include <gs/util/drivers/i2c/slave.h>
#include <stm32l4xx_hal.h>
#include <math.h>
#include "../common.h"

#ifdef I2C4
#define GS_EMBED_I2C_NUMBER_OF_HANDLES 4
#else
#define GS_EMBED_I2C_NUMBER_OF_HANDLES 3
#endif

typedef struct buffer {
    //! Pointer to buffer
    uint8_t *buffer;
    //! Buffer index
    uint8_t index;
    //! Length of buffer
    uint16_t length;
} buffer_t;

typedef struct gs_stm32l4_i2c_handle {
    //! Device number
    uint8_t device;
    //! Bool true if active
    bool inuse;
    //! Lock
    gs_sem_t lock;
    //! Cross platform configurations
    gs_i2c_bus_config_t config;
    //! Slave/multimaster RX callback (can be null if only master mode is used)
    gs_i2c_slave_receive_t slave_rx_callback;
    //! Slave/multimaster RX buffer get callback (can be null if only master mode is used)
    gs_i2c_slave_get_rx_buf_t slave_get_rx_buf_callback;
    //! Master RX buffer
    buffer_t master_rx_buf;
    //! Slave RX buffer
    buffer_t slave_rx_buf;
    //! Master TX buffer
    buffer_t master_tx_buf;
    //! Slave TX buffer
    buffer_t slave_tx_buf;
    //! Semaphore indicating completion of transmit/receive
    gs_sem_t master_tx_isr_sync_sem;
    //! Transfer status
    gs_error_t master_transfer_status;
    //! Slave TX flag, true if slave is requested to TX
    bool slave_tx;
    //! Slave RX flag, true if slave is requested to RX
    bool slave_rx;
    //! Device instance
    I2C_TypeDef * instance;
} gs_stm32l4_i2c_handle_t;

static gs_stm32l4_i2c_handle_t i2c_buses[GS_EMBED_I2C_NUMBER_OF_HANDLES + 1];

static gs_error_t calculate_timing_register(size_t speed, uint32_t * register_val)
{
    GS_CHECK_RANGE(speed >= 100000 && speed <= 400000);
    *register_val = 0;
    float clock_freq_mhz = (float)HAL_RCC_GetPCLK1Freq() / 1000000;
    float scl_high_us, scl_low_us, sda_del_us, scl_del_us;
    if (speed < 250000) {
        float scl_period_minus = 10 - ((float)1000000 / speed);
        scl_low_us = 5 - (scl_period_minus / 2);
        scl_high_us = 4 - (scl_period_minus / 2);
        scl_del_us = 1.25;
        sda_del_us = 0.5;
    } else {
        float scl_period_plus = ((float)1000000 / speed) - 2.5;
        scl_low_us = 1.25 + (scl_period_plus / 2);
        scl_high_us = 0.5 + (scl_period_plus / 2);
        scl_del_us = 0.5;
        sda_del_us = 0.375;
    }
    float scl_us;
    unsigned int prescaler;
    float scl_high_count, scl_low_count, scl_del_count, sda_del_count;
    for(prescaler = 1; prescaler <= 17; prescaler++) {
        if (prescaler == 17) {
            return GS_ERROR_ARG;
        }
        scl_us = 1 / (clock_freq_mhz / prescaler);
        scl_high_count = scl_high_us / scl_us;
        scl_low_count = scl_low_us / scl_us;
        sda_del_count = sda_del_us / scl_us;
        scl_del_count = scl_del_us / scl_us;
        if (scl_del_count <= 16 && scl_del_count >= 1 && sda_del_count <= 15 && sda_del_count >= 1) {
            break;
        }
    }
    *register_val |= (prescaler - 1) << 28;
    *register_val |= (int)(ceil(scl_del_count) - 1) << 20;
    *register_val |= (int)ceil(sda_del_count) << 16;
    *register_val |= (int)(ceil(scl_high_count) - 1) << 8;
    *register_val |= (int)(ceil(scl_low_count) - 1) << 0;
    return GS_OK;
}

gs_error_t gs_stm32l4_i2c_init(uint8_t device, const gs_i2c_bus_config_t * config)
{
    GS_CHECK_HANDLE((device >= 1) && (device <= GS_EMBED_I2C_NUMBER_OF_HANDLES));
    GS_CHECK_ARG(config->data_order_msb);
    GS_CHECK_ARG(config->addrbits == 7);
    GS_CHECK_ARG(config->mode == GS_I2C_MASTER ||config->mode == GS_I2C_MULTI_MASTER ||config->mode == GS_I2C_SLAVE);
    gs_stm32l4_i2c_handle_t * handle = &i2c_buses[device];

    if (handle->inuse) {
        return GS_ERROR_IN_USE;
    }

    IRQn_Type I2Cx_EV_IRQn, I2Cx_ER_IRQn;
    RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;
    switch (device) {
        case 1:
            /* Enable I2C1 clock */
            __HAL_RCC_I2C1_CLK_ENABLE();
            RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
            RCC_PeriphCLKInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
            HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

            /* Link device */
            handle->instance = I2C1;
            I2Cx_EV_IRQn = I2C1_EV_IRQn;
            I2Cx_ER_IRQn = I2C1_ER_IRQn;
            break;
        case 2:
            /* Enable I2C2 clock */
            __HAL_RCC_I2C2_CLK_ENABLE();
            RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
            RCC_PeriphCLKInitStruct.I2c2ClockSelection = RCC_I2C2CLKSOURCE_SYSCLK;
            HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

            /* Link device */
            handle->instance = I2C2;
            I2Cx_EV_IRQn = I2C2_EV_IRQn;
            I2Cx_ER_IRQn = I2C2_ER_IRQn;
            break;
        case 3:
            /* Enable I2C3 clock */
            __HAL_RCC_I2C3_CLK_ENABLE();
            RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C3;
            RCC_PeriphCLKInitStruct.I2c3ClockSelection = RCC_I2C3CLKSOURCE_SYSCLK;
            HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

            /* Link device */
            handle->instance = I2C3;
            I2Cx_EV_IRQn = I2C3_EV_IRQn;
            I2Cx_ER_IRQn = I2C3_ER_IRQn;
            break;
#if(GS_EMBED_I2C_NUMBER_OF_HANDLES == 4)
        case 4:
            /* Enable I2C4 clock */
            __HAL_RCC_I2C4_CLK_ENABLE();
            RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C4;
            RCC_PeriphCLKInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_SYSCLK;
            HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

            /* Link device */
            handle->instance = I2C4;
            I2Cx_EV_IRQn = I2C4_EV_IRQn;
            I2Cx_ER_IRQn = I2C4_ER_IRQn;
            break;
#endif
        default:
            return GS_ERROR_ARG;
    }


    uint32_t timing_reg;
    gs_error_t res = calculate_timing_register(config->bps, &timing_reg);
    if (res != GS_OK) {
        return res;
    }

    /* Configure speed */
    handle->instance->TIMINGR = timing_reg & 0xF0FFFFFF;
    /* Configure I2C Own Address */
    handle->instance->OAR1 = (I2C_OAR1_OA1EN | config->addr << 1);
    /* Enable wake up from stop mode */
    handle->instance->CR1 |= I2C_CR1_WUPEN;
    /* Enable error interrupts, NACK interrupt and STOP interrupt */
    handle->instance->CR1 |= I2C_CR1_ERRIE | I2C_CR1_NACKIE | I2C_CR1_STOPIE;
    /* Enable */
    handle->instance->CR1 |= I2C_CR1_PE;


    res = gs_sem_create(1, &i2c_buses[device].lock);
    if (res != GS_OK) {
        return res;
    }

    res = gs_sem_create(0, &i2c_buses[device].master_tx_isr_sync_sem);
    if (res != GS_OK) {
        return res;
    }

    /* NVIC for I2Cx */
    HAL_NVIC_SetPriority(I2Cx_ER_IRQn, GS_STM32L4_DEFAULT_ISR_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(I2Cx_ER_IRQn);
    HAL_NVIC_SetPriority(I2Cx_EV_IRQn, GS_STM32L4_DEFAULT_ISR_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(I2Cx_EV_IRQn);

    i2c_buses[device].inuse = true;
    i2c_buses[device].config = *config;
    i2c_buses[device].device = device;

    return GS_OK;
}

gs_error_t gs_i2c_slave_set_rx(uint8_t device, gs_i2c_slave_receive_t rx)
{
    GS_CHECK_HANDLE((device >= 1) && (device <= GS_EMBED_I2C_NUMBER_OF_HANDLES));
    gs_stm32l4_i2c_handle_t * handle = &i2c_buses[device];
    if (handle->inuse) {
        if ((handle->config.mode == GS_I2C_MULTI_MASTER) || (handle->config.mode == GS_I2C_SLAVE)){
            handle->slave_rx_callback = rx;
            return GS_OK;
        }
    }
    return GS_ERROR_HANDLE;
}

gs_error_t gs_i2c_slave_set_get_rx_buf(uint8_t device, gs_i2c_slave_get_rx_buf_t get_rx_buf, size_t buf_length)
{
    GS_CHECK_HANDLE((device >= 1) && (device <= GS_EMBED_I2C_NUMBER_OF_HANDLES));
    gs_stm32l4_i2c_handle_t * handle = &i2c_buses[device];
    if (handle->inuse) {
        if ((handle->config.mode == GS_I2C_MULTI_MASTER) || (handle->config.mode == GS_I2C_SLAVE)){
            handle->slave_get_rx_buf_callback = get_rx_buf;
            handle->slave_rx_buf.length = buf_length;
            return GS_OK;
        }
    }
    return GS_ERROR_HANDLE;
}

gs_error_t gs_i2c_slave_set_response(uint8_t device, const uint8_t * tx, size_t size)
{
    GS_CHECK_HANDLE((device >= 1) && (device <= GS_EMBED_I2C_NUMBER_OF_HANDLES));
    GS_CHECK_ARG(size <= 255);
    gs_stm32l4_i2c_handle_t * handle = &i2c_buses[device];
    if (handle->inuse) {
        if (handle->config.mode == GS_I2C_SLAVE) {
            handle->slave_tx_buf.buffer = (void *)tx;
            handle->slave_tx_buf.length = size;
            handle->slave_tx_buf.index = 0;
            /* Flush TX register */
            handle->instance->ISR |= I2C_ISR_TXE;
            /* Set first byte to be transmitted */
            handle->instance->TXDR = handle->slave_tx_buf.buffer[handle->slave_tx_buf.index];
            return GS_OK;
        }
    }
    return GS_ERROR_HANDLE;
}

gs_error_t gs_i2c_slave_start(uint8_t device)
{
    GS_CHECK_HANDLE((device >= 1) && (device <= GS_EMBED_I2C_NUMBER_OF_HANDLES));
    gs_stm32l4_i2c_handle_t * handle = &i2c_buses[device];
    GS_CHECK_HANDLE(handle->slave_get_rx_buf_callback != NULL);
    GS_CHECK_HANDLE(handle->slave_rx_callback != NULL);

    /* Enable address match interrupt*/
    handle->instance->CR1 |= I2C_CR1_ADDRIE;

    return GS_OK;
}

#define MIN_TIMEOUT     1000

gs_error_t gs_i2c_master_transaction(uint8_t device, uint8_t addr, const void * tx, size_t txlen, void * rx,
                                     size_t rxlen, int timeout_ms)
{
    GS_CHECK_HANDLE((device >= 1) && (device <= GS_EMBED_I2C_NUMBER_OF_HANDLES));
    bool tx_flag = txlen && tx;
    bool rx_flag = rxlen && rx;
    GS_CHECK_ARG((txlen <= 255) && (rxlen <= 255));
    GS_CHECK_ARG(tx_flag || rx_flag);
    uint8_t transfer_length = tx_flag ? txlen : rxlen;
    gs_stm32l4_i2c_handle_t * handle = &i2c_buses[device];
    GS_CHECK_HANDLE(handle->inuse);
    GS_CHECK_HANDLE(handle->config.mode == GS_I2C_MASTER || handle->config.mode == GS_I2C_MULTI_MASTER);

    if(timeout_ms < MIN_TIMEOUT) {
        timeout_ms = MIN_TIMEOUT;
    }

    gs_error_t res = gs_sem_wait(handle->lock, timeout_ms);
    if (res != GS_OK) {
        return res;
    }

    /* Makes sure, that semaphore is taken */
    while(gs_sem_wait(handle->master_tx_isr_sync_sem, 0) == GS_OK) {}

    handle->master_transfer_status = GS_ERROR_AGAIN;
    uint32_t time = gs_time_rel_ms();
    while((res == GS_OK) && (handle->master_transfer_status == GS_ERROR_AGAIN)) {
        /* Setup buffers*/
        if (tx_flag) {
            handle->master_tx_buf.buffer = (uint8_t *)tx;
            handle->master_tx_buf.length = txlen;
            handle->master_tx_buf.index = 0;
        }
        if (rx_flag) {
            handle->master_rx_buf.buffer = (uint8_t *)rx;
            handle->master_rx_buf.length = rxlen;
            handle->master_rx_buf.index = 0;
        }

        if (tx_flag) {
            /* If tx, set first byte to be transmitted */
            handle->instance->TXDR = handle->master_tx_buf.buffer[handle->master_tx_buf.index];
            handle->master_tx_buf.index++;
            handle->instance->CR1 |= I2C_CR1_TXIE; // Enable TX register empty
        } else {
            handle->instance->CR1 |= I2C_CR1_RXIE; // Enable RX register empty
        }

        handle->instance->CR1 |= I2C_CR1_TCIE; // Enable transfer complete flag
        handle->instance->CR2 &= ~(I2C_CR2_NBYTES_Msk | I2C_CR2_SADD_Msk); // Resetting address and length
        handle->instance->CR2 |= (transfer_length & 0x000000FF) << I2C_CR2_NBYTES_Pos; // Setting bytes to be sent
        handle->instance->CR2 |= (addr & 0x0000007F) << (I2C_CR2_SADD_Pos + 1); // Setting receiver address

        if(tx_flag) {
            handle->instance->CR2 &= ~I2C_CR2_RD_WRN; // Setting write direction
        } else {
            handle->instance->CR2 |= I2C_CR2_RD_WRN; // Setting read direction
        }
        handle->instance->CR2 |= I2C_CR2_START; // Sending start condition

        res = gs_sem_wait(handle->master_tx_isr_sync_sem, timeout_ms);

        if ((res == GS_OK) && (handle->master_transfer_status == GS_ERROR_AGAIN)) {
            uint32_t time_diff = gs_time_diff_ms(time, gs_time_rel_ms());
            if (time_diff >= (uint32_t)timeout_ms) {
                res = GS_ERROR_TIMEOUT;
            }
        }
    }
    if (res == GS_OK) {
        res = handle->master_transfer_status;
    }
    gs_sem_post(handle->lock);
    return res;
}

static void slave_rx_complete(gs_stm32l4_i2c_handle_t * handle, gs_context_switch_t * ctx_switch)
{
    if (handle->slave_rx_callback && handle->slave_rx_buf.buffer) {
        handle->slave_rx_callback(handle->device, handle->slave_rx_buf.buffer, handle->slave_rx_buf.index, ctx_switch);
    }
    handle->slave_rx_buf.index = 0;
    handle->slave_rx = false;
}

static void slave_tx_complete(gs_stm32l4_i2c_handle_t * handle)
{
    handle->slave_tx = false;
    handle->slave_tx_buf.index = 0;
}

static void master_transfer_complete(gs_stm32l4_i2c_handle_t * handle, gs_context_switch_t * ctx_switch)
{
    /* Check entire buffer has been transferred */
    if ((handle->master_rx_buf.index == handle->master_rx_buf.length) &&
        (handle->master_tx_buf.index == handle->master_tx_buf.length)){
        handle->master_transfer_status = GS_OK;
    } else {
        handle->master_transfer_status = GS_ERROR_IO;
    }
    handle->master_rx_buf.index = 0;
    handle->master_rx_buf.length = 0;
    handle->master_rx_buf.buffer = NULL;
    handle->master_tx_buf.index = 0;
    handle->master_tx_buf.length = 0;
    handle->master_tx_buf.buffer = NULL;
    gs_sem_post_isr(handle->master_tx_isr_sync_sem, ctx_switch);
}

static void receive_byte(gs_stm32l4_i2c_handle_t * handle)
{
    /* Check RX buffer */
    buffer_t * buf = handle->slave_rx ? &handle->slave_rx_buf : &handle->master_rx_buf;
    if ((buf->buffer != NULL) && (buf->index < buf->length)) {
        buf->buffer[buf->index] = (uint8_t) handle->instance->RXDR;
    } else {
        asm volatile ("" : : "r" (handle->instance->RXDR)); // Reading RXDR without storing it
    }
    buf->index++;
}

static void transmit_byte(gs_stm32l4_i2c_handle_t * handle)
{
    /* Check TX buffer */
    buffer_t * buf = handle->slave_tx ? &handle->slave_tx_buf : &handle->master_tx_buf;
    if ((buf->buffer != NULL) && (buf->index < buf->length)) {
        handle->instance->TXDR = buf->buffer[buf->index];
    } else {
        handle->instance->TXDR = 0;
    }
    buf->index++;
}

static void reset_handle(gs_stm32l4_i2c_handle_t * handle)
{
    /* Disable device */
    handle->instance->CR1 &= ~I2C_CR1_PE;
    handle->slave_tx = false;
    handle->slave_rx = false;
    handle->instance->CR1 &= ~(I2C_CR1_TXIE | I2C_CR1_RXIE); // Disable TX and RX empty interrupt
    handle->master_tx_buf.index = 0;
    handle->slave_tx_buf.index = 0;
    handle->master_rx_buf.index = 0;
    handle->slave_rx_buf.index = 0;
    /* Enable again
     * note, at least three APB clock cycles must pass
     * this is usually just system clock so we are fine */
    handle->instance->CR1 |= I2C_CR1_PE;
}

/**
   This function handles I2C event interrupt request.
 */
static void I2C_EV_IRQHandler(gs_stm32l4_i2c_handle_t * handle)
{
    gs_context_switch_t ctx_switch = GS_CONTEXT_SWITCH_INIT;
    uint32_t ISR = handle->instance->ISR;
    /* Busy is set between START and STOP conditions */
    if ((ISR & I2C_ISR_BUSY) && !(ISR & I2C_ISR_STOPF) && !(ISR & I2C_ISR_NACKF)) {
        /* Slave mode initiated */
        if ((ISR & I2C_ISR_ADDR)&& (handle->instance->CR1 & I2C_CR1_ADDRIE)) {
            /* If repeated start after a RX operation */
            if (handle->slave_rx_buf.index != 0) {
                slave_rx_complete(handle, &ctx_switch);
            }
            /* If repeated start after a TX operation */
            if (handle->slave_tx_buf.index != 0) {
                slave_tx_complete(handle);
            }
            if (handle->instance->CR2 & I2C_CR2_START) {
                handle->master_transfer_status = GS_ERROR_AGAIN; // Set tx_error, in case it happens during transmit
                reset_handle(handle);
                gs_sem_post_isr(handle->master_tx_isr_sync_sem, &ctx_switch);
            }
            handle->instance->ICR |= I2C_ICR_ADDRCF; // Clear address match flag
            handle->instance->CR2 &= ~I2C_CR2_START; // Setting start bit low as an error exists on the MCU - see errata sheet

            /* Slave RX initiated */
            if (!(ISR & I2C_ISR_DIR)) {
                handle->slave_rx = true;
                handle->instance->CR1 |= I2C_CR1_RXIE; // Enable RX register not empty flag
                handle->slave_rx_buf.index = 0;
                handle->slave_rx_buf.buffer = handle->slave_get_rx_buf_callback(handle->device);
            }
            /* Slave TX initiated */
            else {
                handle->slave_tx = true;
                handle->instance->CR1 |= I2C_CR1_TXIE; // Enable TX register empty flag
                handle->slave_tx_buf.index++;
            }
        }
        /* Slave or master received data */
        else if ((ISR & I2C_ISR_RXNE) && (handle->instance->CR1 & I2C_CR1_RXIE)) {
            receive_byte(handle);
        }
        /* Slave or master transmit data */
        else if ((ISR & I2C_ISR_TXIS) && (handle->instance->CR1 & I2C_CR1_TXIE)) {
            transmit_byte(handle);
        }
        /* Master transfer complete */
        else if ((ISR & I2C_ISR_TC) && (handle->instance->CR1 & I2C_CR1_TCIE)) {
            /* Master TX complete and repeated start for master RX wanted */
            if (!(handle->instance->CR2 & I2C_CR2_RD_WRN) && handle->master_rx_buf.length) {
                handle->instance->CR1 &= ~I2C_CR1_TXIE; // Disable TX register empty interrupt
                handle->instance->CR1 |= I2C_CR1_RXIE; // Enable RX register empty interrupt
                handle->instance->CR2 &= ~I2C_CR2_NBYTES_Msk; // Resetting length
                handle->instance->CR2 |= (handle->master_rx_buf.length & 0x000000FF) << I2C_CR2_NBYTES_Pos; // Setting bytes to be read
                handle->instance->CR2 |= I2C_CR2_RD_WRN; // Setting read direction
                handle->instance->CR2 |= I2C_CR2_START; // Sending start condition
            }
            /* Master transfer complete (either RX or TX) */
            else {
                master_transfer_complete(handle, &ctx_switch);
                handle->instance->CR1 &= ~(I2C_CR1_TXIE | I2C_CR1_RXIE); // Disable TX and RX register empty interrupts
                handle->instance->CR2 |= I2C_CR2_STOP; // Send stop
            }
        }
    }
    /* Stop condition received */
    else if ((ISR & I2C_ISR_STOPF) && (handle->instance->CR1 & I2C_CR1_STOPIE)) {
        /* Slave received data */
        if (handle->slave_rx_buf.index != 0) {
            /* Make sure last byte is received - test showed a stop condition can be received even though last byte is not read */
            if ((ISR & I2C_ISR_RXNE) && (handle->instance->CR1 & I2C_CR1_RXIE)) {
                receive_byte(handle);
            }
            slave_rx_complete(handle, &ctx_switch);
            handle->instance->CR1 &= ~I2C_CR1_RXIE;
        }
        /* Slave sent data */
        else if (handle->slave_tx_buf.index != 0) {
            slave_tx_complete(handle);
        }
        handle->instance->ICR |= I2C_ICR_STOPCF; // Clear stop flag
    }
    /* NACK received */
    else if ((ISR & I2C_ISR_NACKF) && (handle->instance->CR1 & I2C_CR1_NACKIE)) {
        if(!handle->slave_tx) {
            handle->master_transfer_status = GS_ERROR_IO; // Set tx_error, in case it happens during transmit
            reset_handle(handle);
            gs_sem_post_isr(handle->master_tx_isr_sync_sem, &ctx_switch);
        }
        handle->instance->ICR |= I2C_ICR_NACKCF; // Clear not acknowledge flag
    }
    portEND_SWITCHING_ISR(ctx_switch.task_woken);
}

/**
   This function handles I2C error interrupt request.
 */
static void I2C_ER_IRQHandler(gs_stm32l4_i2c_handle_t * handle)
{
    gs_context_switch_t ctx_switch = GS_CONTEXT_SWITCH_INIT;
    uint32_t ISR = handle->instance->ISR;

    /* Arbitration lost */
    if ((ISR & I2C_ISR_ARLO) && (handle->instance->CR1 & I2C_CR1_ERRIE)) {
        handle->instance->ICR |= I2C_ICR_ARLOCF; // Clear arbitration los flag and not acknowledge flag
        handle->master_transfer_status = GS_ERROR_IO; // Set tx_error, in case it happens during transmit
        reset_handle(handle);
        gs_sem_post_isr(handle->master_tx_isr_sync_sem, &ctx_switch);
    }
    /* Bus error */
    if ((ISR & I2C_ISR_BERR) && (handle->instance->CR1 & I2C_CR1_ERRIE)) {
        handle->master_transfer_status = GS_ERROR_IO; // Set tx_error, in case it happens during transmit
        reset_handle(handle);
        handle->instance->ICR |= I2C_ICR_BERRCF; // Clear bus error flag and not acknowledge flag
        gs_sem_post_isr(handle->master_tx_isr_sync_sem, &ctx_switch);
    }
    /* Overrun error (slave mode) */
    if ((ISR & I2C_ISR_OVR) && (handle->instance->CR1 & I2C_CR1_ERRIE)) {
        handle->instance->ICR |= I2C_ICR_OVRCF; // Clear overrun flag and not acknowledge flag
    }
    portEND_SWITCHING_ISR(ctx_switch.task_woken);
}

void I2C1_EV_IRQHandler(void)
{
    I2C_EV_IRQHandler(&i2c_buses[1]);
}

/**
   This function handles I2C1 error interrupt request.
 */
void I2C1_ER_IRQHandler(void)
{
    I2C_ER_IRQHandler(&i2c_buses[1]);
}

/**
   This function handles I2C2 event interrupt request.
 */
void I2C2_EV_IRQHandler(void)
{
    I2C_EV_IRQHandler(&i2c_buses[2]);
}

/**
   This function handles I2C2 error interrupt request.
 */
void I2C2_ER_IRQHandler(void)
{
    I2C_ER_IRQHandler(&i2c_buses[2]);
}

/**
   This function handles I2C3 event interrupt request.
 */
void I2C3_EV_IRQHandler(void)
{
    I2C_EV_IRQHandler(&i2c_buses[3]);
}

/**
   This function handles I2C3 error interrupt request.
 */
void I2C3_ER_IRQHandler(void)
{
    I2C_ER_IRQHandler(&i2c_buses[3]);
}

#if(GS_EMBED_I2C_NUMBER_OF_HANDLES == 4)

/**
   This function handles I2C4 event interrupt request.
 */
void I2C4_EV_IRQHandler(void)
{
    I2C_EV_IRQHandler(&i2c_buses[4]);
}

/**
   This function handles I2C4 error interrupt request.
 */
void I2C4_ER_IRQHandler(void)
{
    I2C_ER_IRQHandler(&i2c_buses[4]);
}
#endif
