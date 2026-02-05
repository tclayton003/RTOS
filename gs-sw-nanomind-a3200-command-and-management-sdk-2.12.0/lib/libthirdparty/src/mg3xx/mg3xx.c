/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   SRC file for IMU M-G3xx driver (Gyroscope, accelerometer and temperature sensor)
*/

#include <stdlib.h>
#include <gs/util/error.h>
#include <gs/util/check.h>
#include <gs/util/mutex.h>
#include <gs/util/time.h>
#include <gs/util/byteorder.h>
#include <gs/util/gosh/command.h>
#include <gs/util/drivers/spi/master.h>
#include <gs/thirdparty/mg3xx/mg3xx.h>

/**
   SPI timeout for every SPI driver call.
 */
#define GS_MG3xx_SPI_TIMEOUT_MS         500

/**
   Internal m-g3xx mode.
   @note Modes corresponds to mode register bits.
 */
typedef enum gs_mg3xx_model {
    /**
       M-G364
     */
    GS_MG364 = 364,
    /**
       M-G365
     */
    GS_MG365 = 365,
    /**
       M-G370
     */
    GS_MG370 = 370,
} gs_mg3xx_model_t ;

/**
   Internal m-g3xx mode.
   @note Modes corresponds to mode register bits.
 */
typedef enum gs_mg3xx_mode {
    /**
       Sample mode
     */
    GS_MG3XX_SAMPLE_MODE = 0x01,
    /**
       Configuration mode
     */
    GS_MG3XX_CONF_MODE = 0x02,
} gs_mg3xx_mode_t ;

typedef struct {
    /**
       Register address (low byte)
     */
    uint8_t addr;
    /**
       Register window (0 / 1)
     */
    uint8_t window;
} mg3xx_reg_t;

typedef struct gs_mg3xx_handle {
    /**
       SPI slave
     */
    uint8_t slave;
    /**
       Gyro model (ex. M-G364)
     */
    gs_mg3xx_model_t model;
    /**
       Active window (0 / 1)
     */
    uint8_t active_window;
    /**
       In use status (true if in use)
     */
    bool inuse;
    /**
       Lock
     */
    gs_mutex_t lock;
    /**
       Current sample mode
     */
    gs_gyro_sample_mode_t sample_mode;
    /**
       Configured filter
     */
    gs_mg3xx_filter_t filter;
} gs_mg3xx_handle_t;

/**
   Burst packet size (dependent on configuration of BURST_CTRL1 and BURST_CTRL2)
 */
#define BURST_PACKET_SIZE 16

/**
   Address for window control register, which do not consist in a specific window it self.
 */
#define WIN_CTRL_ADDR                   0x7E

/**
   Registers with window zero.
   @note Only used registers are written.
   */
#define BURST           (mg3xx_reg_t){0x00, 0}
#define MODE_CTRL       (mg3xx_reg_t){0x02, 0}
#define DIAG_STAT       (mg3xx_reg_t){0x04, 0}
#define FLAG            (mg3xx_reg_t){0x06, 0}
#define TEMP_HIGH       (mg3xx_reg_t){0x0E, 0}
#define XGYRO_HIGH      (mg3xx_reg_t){0x12, 0}
#define YGYRO_HIGH      (mg3xx_reg_t){0x16, 0}
#define ZGYRO_HIGH      (mg3xx_reg_t){0x1A, 0}

/**
   Registers with window one.
   @note Only used registers are written.
   */
#define SIG_CTRL        (mg3xx_reg_t){0x00, 1}
#define MSC_CTRL        (mg3xx_reg_t){0x02, 1}
#define SMPL_CTRL       (mg3xx_reg_t){0x04, 1}
#define FILTER_CTRL     (mg3xx_reg_t){0x06, 1}
#define UART_CTRL       (mg3xx_reg_t){0x08, 1}
#define GLOB_CMD        (mg3xx_reg_t){0x0A, 1}
#define BURST_CTRL1     (mg3xx_reg_t){0x0C, 1}
#define BURST_CTRL2     (mg3xx_reg_t){0x0E, 1}
#define PROD_ID1        (mg3xx_reg_t){0x6A, 1}
#define PROD_ID2        (mg3xx_reg_t){0x6C, 1}
#define PROD_ID3        (mg3xx_reg_t){0x6E, 1}
#define PROD_ID4        (mg3xx_reg_t){0x70, 1}
#define VERSION         (mg3xx_reg_t){0x72, 1}
#define SERIAL_NUM1     (mg3xx_reg_t){0x74, 1}
#define SERIAL_NUM2     (mg3xx_reg_t){0x76, 1}
#define SERIAL_NUM3     (mg3xx_reg_t){0x78, 1}
#define SERIAL_NUM4     (mg3xx_reg_t){0x7A, 1}

/**
   Const used to for TX and read situations.
 */
static const uint16_t uint16_zero = 0;

/**
   Number of handles.
   @note This is also a limit to the highest handle number a device can be initialized with.
 */
#define GS_MG3XX_MAX_NUMBER_HANDLES 2

/**
   Handles for m-g3xx devices.
 */
static gs_mg3xx_handle_t handles[GS_MG3XX_MAX_NUMBER_HANDLES];

/**
   Macros for write and read registers commands.

   write_bit [15] | reg_address [14:8] | value [7 : 0]
 */
#define MG3XX_REG_READ_COMMAND(addr)            util_htons(addr << 8)
#define MG3XX_REG_WRITE_COMMAND(addr, value)    util_htons(0x8000 | (addr << 8) | value)

/**
   Write value to a 8 bit register.
   @note Address needs to be specific (also odd addresses).
 */
static inline gs_error_t write_reg(uint8_t slave, uint8_t reg_addr, uint8_t value)
{
    uint16_t cmd = MG3XX_REG_WRITE_COMMAND(reg_addr, value);
    gs_error_t res = gs_spi_master_transaction(slave, &cmd, NULL, sizeof(cmd), GS_MG3xx_SPI_TIMEOUT_MS);

    /* Takes time for device to get ready for next command - see t_stall in data sheet.
       Ideally we sleep only 30 uS, but FreeRTOS only allow sleep of 1 tick = 1 mS */
    gs_time_sleep_ms(1);

    return res;
}

static inline gs_error_t load_reg(uint8_t slave, uint8_t reg_addr, uint16_t * value)
{
    uint16_t cmd = MG3XX_REG_READ_COMMAND(reg_addr);
    gs_error_t res = gs_spi_master_transaction(slave, &cmd, NULL, sizeof(cmd), GS_MG3xx_SPI_TIMEOUT_MS);
    if (res != GS_OK) {
        return res;
    }

    /* Takes time for device to get data ready - see t_stall in data sheet.
       Ideally we sleep only 30 uS, but FreeRTOS only allow sleep of 1 tick = 1 mS */
    gs_time_sleep_ms(1);

    res = gs_spi_master_transaction(slave, &uint16_zero, value, sizeof(cmd), GS_MG3xx_SPI_TIMEOUT_MS);
    *value = util_ntohs(*value);

    /* Takes time for device to get ready for next command - see t_stall in data sheet.
       Ideally we sleep only 30 uS, but FreeRTOS only allow sleep of 1 tick = 1 mS */
    gs_time_sleep_ms(1);
    return res;
}

/**
   Set the window register to either window 0 or window 1.
 */
static inline gs_error_t set_window(gs_mg3xx_handle_t * handle, uint8_t window) {
    if (handle->active_window != window) {
        gs_error_t res = write_reg(handle->slave, WIN_CTRL_ADDR, window);
        if (res != GS_OK) {
            return res;
        }
        handle->active_window = window;
    }
    return GS_OK;
}

/**
   Set register.
   This function sets the window and writes 8 bit value to a 16 bit register in either MSB or LSB.
 */
static gs_error_t set_reg(gs_mg3xx_handle_t * handle, mg3xx_reg_t reg, bool msb, bool read_back, uint8_t value)
{
    gs_error_t res = set_window(handle, reg.window);
    if (res != GS_OK) {
        return res;
    }

    uint8_t addr = msb ? (reg.addr + 1) : reg.addr;

    res = write_reg(handle->slave, addr, value);
    if (!read_back) {
        return res;
    }

    uint16_t reg_new_val;
    res= load_reg(handle->slave, reg.addr, &reg_new_val);
    if (res != GS_OK) {
        return res;
    }
    if ((uint8_t)((reg_new_val >> (msb ? 8 : 0)) & 0x00FF) != value) {
        return GS_ERROR_IO;
    }
    return GS_OK;
}

/**
   Read 16 bit register.
   This function sets the window and reads the 16 bit value.
 */
static gs_error_t read_reg(gs_mg3xx_handle_t * handle, mg3xx_reg_t reg, uint16_t * value)
{
    gs_error_t res = set_window(handle, reg.window);
    if (res != GS_OK) {
        return res;
    }

    return load_reg(handle->slave, reg.addr, value);
}

/**
   Change mode.
   Changes the internal mode on the m-g3xx device.
   @note It may take some time to change mode.
 */
static gs_error_t change_mode(gs_mg3xx_handle_t * handle, gs_mg3xx_mode_t mode) {
    gs_error_t res = set_reg(handle, MODE_CTRL, true, false, (uint8_t)mode);
    if (res != GS_OK) {
        return res;
    }
    uint16_t reg;
    for (int i = 0; i < 3; i++) {
        res = read_reg(handle, MODE_CTRL, &reg);
        if (res != GS_OK) {
            return res;
        }
        if ((reg & 0x0300) == 0) {
            return GS_OK;
        }
        gs_time_sleep_ms(10);
    }
    log_error("m-g3xx: Change mode error - MODE_CTRL: %04x", reg);
    return GS_ERROR_IO;
}

/**
   Waits for device to get ready.
 */
static gs_error_t wait_for_ready(gs_mg3xx_handle_t * handle)
{
    uint16_t glob_cmd_reg;
    gs_error_t res;
    for(int i = 0; i < 10; i++) {
        res = read_reg(handle, GLOB_CMD, &glob_cmd_reg);
        if (res != GS_OK) {
            return res;
        }
        if (!(glob_cmd_reg & 0x0400)) {
           return GS_OK;
        }
        gs_time_sleep_ms(100);
    }
    return GS_ERROR_TIMEOUT;
}

/**
   Internal self test
 */
static gs_error_t mg3xx_self_test(gs_mg3xx_handle_t * h)
{
    /* Self test cmd */
    gs_error_t res = set_reg(h, MSC_CTRL, true, true, 0x04);
    if (res != GS_OK) {
        return res;
    }

    /* Wait for flash test bit to go low */
    uint16_t reg;
    for(int i = 0; i < 100; i++) {
        res = read_reg(h, MSC_CTRL, &reg);
        if (res != GS_OK) {
            break;
        }
        if (reg & 0x0400) {
            res = GS_ERROR_TIMEOUT;
        } else {
            break;
        }
        gs_time_sleep_ms(10);
    }
    if (res == GS_OK) {
        res = read_reg(h, DIAG_STAT, &reg);
        if (res == GS_OK) {
            if (reg != 0) {
                log_error("m-g3xx: Error in self test - DIAG_STAT: %04x", reg);
                return GS_ERROR_STATE;
            }
        }
    }
    return res;
}

gs_error_t gs_mg3xx_self_test(uint8_t handle, void * driver_data)
{
    GS_CHECK_HANDLE(handle < GS_MG3XX_MAX_NUMBER_HANDLES);
    gs_mg3xx_handle_t * h = &handles[handle];
    GS_CHECK_HANDLE(h->inuse);

    gs_mutex_lock(h->lock);

    gs_error_t res = mg3xx_self_test(h);

    gs_mutex_unlock(h->lock);
    return res;
}

gs_error_t gs_mg3xx_get_product_id(uint8_t handle, char * id_buffer, size_t buffer_len, void * driver_data)
{
    GS_CHECK_HANDLE(handle < GS_MG3XX_MAX_NUMBER_HANDLES);
    gs_mg3xx_handle_t * h = &handles[handle];
    GS_CHECK_HANDLE(h->inuse);
    GS_CHECK_ARG(buffer_len >= 9);
    GS_CHECK_ARG(id_buffer);

    gs_mutex_lock(h->lock);

    uint16_t id[4];
    gs_error_t res = read_reg(h, PROD_ID1, &id[0]);
    if (res == GS_OK) {
        res = read_reg(h, PROD_ID2, &id[1]);
        if (res == GS_OK) {
            res = read_reg(h, PROD_ID3, &id[2]);
            if (res == GS_OK) {
                res = read_reg(h, PROD_ID4, &id[3]);
                if (res == GS_OK) {
                    for (int i = 0; i < 4; i++) {
                        uint16_t tmp = id[i];
                        id_buffer[i * 2] = (char) (tmp & 0xFF);
                        id_buffer[i * 2 + 1] = (char) ((tmp >> 8) & 0xFF);
                    }
                    id_buffer[8] = 0;
                }
            }
        }
    }

    gs_mutex_unlock(h->lock);
    return res;
}

gs_error_t gs_mg3xx_get_serial(uint8_t handle, char * serial_buffer, size_t buffer_len, void * driver_data)
{
    GS_CHECK_HANDLE(handle < GS_MG3XX_MAX_NUMBER_HANDLES);
    gs_mg3xx_handle_t * h = &handles[handle];
    GS_CHECK_HANDLE(h->inuse);
    GS_CHECK_ARG(buffer_len >= 9);
    GS_CHECK_ARG(serial_buffer);

    gs_mutex_lock(h->lock);

    uint16_t ser[4];

    gs_error_t res = read_reg(h, SERIAL_NUM1, &ser[0]);
    if (res == GS_OK) {
        res = read_reg(h, SERIAL_NUM2, &ser[1]);
        if (res == GS_OK) {
            res = read_reg(h, SERIAL_NUM3, &ser[2]);
            if (res == GS_OK) {
                res = read_reg(h, SERIAL_NUM4, &ser[3]);
                if (res == GS_OK) {
                    for(int i = 0; i < 4; i++) {
                        uint16_t tmp = ser[i];
                        serial_buffer[i * 2] = (char)(tmp & 0xFF);
                        serial_buffer[i * 2 + 1] = (char)((tmp >> 8) & 0xFF);
                    }
                    serial_buffer[8] = 0;
                }
            }
        }
    }

    gs_mutex_unlock(h->lock);
    return res;
}

gs_error_t gs_mg3xx_get_version(uint8_t handle, uint16_t * version, void * driver_data)
{
    GS_CHECK_HANDLE(handle < GS_MG3XX_MAX_NUMBER_HANDLES);
    gs_mg3xx_handle_t * h = &handles[handle];
    GS_CHECK_HANDLE(h->inuse);
    GS_CHECK_ARG(version);

    gs_mutex_lock(h->lock);

    gs_error_t res =  read_reg(h, VERSION, version);

    gs_mutex_unlock(h->lock);
    return res;
}

gs_error_t gs_mg3xx_start_sampling(uint8_t handle, gs_gyro_sample_mode_t mode, void * driver_data)
{
    GS_CHECK_SUPPORTED(mode == GS_GYRO_SINGLE_SAMPLE || mode == GS_GYRO_FREE_RUNNING);
    GS_CHECK_HANDLE(handle < GS_MG3XX_MAX_NUMBER_HANDLES);
    gs_mg3xx_handle_t * h = &handles[handle];
    GS_CHECK_HANDLE(h->inuse);
    gs_error_t res = GS_OK;

    gs_mutex_lock(h->lock);

    if (h->sample_mode == GS_GYRO_FREE_RUNNING) {
        if (mode == GS_GYRO_SINGLE_SAMPLE) {
            res = GS_ERROR_BUSY;
        }
    } else {
        res = change_mode(h, GS_MG3XX_SAMPLE_MODE);
    }
    if (res == GS_OK) {
        h->sample_mode = mode;
    }

    gs_mutex_unlock(h->lock);
    return res;
}

static gs_error_t mg3xx_get_sample(gs_mg3xx_handle_t * h, gs_gyro_sample_t * sample)
{
    int16_t rx_buf[BURST_PACKET_SIZE];
    const uint16_t unspecified_tx_data[BURST_PACKET_SIZE] = {0};

    /* Send burst cmd */
    gs_error_t res = set_reg(h, BURST, false, false, 0x00);
    if (res != GS_OK) {
        return res;
    }
    /* Takes time for device to get data ready - see t_stall in data sheet.
       Ideally we sleep only 50 uS, but FreeRTOS only allow sleep of 1 tick = 1 mS */
    gs_time_sleep_ms(1);

    res = gs_spi_master_transaction(h->slave, unspecified_tx_data, rx_buf, sizeof(rx_buf), GS_MG3xx_SPI_TIMEOUT_MS);
    if (res != GS_OK) {
        return res;
    }

    for (unsigned int i = 0; i < (sizeof(rx_buf) / 2); i++) {
        rx_buf[i] = (int16_t)util_ntohs(rx_buf[i]);
    }

    if (h->sample_mode == GS_GYRO_SINGLE_SAMPLE) {
        /* Put back in configuration mode.
           Ideally we sleep only 50 uS, but FreeRTOS only allow sleep of 1 tick = 1 mS */
        gs_time_sleep_ms(1);
        res = set_reg(h, MODE_CTRL, true, false, 0x02);
        if (res != GS_OK) {
            return res;
        }
    }
    if ((rx_buf[0] & 0xF001) != 0xF000) {
        log_error("m-g3xx: Sample error - FLAG: %04x", rx_buf[0]);
        res = GS_ERROR_DATA;
        return res;
    }

    /* Scaling factor from data sheet (same for 364, 365 and 370) */
    const float SF_temp = -0.0037918;
    /* Conversion from data sheet */
    sample->temp = (int16_t)((SF_temp * (float)rx_buf[1] + 34.98) * 10);

    /* Scaling factor from data sheet */
    float SF_gyro = 0;
    switch (h->model) {
        case GS_MG364:
            h->model = 0.0075;
            break;
        case GS_MG365:
        case GS_MG370:
        default:
            SF_gyro = 0.0151515;
            break;
    }

    for(int i = 0; i < 3; i++) {
        /* Conversion from data sheet */
        sample->axes[i] = SF_gyro * (float)rx_buf[2 + i * 2];
    }

    return GS_OK;
}

gs_error_t gs_mg3xx_get_sample(uint8_t handle, gs_gyro_sample_t * sample, void * driver_data)
{
    GS_CHECK_ARG(sample);
    GS_CHECK_HANDLE(handle < GS_MG3XX_MAX_NUMBER_HANDLES);
    gs_mg3xx_handle_t * h = &handles[handle];
    GS_CHECK_HANDLE(h->inuse);

    gs_mutex_lock(h->lock);

    gs_error_t res = mg3xx_get_sample(h, sample);

    gs_mutex_unlock(h->lock);
    return res;

}

gs_error_t gs_mg3xx_init(uint8_t handle, const gs_mg3xx_conf_t * config, void * driver_data)
{
    GS_CHECK_HANDLE(handle < GS_MG3XX_MAX_NUMBER_HANDLES);
    GS_CHECK_ARG(config);

    switch (config->filter) {
        case KAISER_FC50_TAP_32_125SPS:
            break;
        case NO_FILTER_1000SPS:
            break;
        default:
            log_error("m-g3xx: Filter/sps configuration %02x is not supported", (uint8_t)config->filter);
            return GS_ERROR_NOT_SUPPORTED;
    }

    gs_mg3xx_handle_t * h = NULL;
    if (handles[handle].inuse) {
        return GS_ERROR_IN_USE;
    }
    h = &handles[handle];

    h->slave = config->slave;
    /* Set to neither 0 or 1 */
    h->active_window = 0xFF;

    gs_error_t res = wait_for_ready(h);
    if (res != GS_OK) {
        return res;
    }

    uint16_t reg;
    res = read_reg(h, DIAG_STAT, &reg);
    if (res != GS_OK) {
        return res;
    }
    if (reg) {
        log_error("m-g3xx: Init error - DIAG_STAT: %04x", reg);
        return GS_ERROR_IO;
    }
    res = change_mode(h, GS_MG3XX_CONF_MODE);
    if (res != GS_OK) {
        return res;
    }
    /* Turn on temperature and gyro (no accelerometer) */
    res = set_reg(h, SIG_CTRL, true, true, 0xF0);
    if (res != GS_OK) {
        return res;
    }
    /* Turn off delta angle, delta velocity */
    res = set_reg(h, SIG_CTRL, false, true, 0x00);
    if (res != GS_OK) {
        return res;
    }
    /* Set filter */
    res = set_reg(h, FILTER_CTRL, false, true, (uint8_t)((config->filter & 0xFF00) >> 8));
    if (res != GS_OK) {
        return res;
    }
    /* Set sample rate */
    res = set_reg(h, SMPL_CTRL, true, true, (uint8_t)(config->filter & 0xFF));
    if (res != GS_OK) {
        return res;
    }

    /* Let filter be configured - data sheet states it will take 1 ms */
    gs_time_sleep_ms(2);

    res = read_reg(h, FILTER_CTRL, &reg);
    if (res != GS_OK) {
        return res;
    }
    if (reg & 0x0020) {
        log_error("m-g3xx: Filter could not set");
        return GS_ERROR_DATA;
    }
    res = read_reg(h, DIAG_STAT, &reg);
    if (res != GS_OK) {
        return res;
    }
    if (reg) {
        if (reg & 0x0400) {
            log_error("m-g3xx: Wrong filter/sps combination - DIAG_STAT: %04x", reg);
            return GS_ERROR_NOT_SUPPORTED;
        } else {
            log_error("m-g3xx: Error after configuration - DIAG_STAT: %04x", reg);
            return GS_ERROR_IO;
        }
    }

    h->filter = config->filter;

    /* Disabling UART auto mode - just in case */
    res = set_reg(h, UART_CTRL, true, true, 0x00);
    if (res != GS_OK) {
        return res;
    }

    /* Setting burst control to settings which allows for continuous reading (0 delay between consecutive reads) */
    res = set_reg(h, BURST_CTRL1, true, true, 0xF0);
    if (res != GS_OK) {
        return res;
    }
    res = set_reg(h, BURST_CTRL1, false, true, 0x03);
    if (res != GS_OK) {
        return res;
    }
    res = set_reg(h, BURST_CTRL2, true, true, 0x30);
    if (res != GS_OK) {
        return res;
    }

    res = gs_mutex_create(&h->lock);
    if (res != GS_OK) {
        return res;
    }

    h->inuse = true;

    char product_id[9];
    res = gs_mg3xx_get_product_id(handle, product_id, 9, NULL); // Returns something like "G364PDCA"
    if (res != GS_OK) {
        log_error("m-g3xx: gs_mg3xx_get_product_id(%i)", res);
        return res;
    }
    char product_id_reduced[9];
    memcpy(product_id_reduced, product_id, 9);
    product_id_reduced[4] = 0;
    int model = atoi(&product_id_reduced[1]);

    switch (model) {
        case GS_MG364:
            h->model = GS_MG364;
            break;
        case GS_MG365:
            h->model = GS_MG365;
            break;
        case GS_MG370:
            h->model = GS_MG370;
            break;
        default:
            log_warning("m-g3xx: product ID %s not supported . initialized as MG370", product_id);
            h->model = GS_MG370;
            break;
    }

    log_debug("m-g3xx: product ID %s initialized", product_id);


    return GS_OK;
}

gs_error_t gs_mg3xx_max_norm_sample_time(uint8_t handle, uint32_t * max_sample_time_ms, void * driver_data)
{
    GS_CHECK_ARG(max_sample_time_ms);
    GS_CHECK_HANDLE(handle < GS_MG3XX_MAX_NUMBER_HANDLES);
    gs_mg3xx_handle_t * h = &handles[handle];
    GS_CHECK_HANDLE(h->inuse);

    switch (h->filter) {
        case KAISER_FC50_TAP_32_125SPS:
            /* Derived from datasheet */
            *max_sample_time_ms = 32;
            return GS_OK;
        case NO_FILTER_1000SPS:
            /* Derived from datasheet */
            *max_sample_time_ms = 1;
            return GS_OK;
        default:
            log_error("m-g3xx: Current filter/sps configuration is not supported");
            return GS_ERROR_NOT_SUPPORTED;
    }
}

gs_error_t gs_mg3xx_spi_default_config(gs_spi_master_slave_config_t * config, void * driver_data)
{
    GS_CHECK_ARG(config);
    memset(config, 0, sizeof(*config));
    config->data_order_msb = true;
    config->bps = 500000;
    config->mode = GS_SPI_MODE_CPOL1_CPHA1;
    config->bits = 8;
    return GS_OK;
}

static int cmd_info(gs_command_context_t *ctx)
{
    GS_CHECK_ARG(ctx->argc == 1);

    gs_mg3xx_handle_t * handle;
    gs_error_t res = GS_ERROR_NOT_FOUND;
    for (int i = 0; i < GS_MG3XX_MAX_NUMBER_HANDLES; i++) {
        handle = &handles[i];
        if (handle->inuse) {
            char product_id[9];
            res = gs_mg3xx_get_product_id(i, product_id, 9, NULL);
            if (res != GS_OK) {
                return res;
            }
            char serial[9];
            res = gs_mg3xx_get_serial(i, serial, 9, NULL);
            if (res != GS_OK) {
                return res;
            }
            uint16_t version;
            res = gs_mg3xx_get_version(i, &version, NULL);
            if (res != GS_OK) {
                return res;
            }
            char group[15];
            sprintf(group, "device_%i", i);
            char slave_str[10];
            sprintf(slave_str, "%i", i);
            ctx->io_functions->set_result(ctx, group, "slave", slave_str);
            ctx->io_functions->set_result(ctx, group, "product_id", product_id);
            ctx->io_functions->set_result(ctx, group, "serial", serial);
            char version_str[10];
            sprintf(version_str, "%"PRIu16"", version);
            ctx->io_functions->set_result(ctx, group, "sw_version", version_str);

            ctx->io_functions->flush(ctx);
        }
    }

    return GS_OK;
}

static int cmd_get_sample(gs_command_context_t *ctx)
{
    GS_CHECK_ARG(ctx->argc == 2);
    int handle = atoi(ctx->argv[1]);
    GS_CHECK_ARG(handle < GS_MG3XX_MAX_NUMBER_HANDLES);

    uint32_t max_sample_time_ms;
    gs_error_t res = gs_mg3xx_max_norm_sample_time(handle, &max_sample_time_ms, NULL);
    if (res != GS_OK) {
        return res;
    }
    gs_gyro_sample_t sample;
    res = gs_mg3xx_start_sampling(handle, GS_GYRO_SINGLE_SAMPLE, NULL);
    if (res != GS_OK) {
        return res;
    }
    printf("Sleeping %"PRIu32"\n\r", max_sample_time_ms);
    gs_time_sleep_ms(max_sample_time_ms);
    res = gs_mg3xx_get_sample(handle, &sample, NULL);
    if (res != GS_OK) {
        return res;
    }

    char sample_gyro_str[3][20];
    for (int i = 0; i < 3; i++) {
        sprintf(sample_gyro_str[i], "%.9g", sample.axes[i]);
    }
    ctx->io_functions->set_result(ctx, NULL, "gyro_x", sample_gyro_str[0]);
    ctx->io_functions->set_result(ctx, NULL, "gyro_y", sample_gyro_str[1]);
    ctx->io_functions->set_result(ctx, NULL, "gyro_z", sample_gyro_str[2]);

    char temp_str[10];
    sprintf(temp_str, "%"PRIi16"", sample.temp);
    ctx->io_functions->set_result(ctx, NULL, "temp", temp_str);

    ctx->io_functions->flush(ctx);

    return GS_OK;
}

static int cmd_self_test(gs_command_context_t *ctx)
{
    GS_CHECK_ARG(ctx->argc == 2);
    int handle = atoi(ctx->argv[1]);
    GS_CHECK_ARG(handle < GS_MG3XX_MAX_NUMBER_HANDLES);

    gs_error_t res = gs_mg3xx_self_test(handle, NULL);

    ctx->io_functions->set_result(ctx, NULL, "test_result", gs_error_string(res));
    ctx->io_functions->flush(ctx);

    return res;
}

static const gs_command_t GS_COMMAND_SUB mg3xx_subcommands[] = {
    {
        .name = "info",
        .help = "Get static info from initialized device(s)",
        .handler = cmd_info,
        .mandatory_args = 0,
        .optional_args = 0,
    },
    {
        .name = "get_sample",
        .help = "Get sample from device",
        .usage = "[handle]",
        .handler = cmd_get_sample,
        .mandatory_args = 1,
        .optional_args = 0,
    },
    {
        .name = "self_test",
        .help = "Self test on device",
        .usage = "[handle]",
        .handler = cmd_self_test,
        .mandatory_args = 1,
        .optional_args = 0,
    },
};

static const gs_command_t GS_COMMAND_ROOT mg3xx_commands[] = {
    {
        .name = "mg3xx",
        .help = "M-G3xx Gyroscope",
        .chain = GS_COMMAND_INIT_CHAIN(mg3xx_subcommands),
    },
};

gs_error_t gs_mg3xx_register_commands(void)
{
    return GS_COMMAND_REGISTER(mg3xx_commands);
}
