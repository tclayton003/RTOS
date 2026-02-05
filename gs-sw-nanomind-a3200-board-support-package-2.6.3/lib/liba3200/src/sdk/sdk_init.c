/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/a3200/led.h>
#include <gs/a3200/pwr_switch.h>
#include <gs/a3200/adc_channels.h>
#include <gs/a3200/lm71.h>
#include <gs/a3200/mpu3300.h>
#include <gs/a3200/hmc5843.h>
#include <gs/a3200/uart.h>
#include <gs/thirdparty/flash/spn_fl512s.h>
#include <gs/a3200/fs.h>
#include <gs/a3200/pwm.h>
#include "../local.h"
#include <gs/csp/csp.h>
#include <gs/csp/rtable.h>
#include <gs/csp/drivers/i2c/i2c.h>
#include <csp/interfaces/csp_if_kiss.h>
#include <csp/interfaces/csp_if_i2c.h>
#include <csp/interfaces/csp_if_can.h>
#include <gs/asf/avr32/asf.h>
#include <gs/embed/asf/drivers/spi/master.h>
#include <gs/embed/stdio.h>
#include <gs/embed/drivers/uart/uart.h>
#include <gs/embed/asf/avr32/drivers/can/can.h>
#include <gs/embed/asf/drivers/i2c/i2c.h>
#include <gs/embed/drivers/sys/reset.h>
#include <gs/embed/command.h>
#include <gs/csp/router.h>
#include <gs/csp/drivers/can/can.h>
#include <gs/csp/log.h>
#include <gs/csp/command.h>
#include <gs/log/appender/memory.h>
#include <gs/util/clock.h>
#include <gs/util/time.h>
#include <gs/util/gosh/command.h>
#include <gs/util/gosh/console.h>
#include <gs/util/vmem.h>
#include <gs/ftp/server.h>
#include <gs/ftp/client.h>
#include <gs/a3200dock/dock.h>
#include <gs/param/host_command.h>
#include <gs/param/rparam.h>
#include <gs/thirdparty/fram/fm33256b.h>
#include <gs/thirdparty/flash/spn_fl512s.h>
#include <gs/storage/vfs/vfs.h>
#include <gs/storage/uffs/command.h>
#include <ctype.h>

static char hostname[GS_A3200_BOARD_UID_LENGTH + 1];
static gs_log_appender_memory_config_t gs_log_appender_fram_config = {0,0};

static gs_log_appender_t gs_log_appender_fram = {
    .name = "fram",
    .drv = &gs_log_appender_memory_driver,
    .drv_config = &gs_log_appender_fram_config,
    .mask = LOG_DEFAULT_MASK,
};

static void init_hostname(void)
{
    // Try and use board->uid as hostname - but only if all characters are printable
    if (GS_A3200_BOARD_UID_GET(hostname, sizeof(hostname)) != GS_OK) {
        hostname[0] = 0;
    }
    hostname[sizeof(hostname) - 1] = 0;

    for (unsigned int i = 0; (i < sizeof(hostname)) && hostname[i]; ++i) {
        if (isprint(hostname[i]) == 0) {
            hostname[0] = 0;
            break;
        }
    }

    // if no valid hostname, fallback to hostname specified at compile time
    if (gs_string_empty(hostname)) {
        GS_STRNCPY(hostname, GS_A3200_HOSTNAME);
    }
}

static void fs_mounted(gs_error_t result)
{
    if (result == GS_OK) {
        GS_A3200_TELEMETRY_FS_MOUNTED_SET_NO_CALLBACK(true);
    } else {
        GS_A3200_TELEMETRY_FS_MOUNTED_SET_NO_CALLBACK(false);
    }
}

static csp_iface_t csp_if_kiss;
static csp_kiss_handle_t csp_kiss_handle;
static uint8_t kiss_uart;

static void kiss_putc_f(char c) 
{
    gs_uart_write(kiss_uart, -1, c);
}

static void kiss_discard_f(char c, void * pxTaskWoken) 
{
}

static void kiss_uart_callback(void * user_data, const uint8_t * data, size_t data_size, gs_context_switch_t * cswitch) 
{
    csp_kiss_rx(&csp_if_kiss, (uint8_t*)data, data_size, &cswitch->task_woken);
}

static void init_csp(void)
{
    // Initialize CSP
    {
        gs_csp_conf_t conf;
        gs_csp_conf_get_defaults_embedded(&conf);
        conf.csp_buffers = 400;

        conf.hostname = hostname;
        conf.model = GS_A3200_MODEL;
        conf.revision = GS_A3200_REVISION;

        conf.address = GS_A3200_BOARD_CSP_ADDR_GET_NC();
        if (gs_csp_is_address_valid(conf.address) == false) {
            conf.address = GS_A3200_NODE;
        }

        gs_csp_init(&conf);
    }

    /* Initialize CSP KISS if enabled in board table */
    if (GS_A3200_BOARD_KISS_EN_GET_NC()) {
        kiss_uart = GS_A3200_BOARD_KISS_UART_GET_NC();
        csp_kiss_init(&csp_if_kiss, &csp_kiss_handle, kiss_putc_f, kiss_discard_f, "KISS");
        gs_uart_set_rx_callback(kiss_uart, kiss_uart_callback, NULL);
    }

    /* Initialize CSP I2C if enabled in board table */
    if (GS_A3200_BOARD_I2C_EN_GET_NC()) {
        gs_a3200_init_twi(0, GS_AVR_I2C_MULTIMASTER, csp_get_address(), GS_A3200_BOARD_I2C_BRATE_GET_NC());
        gs_csp_i2c_init(0, csp_get_address());
    }

    /* Initialize CSP CAN if enabled in board table */
    if (GS_A3200_BOARD_CAN_EN_GET_NC()) {
        gs_csp_can_init2(0, csp_get_address(), CSP_CAN_MTU, NULL, false, NULL);
    }

    /* Load/set routing table */
    char rtable[GS_A3200_BOARD_CSP_RTABLE_LENGTH + 1];
    GS_A3200_BOARD_CSP_RTABLE_GET(rtable, sizeof(rtable));
    gs_csp_rtable_load(rtable, true, true);

    csp_cmp_set_memcpy((csp_memcpy_fnc_t) gs_vmem_cpy);
}

static void init_uart_stdio(void)
{
    uint8_t uart = GS_A3200_BOARD_GOSH_UART_GET_NC();
    if ((uart != 2) && (uart != 4)) {
        uart = gs_stdio_get_default_uart();
        GS_A3200_BOARD_GOSH_UART_SET(uart);
    }
    gs_a3200_uart_init(uart, true, GS_UART_DEFAULT_BPS);
    gs_stdio_set_uart(uart);
}

static void init_uart_kiss(void)
{
    if (GS_A3200_BOARD_KISS_EN_GET_NC()) {
        uint8_t uart = GS_A3200_BOARD_KISS_UART_GET_NC();
        if ((uart != 2) && (uart != 4))
        {
            /* Select the other UART than what is used for stdio */
            if (gs_stdio_get_uart() == 2) {
                uart = 4;
            } else {
                uart = 2;
            }
            GS_A3200_BOARD_KISS_UART_SET(uart);
        }
        gs_a3200_uart_init(uart, true, GS_A3200_BOARD_KISS_BRATE_GET_NC());
    }
}

#define LM71_SPI_TIMEOUT_MS 1000

static void * board_task(void * param)
{
    // Current gs_a200dock implementation requires an array of at least 12
#define ADC_ARRAY 12
#if (ADC_ARRAY < GS_A3200_ADC_NCHANS)
#error Invalid adc array size
#endif

    uint32_t ref_ms = 0;
    int16_t adc[ADC_ARRAY];
    memset(adc, 0, sizeof(adc));

    while (1) {
        /* Sample currents */
        if (gs_a3200_adc_channels_sample(adc) == 0) {
            if (GS_A3200_BOARD_REVISION >= 3) {
                GS_A3200_TELEMETRY_I_GSSB1_SET(GS_A3200_ADC_TO_MV * adc[11] / 0.220 / 100);
                GS_A3200_TELEMETRY_I_GSSB2_SET(GS_A3200_ADC_TO_MV * adc[9] / 0.220 / 100);
                GS_A3200_TELEMETRY_I_FLASH_SET(GS_A3200_ADC_TO_MV * adc[10] / 0.220 / 100);
                GS_A3200_TELEMETRY_I_PWM_SET(GS_A3200_ADC_TO_MV * adc[8] / 0.100 / 100);
                /* Update Dock parameters */
                gs_a3200dock_update_adc(adc, GS_ARRAY_SIZE(adc));
            } else {
                GS_A3200_TELEMETRY_I_GSSB1_SET(GS_A3200_ADC_TO_MV * adc[2] / 0.025 / 100);
                GS_A3200_TELEMETRY_I_GSSB2_SET(GS_A3200_ADC_TO_MV * adc[0] / 0.025 / 100);
                GS_A3200_TELEMETRY_I_FLASH_SET(GS_A3200_ADC_TO_MV * adc[1] / 0.025 / 100);
            }
        }

        /* Sample temperature sensors */
        int16_t temp_cel;
        gs_lm71_read_temp(GS_A3200_SPI_SLAVE_LM71_0, LM71_SPI_TIMEOUT_MS, &temp_cel);
        GS_A3200_TELEMETRY_TEMP_MCU_SET(temp_cel);

        gs_lm71_read_temp(GS_A3200_SPI_SLAVE_LM71_1, LM71_SPI_TIMEOUT_MS, &temp_cel);
        GS_A3200_TELEMETRY_TEMP_RAM_SET(temp_cel);

        /* Set clock and ticks */
        timestamp_t now;
        gs_clock_get_time(&now);
        GS_A3200_TELEMETRY_CLOCK_SET(now.tv_sec);
        GS_A3200_TELEMETRY_UPTIME_SET(gs_time_uptime());

        gs_time_sleep_until_ms(&ref_ms, 1000);
    }

    gs_thread_exit(NULL);
}

// Initialize command-framework and register commands (GOSH).
static void init_and_register_commands(void)
{
    // Initialize command framework - setting minimum stack size for executing commands
    gs_command_init(gs_a3200_get_default_stack_size());

    // On-board (hardware)
    gs_fm33256b_register_commands();
    gs_a3200_adc_register_commands();
    gs_hmc5843_register_commands();
    gs_mpu3300_register_commands();
    gs_a3200_pwm_register_commands();
    gs_embed_register_commands();
    gs_embed_register_driver_commands();
    gs_vmem_register_commands();

    // File system VFS and UFFS
    vfs_register_commands();
#if (GS_STORAGE_VFS)
    gs_spn_fl512s_register_commands();
    gs_storage_register_uffs_commands();
#endif

    // Software
    gs_param_register_commands();
    gs_rparam_register_commands();
    gs_csp_register_commands();
    ftp_register_commands();
}

void * gs_a3200_sdk_init_task(void * param)
{
    /* Disable buffering on standard out */
    setvbuf(stdout, NULL, _IONBF, 0);

    // Initialize SPI1 device - used by FRAM
    gs_a3200_init_spi1();

    // Initialize FRAM and RTC (FM33256B chip)
    gs_a3200_init_fram();

    // Request user to configure VMEM and parameter stores.
    GS_A3200_CALL_HOOK(init_vmem_param);

    // Lock all lockable regions
    gs_vmem_lock_all(true);

    // Create and load 'board' table
    gs_a3200_sdk_param_init_board();

    // Initialize uart for stdio (GOSH)
    init_uart_stdio();

    // LOGGING to stdout is now possible

    // Create and load remaining tables
    gs_a3200_sdk_param_init();

    // Set initial log group/appender level mask configuration (from configuration table)
    {
        const size_t len = (GS_A3200_CONFIGURATION_LOG_MASK_LENGTH + 1);
        char * config = calloc(1, len);
        if (config) {
            if (GS_A3200_CONFIGURATION_LOG_MASK_GET(config, len) == GS_OK) {
                gs_log_set_level_mask_configuration(config);
            }
        }
    }

    // Initialize the FRAM log appender - used for saving primarily error/warning/notice logs
    {
        const gs_vmem_t * vmem = NULL;
        if (GS_A3200_CONFIGURATION_LOG_FRAM_EN_GET_NC()) {
            vmem = gs_vmem_get_by_name("log");
            if (vmem) {
                gs_log_appender_fram_config.address = vmem->virtmem.p;
                gs_log_appender_fram_config.size = vmem->size;
            }
        }
        if (vmem == NULL) {
            const size_t log_size = 0x1000;
            gs_log_appender_fram_config.address = calloc(1, log_size);
            gs_log_appender_fram_config.size = log_size;
        }

        gs_log_appender_add(&gs_log_appender_fram, 1);
        gs_log_group_register_appender(GS_LOG_GROUP_ROOT, gs_log_appender_fram.name);
    }

    // Log and set boot/reset-cause
    {
        gs_sys_boot_cause_t boot_cause;
        gs_sys_reset_cause_t reset_cause;
        gs_sys_log_causes(&boot_cause, &reset_cause, NULL);
        GS_A3200_TELEMETRY_BOOTCAUSE_SET_NO_CALLBACK(boot_cause);
        GS_A3200_TELEMETRY_RESETCAUSE_SET_NO_CALLBACK(reset_cause);
        GS_A3200_TELEMETRY_BOOTCOUNT_SET_NO_CALLBACK(GS_A3200_TELEMETRY_BOOTCOUNT_GET_NC() + 1);
    }

    /* If booted from RAM image we clear the reset cause now - Otherwise we delay this
       until either the RAM image has booted or bypassed. */
    if (GS_A3200_RAM_IMAGE) {
        gs_sys_clear_reset_cause();
    }
    log_notice("Booted from %s image", GS_A3200_RAM_IMAGE ? "Ram" : "Flash");
    GS_A3200_TELEMETRY_RAM_IMAGE_SET_NO_CALLBACK(GS_A3200_RAM_IMAGE ? true : false);

    if (GS_A3200_BOARD_REVISION >= 3) {
        // true  -> Using CS in decode (mux) mode (ADCS-6)
        // false -> Using CS in normal mode (ADCS-3)
        gs_a3200_init_spi0(GS_A3200_CONFIGURATION_SPI0_CS_MODE_GET_NC());
    }

    // Initialize temperature sensors
    gs_a3200_lm71_init();

    // Initialize uart for KISS (over CSP)
    init_uart_kiss();

    // Initialize I2C controller for gyroscope, magnetometer and GSSB devices
    gs_a3200_init_twi(2, GS_AVR_I2C_MASTER, 5, 150000);

    // Initialize can
    gs_a3200_init_can(0, GS_A3200_BOARD_CAN_EN_GET_NC(), GS_A3200_BOARD_CAN_BRATE_GET_NC(), gs_a3200_board_default_values.can_brate);

    // Initialize gyroscope
    gs_mpu3300_init(GS_MPU3300_BW_5, GS_MPU3300_FSR_225);

    // Initialize magnetometer
    gs_hmc5843_init();

    // Setup ADC channels for current measurements
    gs_a3200_adc_channels_init();

    // Initialize dock
    {
        char dock[30];
        gs_a3200_sdk_param_get_dock(dock, sizeof(dock));
        gs_string_trim(dock, sizeof(dock));
        gs_a3200dock_init(dock);
    }

    // Initialize command-framework and register commands (GOSH).
    init_and_register_commands();

    init_hostname();

    init_csp();

    // Start the CSP router task
    gs_csp_router_task_start(gs_a3200_get_default_stack_size(), GS_THREAD_PRIORITY_HIGH);

    // Start the file system mounting task
    gs_a3200_fs_mount(fs_mounted);

    // Start GOSH console on stdio (uart)
    gs_console_start(hostname, 0);

    if (GS_A3200_SDK_BOARD_TASK) {
        gs_thread_create("BOARD", board_task, NULL, gs_a3200_get_default_stack_size(), GS_THREAD_PRIORITY_LOW, 0, NULL);
    }

    GS_A3200_CALL_HOOK(init_complete);

    // wait for File System to be mounted
    {
        int mount_wait_s = 300;
        while (mount_wait_s-- && !GS_A3200_TELEMETRY_FS_MOUNTED_GET_NC()) {
            gs_time_sleep_ms(1000);
        }
    }

    if (!GS_A3200_RAM_IMAGE) {
        gs_thread_create("SWLOAD", gs_a3200_sdk_swload_task, NULL, gs_a3200_get_default_stack_size(), GS_THREAD_PRIORITY_LOW, 0, NULL);
    }

    gs_ftp_server_register_backend(GS_FTP_BACKEND_FILE, gs_ftp_server_backend_file());

    GS_A3200_CALL_HOOK(fs_mounted);

    gs_thread_exit(NULL);
}
