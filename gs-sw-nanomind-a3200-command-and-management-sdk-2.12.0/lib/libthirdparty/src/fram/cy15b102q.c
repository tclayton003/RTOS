/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <string.h>
#include <stdbool.h>

#include <gs/thirdparty/fram/cy15b102q.h>
#include <gs/util/drivers/spi/master.h>
#include <gs/util/drivers/gpio/gpio.h>
#include <gs/util/mutex.h>
#include <gs/util/log.h>
#include <gs/util/check.h>
#include <gs/util/bytebuffer.h>

#define CY15B102Q_FRAM_MAX_HANDLES      2 /* We support up to 2 FRAM devices on the same platform */
#define SPI_TRANSACTION_TIMEOUT_MS      1000
#define FRAM_MEM_SIZE_BYTES             (256 * 1024)

#define SR_WPEN_OFFSET  7
#define SR_BP_OFFSET    2
#define SR_WEL_OFFSET   1

#define SR_WPEN_SET     (1 << SR_WPEN_OFFSET)
#define SR_BP_SET(bp)   (bp << SR_BP_OFFSET)

typedef struct cy15b102q_fram_handle {
    bool initialized;
    gs_mutex_t mutex;
    gs_cy15b102q_fram_config_t config;
} cy15b102q_fram_handle_t;

static cy15b102q_fram_handle_t g_hdl[CY15B102Q_FRAM_MAX_HANDLES];

typedef enum cy15b102q_cmd {
    CMD_WREN = 0x06,
    // CMD_WRDI = 0x04,
    CMD_RDSR = 0x05,
    CMD_WRSR = 0x01,
    CMD_READ = 0x03,
    // CMD_FSTRD = 0x0B,
    CMD_WRITE = 0x02,
    // CMD_SLEEP = 0xB9,
    // CMD_RDID = 0x9F
} cy15b102q_cmd_t;

static inline gs_error_t cy15b102q_fram_write_enable(uint8_t device)
{
    /* Set Write Enable Latch */
    char tx_buf[2] = {CMD_WREN, 0};
    return gs_spi_master_transaction(g_hdl[device].config.spi_slave, tx_buf, NULL, 1, SPI_TRANSACTION_TIMEOUT_MS);
}

static inline void set_wp(cy15b102q_fram_handle_t *hdl, bool state)
{
    if (hdl->config.enable_write_protect) {
        gs_gpio_set(hdl->config.write_protect, state);
    }
}

gs_error_t gs_cy15b102q_fram_init(uint8_t device, const gs_cy15b102q_fram_config_t *config)
{
    GS_CHECK_HANDLE(device < CY15B102Q_FRAM_MAX_HANDLES);
    GS_CHECK_ARG(config != NULL);

    cy15b102q_fram_handle_t* hdl = &g_hdl[device];

    if (hdl->initialized) {
        return GS_ERROR_IN_USE;
    }

    if (gs_mutex_create(&hdl->mutex) != GS_OK) {
        return GS_ERROR_ALLOC;
    }

    hdl->config = *config;
    hdl->initialized = true;

    return GS_OK;
}

gs_error_t gs_cy15b102q_fram_write(uint8_t device, uint32_t to, const void *from, size_t len)
{
    GS_CHECK_HANDLE(device < CY15B102Q_FRAM_MAX_HANDLES);
    GS_CHECK_ARG(len > 0);

    if ((to + len) > FRAM_MEM_SIZE_BYTES) {
        return GS_ERROR_OVERFLOW;
    }

    cy15b102q_fram_handle_t* hdl = &g_hdl[device];

    if (!hdl->initialized)
        return GS_ERROR_NOT_FOUND;

    gs_mutex_lock(hdl->mutex);

    gs_error_t ret = cy15b102q_fram_write_enable(device);
    if (ret == GS_OK) {
        uint8_t write_cmd[4] = {CMD_WRITE, (to >> 16) & 0xFF, (to >> 8) & 0xFF, to & 0xFF};
        gs_spi_master_trans_t t[2] = { {write_cmd, NULL, sizeof(write_cmd)},
                                       {from, NULL, len} };
        ret = gs_spi_master_transactions(hdl->config.spi_slave, t, GS_ARRAY_SIZE(t), SPI_TRANSACTION_TIMEOUT_MS);
    }

    gs_mutex_unlock(hdl->mutex);

    return ret;
}

gs_error_t gs_cy15b102q_fram_read(uint8_t device, uint32_t from, void *to, size_t len)
{
    GS_CHECK_HANDLE(device < CY15B102Q_FRAM_MAX_HANDLES);
    GS_CHECK_ARG(len > 0);

    if ((from + len) > FRAM_MEM_SIZE_BYTES) {
        return GS_ERROR_OVERFLOW;
    }

    cy15b102q_fram_handle_t* hdl = &g_hdl[device];

    if (!hdl->initialized)
        return GS_ERROR_NOT_FOUND;

    gs_mutex_lock(hdl->mutex);

    uint8_t read_cmd[4] = {CMD_READ, (from >> 16) & 0xFF, (from >> 8) & 0xFF, from & 0xFF};
    gs_spi_master_trans_t t[2] = { {read_cmd, NULL, sizeof(read_cmd)},
                                   {NULL, to, len} };
    gs_error_t ret = gs_spi_master_transactions(hdl->config.spi_slave, t, GS_ARRAY_SIZE(t), SPI_TRANSACTION_TIMEOUT_MS);

    gs_mutex_unlock(hdl->mutex);

    return ret;
}

gs_error_t gs_cy15b102q_fram_protect_region(uint8_t device, gs_cy15b102q_fram_region_t region)
{
    GS_CHECK_HANDLE(device < CY15B102Q_FRAM_MAX_HANDLES);

    cy15b102q_fram_handle_t* hdl = &g_hdl[device];

    if (!hdl->initialized)
        return GS_ERROR_NOT_FOUND;

    gs_mutex_lock(hdl->mutex);

    set_wp(hdl, true); /* Disable WP */

    gs_error_t ret = cy15b102q_fram_write_enable(device);
    if (ret == GS_OK)
    {
        /* Set SR (BP1&BP0) to protected region and set (WPEN )*/
        char tx_buf[2] = {CMD_WRSR, (SR_WPEN_SET | SR_BP_SET(region))};
        ret = gs_spi_master_transaction(hdl->config.spi_slave, tx_buf, NULL, sizeof(tx_buf), SPI_TRANSACTION_TIMEOUT_MS);
    }
    /* Ensure that SR and Memory is write protected. */
    set_wp(hdl, false); // WP is active low.

    gs_mutex_unlock(hdl->mutex);

    return ret;
}

/* VMEM driver for the FRAM device */

static inline gs_error_t cy15b102q_vmem_write(const gs_vmem_t * vmem, void * to, const void * from, size_t size)
{
    const gs_cy15b102q_vmem_driver_data_t * dd = vmem->drv_data;
    return gs_cy15b102q_fram_write(dd ? dd->device : 0, GS_TYPES_PTR2UINT(to), from, size);
}

static inline gs_error_t cy15b102q_vmem_read(const gs_vmem_t * vmem, void * to, const void * from, size_t size)
{
    const gs_cy15b102q_vmem_driver_data_t * dd = vmem->drv_data;
    return gs_cy15b102q_fram_read(dd ? dd->device : 0, GS_TYPES_PTR2UINT(from), to, size);
}

static inline gs_error_t cy15b102q_vmem_lock(const gs_vmem_t * vmem, bool on)
{
    if (vmem->drv_data) {
        const gs_cy15b102q_vmem_driver_data_t * dd = vmem->drv_data;
        return gs_cy15b102q_fram_protect_region(dd->device, on ? dd->protect : GS_CY15B102Q_PROTECT_NONE);
    }
    return GS_ERROR_NOT_SUPPORTED;
}

static inline gs_error_t cy15b102q_vmem_info(const gs_vmem_t * vmem, char * buffer, size_t buffer_size)
{
    gs_bytebuffer_t bb;
    gs_bytebuffer_init(&bb, buffer, buffer_size);
    gs_bytebuffer_printf(&bb, "fram: 0x%04x-0x%04x",
                         (unsigned int) vmem->physmem.u, (unsigned int) (vmem->physmem.u + vmem->size - 1));

    bool lockable = false;
    if (vmem->drv_data) {
        const gs_cy15b102q_vmem_driver_data_t * dd = vmem->drv_data;
        lockable = (dd->protect != GS_CY15B102Q_PROTECT_NONE);
    }
    gs_bytebuffer_printf(&bb, ", lockable: %s", (lockable) ? "yes" : "no");

    return GS_OK;
}


const gs_vmem_driver_t gs_cy15b102q_vmem_driver = {
    .write = cy15b102q_vmem_write,
    .read  = cy15b102q_vmem_read,
    .lock  = cy15b102q_vmem_lock,
    .info  = cy15b102q_vmem_info,
};
