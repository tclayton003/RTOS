/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/a3200dock/drivers/sc16is750.h>
#include <gs/a3200/spi_slave.h>
#include <gs/embed/asf/drivers/spi/master.h>

/* GPIO registers */
#define GS_SC16IS750_GPIO_DIR       0x0A
#define GS_SC16IS750_GPIO_STATE     0x0B

/* USART registers */
#define GS_SC16IS750_LCR

#define GS_SC16IS750_REG_RHR        0x00
#define GS_SC16IS750_REG_THR        0X00
#define GS_SC16IS750_REG_IER        0X01
#define GS_SC16IS750_REG_FCR        0X02
#define GS_SC16IS750_REG_IIR        0X02
#define GS_SC16IS750_REG_LCR        0X03
#define GS_SC16IS750_REG_MCR        0X04
#define GS_SC16IS750_REG_LSR        0X05
#define GS_SC16IS750_REG_MSR        0X06
#define GS_SC16IS750_REG_SPR        0X07
#define GS_SC16IS750_REG_TCR        0X06
#define GS_SC16IS750_REG_TLR        0X07
#define GS_SC16IS750_REG_TXLVL      0X08
#define GS_SC16IS750_REG_RXLVL      0X09
#define GS_SC16IS750_REG_IODIR      0X0A
#define GS_SC16IS750_REG_IOSTATE    0X0B
#define GS_SC16IS750_REG_IOINTENA   0X0C
#define GS_SC16IS750_REG_IOCONTROL  0X0E
#define GS_SC16IS750_REG_EFCR       0X0F
#define GS_SC16IS750_REG_DLL        0x00
#define GS_SC16IS750_REG_DLH        0X01

#define GS_SC16IS750_XTAL_FREQ      14745600

#define SPI_TIMEOUT_MS              1000

static gs_error_t sc16is750_spi_setup(void)
{
    /* Setup using decode mode */
    const gs_spi_chip_t spi_chip = {
        .handle = 0,
        .baudrate = 400000,
        .bits = 8,
        .spi_mode = 0,
        .stay_act = 1,
        .trans_delay = 0,
        .chipselect = 4
    };

    return gs_spi_asf_master_configure_slave(GS_A3200_SPI_SLAVE_ADCS_EXT_GYRO, &spi_chip);
}

gs_error_t sc16is750_init(void)
{
    gs_error_t result = sc16is750_spi_setup();
    if (result == GS_OK) {
        result = sc16is750_gpio_set_all(1);
        if (result == GS_OK) {
            result = sc16is750_gpio_direction(0xff);
        }
    }
    return result;
}

static gs_error_t sc16is750_write_reg(uint8_t reg, uint8_t data)
{
    reg = (reg & 0xf) << 3;
    data = (0x00ff & data);
    gs_spi_master_trans_t transactions[] = {
        {
            .tx = &reg,
            .rx = NULL,
            .size = 1
        },
        {
            .tx = &data,
            .rx = NULL,
            .size = 1
        }
    };

    return gs_spi_master_transactions(GS_A3200_SPI_SLAVE_ADCS_EXT_GYRO, transactions, GS_ARRAY_SIZE(transactions), SPI_TIMEOUT_MS);
}

static gs_error_t sc16is750_read_reg(uint8_t reg, uint8_t *data)
{
    reg = ((reg & 0xf) << 3) | 0x80;
    gs_spi_master_trans_t transactions[] = {
        {
            .tx = &reg,
            .rx = NULL,
            .size = 1
        },
        {
            .tx = NULL,
            .rx = data,
            .size = 1
        }
    };

    return gs_spi_master_transactions(GS_A3200_SPI_SLAVE_ADCS_EXT_GYRO, transactions, GS_ARRAY_SIZE(transactions), SPI_TIMEOUT_MS);
}


/* Scratch Pad Register */
gs_error_t sc16is750_scratchpad_ping(void)
{
    /* Function to write and read data from
     * scratch pad to see if device is responding */
    uint8_t ping_data[5] = {0x12, 0x34, 0x56, 0x78, 0x90};
    uint8_t read_buffer = 0;
    gs_error_t res;
    for (int i=0; i<5; i++) {
        res = sc16is750_write_reg(GS_SC16IS750_REG_SPR, ping_data[i]);
        if (res != GS_OK) {
            break;
        }
        res = sc16is750_read_reg(GS_SC16IS750_REG_SPR, &read_buffer);
        if (res != GS_OK) {
            break;
        }
        if (read_buffer != ping_data[i]) {
            res = GS_ERROR_DATA;
            break;
        }
    }
    return res; // All scratch pad transmissions OK = 0
}

gs_error_t sc16is750_gpio_direction(uint8_t gpio_dir)
{
    return sc16is750_write_reg(GS_SC16IS750_REG_IODIR, gpio_dir);
}

gs_error_t sc16is750_gpio_set(uint8_t pin, bool state)
{
    uint8_t reg_tmp = 0;
    gs_error_t res = sc16is750_read_reg(GS_SC16IS750_REG_IOSTATE, &reg_tmp);
    if (res == GS_OK) {
        if (state) {
            reg_tmp |= (0x01 << pin);
        } else {
            reg_tmp  &= (uint8_t)~(0x01 << pin);
        }
        res = sc16is750_write_reg(GS_SC16IS750_REG_IOSTATE, reg_tmp);
    }
    return res;
}

gs_error_t sc16is750_gpio_set_all(bool state)
{
    return sc16is750_write_reg(GS_SC16IS750_REG_IOSTATE, (state) ? 0xff : 0x00);
}

/* Setup USART */
gs_error_t sc16is750_usart_setbaud(uint32_t baud)
{
    gs_error_t res;
    uint8_t reg = 0;

    uint16_t divisor = 0;
    uint8_t prescaler = 1;

    res = sc16is750_read_reg(GS_SC16IS750_REG_MCR, &reg);
    if (res != GS_OK) {
        return res;
    }

    /* Get prescaler value */
    if (reg & 0x80) {
        prescaler = 4;
    }

    divisor = (uint16_t)((GS_SC16IS750_XTAL_FREQ/prescaler)/(baud*16));

    // Unlock latch register
    res = sc16is750_read_reg(GS_SC16IS750_REG_LCR, &reg);
    if (res != GS_OK) {
        return res;
    }

    reg |= 0x80;

    res = sc16is750_write_reg(GS_SC16IS750_REG_LCR, reg);
    if (res != GS_OK) {
        return res;
    }

    res = sc16is750_write_reg(GS_SC16IS750_REG_DLL,(uint8_t)divisor); //DLL
    if (res != GS_OK) {
        return res;
    }

    res = sc16is750_write_reg(GS_SC16IS750_REG_DLH,(uint8_t)(divisor>>8)); //DHL
    if (res != GS_OK) {
        return res;
    }

    // Lock latch register
    reg &= 0x7F;
    return sc16is750_write_reg(GS_SC16IS750_REG_LCR,reg);
}

gs_error_t sc16is750_usart_reset_fifo(void)
{
    /* Enable FIFO */
    uint8_t fcr_tmp = 0;
    fcr_tmp |= 0x01 << 0; //Enable FIFO
    fcr_tmp |= 0x01 << 1; //Reset RX FIFO
    fcr_tmp |= 0x01 << 2; //Reset TX FIFO
    return sc16is750_write_reg(GS_SC16IS750_REG_FCR, fcr_tmp);
}

gs_error_t sc16is750_usart_conn_conf(void)
{
    /* Configure data configuration */
    uint8_t lcr_tmp = 0x00;
    lcr_tmp |= 0x03; //Set data length to 8
    gs_error_t res = sc16is750_write_reg(GS_SC16IS750_REG_LCR, lcr_tmp);
    if (res != GS_OK) {
        return res;
    }

    /* Reset and enable fifo */
    res = sc16is750_usart_reset_fifo();
    if (res != GS_OK) {
        return res;
    }

    /* Enable transmitter */
    return sc16is750_write_reg(GS_SC16IS750_REG_EFCR, 0x00);

}

/* USART read
 * Return 0 for valid databyte
 * Return -1 for no data in FIFO */
gs_error_t sc16is750_usart_read_byte(uint8_t *databyte)
{
    uint8_t lsr_tmp = 0;

    /* Check FIFO for data */
    gs_error_t res = sc16is750_read_reg(GS_SC16IS750_REG_LSR, &lsr_tmp);
    if (res != GS_OK) {
        return res;
    }

    if (lsr_tmp & 0x01) {
        return sc16is750_read_reg(GS_SC16IS750_REG_RHR, databyte);
    }

    return GS_ERROR_NO_DATA;
}


/* USART write */
gs_error_t sc16is750_usart_write_byte(uint8_t databyte)
{
    return sc16is750_write_reg(GS_SC16IS750_REG_THR,databyte);
}



