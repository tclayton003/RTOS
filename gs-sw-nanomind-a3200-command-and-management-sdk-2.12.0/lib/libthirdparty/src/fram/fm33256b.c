/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/fram/fm33256b.h>

#include <string.h>
#include <time.h>
#include <gs/util/drivers/spi/master.h>
#include <gs/util/drivers/gpio/gpio.h>
#include <gs/util/mutex.h>
#include <gs/util/log.h>
#include <gs/util/check.h>
#include <gs/util/bytebuffer.h>

#define FM33256B_MAX_HANDLES        2    /* We support up to 2 FRAM devices on the same platform */
#define SPI_TRANSACTION_TIMEOUT_MS  1000
#define FRAM_MEM_SIZE_BYTES         (32 * 1024)

/* Selected Register masks */
#define ALARM_RTC_CONTROL_RD_MASK   0x01
#define ALARM_RTC_CONTROL_WR_MASK   0x02

/* Status Register */
#define SR_BP_OFFSET    2
#define SR_WEL_OFFSET   1
#define SR_BP_SET(bp)   (bp << SR_BP_OFFSET)

typedef struct fm33256b_handle {
    bool initialized;
    gs_mutex_t mutex;
    gs_fm33256b_config_t config;
} fm33256b_handle_t;

static fm33256b_handle_t g_hdl[FM33256B_MAX_HANDLES];

typedef enum fm33256b_cmd {
    CMD_WREN = 0x06,
    // CMD_WRDI = 0x04,
    // CMD_RDSR = 0x05,
    CMD_WRSR = 0x01,
    CMD_READ = 0x03,
    CMD_WRITE = 0x02,
    CMD_RDPC = 0x13,
    CMD_WRPC = 0x12
} fm33256b_cmd_t;

typedef enum fm33256b_reg {
    /* Misc registers */
    REG_ALARM_RTC_CTRL  = 0x00,
    REG_CAL_CTRL        = 0x01,
    /* RTC registers */
    REG_RTC_SECONDS     = 0x02,
    REG_RTC_MINUTES     = 0x03,
    REG_RTC_HOURS       = 0x04,
    REG_RTC_DAY         = 0x05,
    REG_RTC_DATE        = 0x06,
    REG_RTC_MONTHS      = 0x07,
    REG_RTC_YEARS       = 0x08,
    /* Watchdog registers */
    REG_WDOG_FLAGS      = 0x09,
    REG_WDOG_RESTART    = 0x0A,
    REG_WDOG_CTRL_1     = 0x0B,
    REG_WDOG_CTRL_2     = 0x0C,
    /* Event register */
    REG_EVENT_CTRL      = 0x0D,
    REG_EVENT_CNT_1     = 0x0E,
    REG_EVENT_CNT_2     = 0x0F,
    /* Serial Number registers */
    REG_SERIAL_NUM_0    = 0x10,
    REG_SERIAL_NUM_1    = 0x11,
    REG_SERIAL_NUM_2    = 0x12,
    REG_SERIAL_NUM_3    = 0x13,
    REG_SERIAL_NUM_4    = 0x14,
    REG_SERIAL_NUM_5    = 0x15,
    REG_SERIAL_NUM_6    = 0x16,
    REG_SERIAL_NUM_7    = 0x17,
    /* Companion CTRL registers */
    REG_COMPANION_CTRL  = 0x18,
    /* Alarm registers */
    REG_ALARM_SECONDS   = 0x19,
    REG_ALARM_MINUTES   = 0x1A,
    REG_ALARM_HOURS     = 0x1B,
    REG_ALARM_DATE      = 0x1C,
    REG_ALARM_MONTHS    = 0x1D
} fm33256b_reg_t;

struct fm33256b_rtc_clock {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hour;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} __attribute__ ((packed));

static inline uint8_t bcd_to_sane(uint8_t bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

static inline uint8_t sane_to_bcd(uint8_t sane)
{
    return ((sane / 10) << 4) + (sane % 10);
}

static void bcd_to_sane_clock(struct fm33256b_rtc_clock* rtc_clock)
{
    rtc_clock->seconds = bcd_to_sane(rtc_clock->seconds & 0x7F);
    rtc_clock->minutes = bcd_to_sane(rtc_clock->minutes);
    rtc_clock->hour = bcd_to_sane(rtc_clock->hour);
    rtc_clock->day = bcd_to_sane(rtc_clock->day);
    rtc_clock->date = bcd_to_sane(rtc_clock->date);
    rtc_clock->month = bcd_to_sane(rtc_clock->month);
    rtc_clock->year = bcd_to_sane(rtc_clock->year);
}

static void sane_to_bcd_clock(struct fm33256b_rtc_clock* rtc_clock)
{
    rtc_clock->seconds = sane_to_bcd(rtc_clock->seconds & 0x7F);
    rtc_clock->minutes = sane_to_bcd(rtc_clock->minutes);
    rtc_clock->hour = sane_to_bcd(rtc_clock->hour);
    rtc_clock->day = sane_to_bcd(rtc_clock->day);
    rtc_clock->date = sane_to_bcd(rtc_clock->date);
    rtc_clock->month = sane_to_bcd(rtc_clock->month);
    rtc_clock->year = sane_to_bcd(rtc_clock->year);
}

static inline gs_error_t fm33256b_fram_write_enable(fm33256b_handle_t *hdl)
{
    /* Set Write Enable Latch */
    char tx_buf[1] = {CMD_WREN};
    return gs_spi_master_transaction(hdl->config.spi_slave, tx_buf, NULL, 1, SPI_TRANSACTION_TIMEOUT_MS);
}

static gs_error_t fm33256b_write_reg(fm33256b_handle_t *hdl, fm33256b_reg_t reg, uint8_t* data, uint8_t len)
{
    fm33256b_fram_write_enable(hdl);

    uint8_t cmd[2] = {CMD_WRPC, reg};
    gs_spi_master_trans_t trans[2] = {{cmd, NULL, sizeof(cmd)},
                                      {data, NULL, len}};

    return gs_spi_master_transactions(hdl->config.spi_slave, trans, GS_ARRAY_SIZE(trans), SPI_TRANSACTION_TIMEOUT_MS);
}

static gs_error_t fm33256b_read_reg(fm33256b_handle_t *hdl, fm33256b_reg_t reg, uint8_t* data, uint8_t len)
{
    uint8_t cmd[2] = {CMD_RDPC, reg};
    gs_spi_master_trans_t trans[2] = {{cmd, NULL, sizeof(cmd)},
                                      {NULL, data, len}};

    return gs_spi_master_transactions(hdl->config.spi_slave, trans, GS_ARRAY_SIZE(trans), SPI_TRANSACTION_TIMEOUT_MS);
}

static gs_error_t fm33256b_rtc_time_to_clock(const time_t *time_seconds, struct fm33256b_rtc_clock *rtc_clock)
{
    struct tm tmbuf;
    struct tm *t = gmtime_r(time_seconds, &tmbuf);
    if (t && rtc_clock) {
        rtc_clock->seconds = t->tm_sec;
        rtc_clock->minutes = t->tm_min;
        rtc_clock->hour    = t->tm_hour;
        rtc_clock->date    = t->tm_mday;
        rtc_clock->month   = t->tm_mon + 1;
        rtc_clock->day     = t->tm_wday;
        rtc_clock->year    = t->tm_year-100;
        return GS_OK;
    }
    return GS_ERROR_ARG;
}

static gs_error_t fm33256b_rtc_clock_to_time(time_t *time_seconds, const struct fm33256b_rtc_clock *rtc_clock)
{
    struct tm t;

    if (!time_seconds || !rtc_clock) {
        return GS_ERROR_ARG;
    }

    t.tm_sec = rtc_clock->seconds;
    t.tm_min = rtc_clock->minutes;
    t.tm_hour = rtc_clock->hour;
    t.tm_mday = rtc_clock->date;
    t.tm_mon = rtc_clock->month - 1;
    t.tm_year = rtc_clock->year+100;
    t.tm_isdst = 0;

    time_t seconds = mktime(&t);
    if (seconds < 0) {
        return GS_ERROR_RANGE;
    }

    *time_seconds = seconds;

    return GS_OK;
}

static gs_error_t fm33256b_rtc_set_time(void * driver_data, const gs_timestamp_t * tm)
{
    gs_fm33256b_rtc_driver_data_t * dd = driver_data;
    /* These RTC API's don't have a handle, which means there can only be one.
     * Consequently this is hard-coded to device 0. */
    fm33256b_handle_t* hdl = &g_hdl[dd ? dd->device : 0];

    if (!hdl->initialized)
        return GS_ERROR_HANDLE;

    gs_mutex_lock(hdl->mutex);

    gs_error_t ret = GS_OK;
    /* Freeze the time keeping registers for write */
    uint8_t rtc_alarm_ctrl;
    ret |= fm33256b_read_reg(hdl, REG_ALARM_RTC_CTRL, &rtc_alarm_ctrl, sizeof(rtc_alarm_ctrl));
    rtc_alarm_ctrl |= ALARM_RTC_CONTROL_WR_MASK; // Sets the W bit in the register.
    ret |= fm33256b_write_reg(hdl, REG_ALARM_RTC_CTRL, &rtc_alarm_ctrl, sizeof(rtc_alarm_ctrl));

    if (ret == GS_OK)
    {
        /* Get time from RTC registers: Seconds, minutes, hours,.... */
        struct fm33256b_rtc_clock rtc_clock;
        ret = fm33256b_rtc_time_to_clock((time_t *) &tm->tv_sec, &rtc_clock);
        if (ret == GS_OK)
        {
            /* Convert to BCD before writing to FM33256B */
            sane_to_bcd_clock(&rtc_clock);

            ret |= fm33256b_write_reg(hdl, REG_RTC_SECONDS, (uint8_t *)&rtc_clock, sizeof(rtc_clock));

            /* Un-freeze the RTC registers again */
            rtc_alarm_ctrl &= ~ALARM_RTC_CONTROL_WR_MASK; //Clears the W bit in the register.
            ret |= fm33256b_write_reg(hdl, REG_ALARM_RTC_CTRL, &rtc_alarm_ctrl, sizeof(rtc_alarm_ctrl));
        }
    }

    gs_mutex_unlock(hdl->mutex);

    return ret;
}

static gs_error_t fm33256b_rtc_get_time(void * driver_data, gs_timestamp_t * tm)
{
    gs_fm33256b_rtc_driver_data_t * dd = driver_data;
    /* These RTC API's don't have a handle, which means there can only be one.
     * Consequently this is hard-coded to device 0. */
    fm33256b_handle_t* hdl = &g_hdl[dd ? dd->device : 0];

    if (!hdl->initialized)
        return GS_ERROR_HANDLE;

    gs_mutex_lock(hdl->mutex);

    gs_error_t ret = GS_OK;
    /* Load the RTC registers with current time */
    uint8_t rtc_alarm_ctrl;
    ret |= fm33256b_read_reg(hdl, REG_ALARM_RTC_CTRL, &rtc_alarm_ctrl, sizeof(rtc_alarm_ctrl));
    rtc_alarm_ctrl |= ALARM_RTC_CONTROL_RD_MASK; // Sets the R bit in the register.
    ret |= fm33256b_write_reg(hdl, REG_ALARM_RTC_CTRL, &rtc_alarm_ctrl, sizeof(rtc_alarm_ctrl));

    rtc_alarm_ctrl &= ~ALARM_RTC_CONTROL_RD_MASK; // Clear the R bit in the register.
    ret |= fm33256b_write_reg(hdl, REG_ALARM_RTC_CTRL, &rtc_alarm_ctrl, sizeof(rtc_alarm_ctrl));

    /* Get time from RTC registers: Seconds, minutes, hours,.... */
    struct fm33256b_rtc_clock rtc_clock;
    ret |= fm33256b_read_reg(hdl, REG_RTC_SECONDS, (uint8_t *)&rtc_clock, sizeof(rtc_clock));

    /* Convert from BCD before working on the clock */
    bcd_to_sane_clock(&rtc_clock);

    /* Convert to timestamp */
    time_t tt = 0;
    ret |= fm33256b_rtc_clock_to_time(&tt, &rtc_clock);
    tm->tv_sec = tt;
    tm->tv_nsec = 0;

    gs_mutex_unlock(hdl->mutex);

    return ret;
}

static gs_error_t companion_fm33256b_init(fm33256b_handle_t *hdl)
{
    /* Start the Oscillator to ensure RTC is running. */
    uint8_t rtc_alarm_ctrl = 0x00;
    fm33256b_write_reg(hdl, REG_ALARM_RTC_CTRL, &rtc_alarm_ctrl, sizeof(rtc_alarm_ctrl));

    uint8_t comp_ctrl = 0x3D; // Magic number - See Register manual for reference..
    return fm33256b_write_reg(hdl, REG_COMPANION_CTRL, &comp_ctrl, sizeof(comp_ctrl));


}

static gs_error_t wdog_fm33256b_init(fm33256b_handle_t *hdl)
{
    uint8_t wdog_ctrl = 0x00;
    return fm33256b_write_reg(hdl, REG_WDOG_CTRL_2, &wdog_ctrl, sizeof(wdog_ctrl));
}

gs_error_t gs_fm33256b_init(uint8_t device, const gs_fm33256b_config_t *config)
{
    GS_CHECK_HANDLE(device < FM33256B_MAX_HANDLES);
    GS_CHECK_ARG(config != NULL);

    fm33256b_handle_t* hdl = &g_hdl[device];

    if (hdl->initialized) {
        return GS_ERROR_IN_USE;
    }

    if (gs_mutex_create(&hdl->mutex) != GS_OK) {
        return GS_ERROR_ALLOC;
    }

    hdl->initialized = true;
    hdl->config = *config;

    gs_error_t ret = wdog_fm33256b_init(hdl);
    if (ret == GS_OK) {
        ret = companion_fm33256b_init(hdl);
    }

    return ret;
}

gs_error_t gs_fm33256b_fram_write(uint8_t device, uint32_t to, const void *from, size_t len)
{
    GS_CHECK_HANDLE(device < FM33256B_MAX_HANDLES);
    GS_CHECK_ARG(len > 0);

    if ((to + len) > FRAM_MEM_SIZE_BYTES) {
        return GS_ERROR_OVERFLOW;
    }
            
    fm33256b_handle_t* hdl = &g_hdl[device];
    
    if (!hdl->initialized)
        return GS_ERROR_NOT_FOUND;

    gs_mutex_lock(hdl->mutex);

    gs_error_t ret = fm33256b_fram_write_enable(hdl);
    if (ret == GS_OK)
    {
        uint8_t cmd[3] = {CMD_WRITE, (to >> 8) & 0xFF, to & 0xFF};
        gs_spi_master_trans_t trans[2] = {{cmd, NULL, sizeof(cmd)},
                                          {from, NULL, len}};
        ret = gs_spi_master_transactions(hdl->config.spi_slave, trans, GS_ARRAY_SIZE(trans), SPI_TRANSACTION_TIMEOUT_MS);
    }
    gs_mutex_unlock(hdl->mutex);

    return ret;
}

gs_error_t gs_fm33256b_fram_read(uint8_t device, uint32_t from, void *to, size_t len)
{
    GS_CHECK_HANDLE(device < FM33256B_MAX_HANDLES);
    GS_CHECK_ARG(len > 0);

    if ((from + len) > FRAM_MEM_SIZE_BYTES) {
        return GS_ERROR_OVERFLOW;
    }

    fm33256b_handle_t* hdl = &g_hdl[device];

    if (!hdl->initialized)
        return GS_ERROR_NOT_FOUND;

    gs_mutex_lock(hdl->mutex);

    uint8_t cmd[3] = {CMD_READ, (from >> 8) & 0xFF, from & 0xFF};
    gs_spi_master_trans_t trans[2] = {{cmd, NULL, sizeof(cmd)},
                                      {NULL, to, len}};
    gs_error_t ret = gs_spi_master_transactions(hdl->config.spi_slave, trans, GS_ARRAY_SIZE(trans), SPI_TRANSACTION_TIMEOUT_MS);

    gs_mutex_unlock(hdl->mutex);

    return ret;
}

gs_error_t gs_fm33256b_fram_protect_region(uint8_t device, gs_fm33256b_fram_region_t region)
{
    GS_CHECK_HANDLE(device < FM33256B_MAX_HANDLES);

    fm33256b_handle_t* hdl = &g_hdl[device];

    if (!hdl->initialized)
        return GS_ERROR_NOT_FOUND;

    gs_mutex_lock(hdl->mutex);

    gs_error_t ret = fm33256b_fram_write_enable(hdl);
    if (ret == GS_OK)
    {
        /* Set SR (BP1&BP0) to protected region */
        char tx_buf[2] = {CMD_WRSR, SR_BP_SET(region)};
        ret = gs_spi_master_transaction(hdl->config.spi_slave, tx_buf, NULL, sizeof(tx_buf), SPI_TRANSACTION_TIMEOUT_MS);
    }

    gs_mutex_unlock(hdl->mutex);

    return ret;
}

const gs_rtc_driver_t gs_fm33256b_rtc_driver = {
    .set_time = fm33256b_rtc_set_time,
    .get_time = fm33256b_rtc_get_time,
};

static inline gs_error_t fm33256b_vmem_write(const gs_vmem_t * vmem, void * to, const void * from, size_t size)
{
    const gs_fm33256b_vmem_driver_data_t * dd = vmem->drv_data;
    return gs_fm33256b_fram_write(dd ? dd->device : 0, GS_TYPES_PTR2UINT(to), from, size);
}

static inline gs_error_t fm33256b_vmem_read(const gs_vmem_t * vmem, void * to, const void * from, size_t size)
{
    const gs_fm33256b_vmem_driver_data_t * dd = vmem->drv_data;
    return gs_fm33256b_fram_read(dd ? dd->device : 0, GS_TYPES_PTR2UINT(from), to, size);
}

static inline gs_error_t fm33256b_vmem_lock(const gs_vmem_t * vmem, bool on)
{
    if (vmem->drv_data) {
        const gs_fm33256b_vmem_driver_data_t * dd = vmem->drv_data;
        return gs_fm33256b_fram_protect_region(dd->device, on ? dd->protect : GS_FM33256B_PROTECT_NONE);
    }
    return GS_ERROR_NOT_SUPPORTED;
}

static inline gs_error_t fm33256b_vmem_info(const gs_vmem_t * vmem, char * buffer, size_t buffer_size)
{
    gs_bytebuffer_t bb;
    gs_bytebuffer_init(&bb, buffer, buffer_size);
    gs_bytebuffer_printf(&bb, "fram: 0x%04x-0x%04x",
                         (unsigned int) vmem->physmem.u, (unsigned int) (vmem->physmem.u + vmem->size - 1));

    bool lockable = false;
    if (vmem->drv_data) {
        const gs_fm33256b_vmem_driver_data_t * dd = vmem->drv_data;
        lockable = (dd->protect != GS_FM33256B_PROTECT_NONE);
    }
    gs_bytebuffer_printf(&bb, ", lockable: %s", (lockable) ? "yes" : "no");

    return GS_OK;
}

const gs_vmem_driver_t gs_fm33256b_vmem_driver = {
    .write = fm33256b_vmem_write,
    .read  = fm33256b_vmem_read,
    .lock  = fm33256b_vmem_lock,
    .info  = fm33256b_vmem_info,
};
