/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/a3200/pwr_switch.h>
#include <gs/a3200/lm71.h>
#include <gs/a3200/adc_channels.h>
#include <gs/a3200/led.h>
#include <gs/a3200/spi_slave.h>
#include <gs/a3200/uart.h>
#include "local.h"
#include <spi.h>
#include <sdramc.h>
#include <gs/thirdparty/fram/fm33256b.h>
#include <gs/embed/drivers/sys/reset.h>
#include <gs/embed/drivers/sys/memory.h>
#include <gs/embed/asf/drivers/spi/master.h>
#include <gs/embed/drivers/uart/uart.h>
#include <gs/embed/asf/drivers/i2c/i2c.h>
#include <gs/embed/asf/avr32/drivers/can/can.h>
#include <gs/embed/stdio.h>
#include "gs/embed/asf/avr32/drivers/sys/avr32_reset_all_interrupt_settings.h"
#include <gs/util/rtc.h>
#include <gs/util/time.h>
#include <gs/util/thread.h>
#include <gs/util/clock.h>

#ifndef __linux
/* This is a required to enable CPP linking */
void *__dso_handle __attribute__((used)) = 0;
#endif

const gs_a3200_hooks_t * gs_a3200_hooks;

void gs_a3200_init_spi0(bool decode)
{
    const gpio_map_t map = {
        {AVR32_SPI0_MOSI_2_PIN, AVR32_SPI0_MOSI_2_FUNCTION},
        {AVR32_SPI0_MISO_2_PIN, AVR32_SPI0_MISO_2_FUNCTION},
        {AVR32_SPI0_SCK_2_PIN, AVR32_SPI0_SCK_2_FUNCTION},
        {AVR32_SPI0_NPCS_0_2_PIN, AVR32_SPI0_NPCS_0_2_FUNCTION},
        {AVR32_SPI0_NPCS_1_2_PIN, AVR32_SPI0_NPCS_1_2_FUNCTION},
        {AVR32_SPI0_NPCS_2_2_PIN, AVR32_SPI0_NPCS_2_2_FUNCTION},
    };
    gpio_enable_module(map, GS_ARRAY_SIZE(map));

    gs_spi_init_device(0, decode);
}

void gs_a3200_init_spi1(void)
{
    const gpio_map_t map = {
        {AVR32_SPI1_SCK_0_1_PIN, AVR32_SPI1_SCK_0_1_FUNCTION},
        {AVR32_SPI1_MOSI_0_1_PIN, AVR32_SPI1_MOSI_0_1_FUNCTION},
        {AVR32_SPI1_MOSI_0_2_PIN, AVR32_SPI1_MOSI_0_2_FUNCTION},
        {AVR32_SPI1_MISO_0_1_PIN, AVR32_SPI1_MISO_0_1_FUNCTION},
        {AVR32_SPI1_NPCS_2_2_PIN, AVR32_SPI1_NPCS_2_2_FUNCTION},
        {AVR32_SPI1_NPCS_3_2_PIN, AVR32_SPI1_NPCS_3_2_FUNCTION},
        {AVR32_SPI1_NPCS_0_PIN, AVR32_SPI1_NPCS_0_FUNCTION},
        {AVR32_SPI1_NPCS_1_2_PIN, AVR32_SPI1_NPCS_1_2_FUNCTION},
    };
    gpio_enable_module(map, GS_ARRAY_SIZE(map));

    gs_spi_init_device(1, false);
}

gs_error_t gs_a3200_init_twi(uint8_t device, gs_avr32_i2c_mode_t mode, uint8_t addr, uint32_t bps)
{
    if (device == 0) {
        const gpio_map_t map = {
            {AVR32_TWIMS0_TWCK_0_0_PIN, AVR32_TWIMS0_TWCK_0_0_FUNCTION},
            {AVR32_TWIMS0_TWD_0_0_PIN, AVR32_TWIMS0_TWD_0_0_FUNCTION}
        };
        gpio_configure_pin(AVR32_TWIMS0_TWCK_0_0_PIN, GPIO_DRIVE_HIGH);
        gpio_configure_pin(AVR32_TWIMS0_TWD_0_0_PIN, GPIO_DRIVE_HIGH);
        gpio_enable_module(map, GS_ARRAY_SIZE(map));

    } else if (device == 2) {
        const gpio_map_t map = {
            {AVR32_TWIMS2_TWCK_0_0_PIN, AVR32_TWIMS2_TWCK_0_0_FUNCTION},
            {AVR32_TWIMS2_TWD_0_0_PIN, AVR32_TWIMS2_TWD_0_0_FUNCTION}
        };
        gpio_enable_module(map, GS_ARRAY_SIZE(map));

    } else {
        return GS_ERROR_HANDLE;
    }

    return gs_avr_i2c_init(device, mode, addr, bps);
}

gs_error_t gs_a3200_init_can(uint8_t device, bool enable, uint32_t bps, uint32_t fallback_bps)
{
    if (device != 0) {
        return GS_ERROR_HANDLE;
    }
    
    /* Setup the generic clock for CAN */
    scif_gc_setup(AVR32_SCIF_GCLK_CANIF,
                  SCIF_GCCTRL_OSC0,
                  AVR32_SCIF_GC_NO_DIV_CLOCK,
                  0);

    if (enable) {
        /* Enable the generic clock */
        scif_gc_enable(AVR32_SCIF_GCLK_CANIF);

        /* Setup GPIO map for CAN connection in stack */
        const gpio_map_t map = {
            {AVR32_CANIF_RXLINE_0_0_PIN, AVR32_CANIF_RXLINE_0_0_FUNCTION},
            {AVR32_CANIF_TXLINE_0_0_PIN, AVR32_CANIF_TXLINE_0_0_FUNCTION}
        };
        gpio_enable_module(map, GS_ARRAY_SIZE(map));

        /* Initialize PA15 (CANMODE) GPIO, must be pulled low to enable CAN Transceiver TI SN65HVD230 */
        gpio_configure_pin(AVR32_PIN_PA15, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);

        return gs_can_init_w_fallback(device, bps, fallback_bps);
    }

    // disable CAN
    /* Stop the generic clock */
    scif_stop_gclk(AVR32_SCIF_GCLK_CANIF);
    /* Ensure can output is high to disable/power down transceiver */
    gpio_configure_pin(AVR32_PIN_PA15, (GPIO_DIR_OUTPUT | GPIO_INIT_HIGH));

    return GS_OK;
}

gs_error_t gs_a3200_uart_init(uint8_t uart, bool enable, uint32_t bps)
{
    if (uart == 1) {
        /* UART 1 (FSI connector / GPS) */
        const gpio_map_t map = {
            {AVR32_USART1_RXD_PIN, AVR32_USART1_RXD_FUNCTION},
            {AVR32_USART1_TXD_PIN, AVR32_USART1_TXD_FUNCTION},
        };
        gpio_enable_module(map, GS_ARRAY_SIZE(map));
    } else if (uart == 2) {
    	/* UART 2 (DEBUG connector) */
        const gpio_map_t map = {
            {AVR32_USART2_RXD_1_PIN, AVR32_USART2_RXD_1_FUNCTION},
            {AVR32_USART2_TXD_1_PIN, AVR32_USART2_TXD_1_FUNCTION},
    	};
    	gpio_enable_module(map, GS_ARRAY_SIZE(map));
    } else if (uart == 4) {
    	/* UART 4 (FSI connector) */
    	const gpio_map_t map = {
            {AVR32_USART4_RXD_PIN, AVR32_USART4_RXD_FUNCTION},
            {AVR32_USART4_TXD_PIN, AVR32_USART4_TXD_FUNCTION},
    	};
    	gpio_enable_module(map, GS_ARRAY_SIZE(map));
    } else {
        return GS_ERROR_HANDLE;
    }

    gs_uart_config_t uart_config;
    gs_uart_get_default_config(&uart_config);
    uart_config.tx_queue_size = 0; // direct write
    uart_config.comm.bps = bps;

    gs_error_t error = gs_uart_init(uart, &uart_config);
    if (error) {
        log_error("Failed to initialize UART: %u, bps: %"PRIu32" - gs_uart_init() return: %s ",
                  uart, bps, gs_error_string(error));
    }
    return error;
}

static gs_error_t init_rtc(void)
{
    /* 32kHz Crystal setup */
    osc_enable(OSC_ID_OSC32);

    return gs_rtc_register(&gs_fm33256b_rtc_driver, NULL);
}

gs_error_t gs_a3200_init_fram(void)
{
    const gs_spi_chip_t spi = {.handle = 1,
                               .baudrate = 8000000,
                               .bits = 8,
                               .chipselect = FM33256_SPI_CS,
                               .spi_mode = 0,
                               .stay_act = 1,
                               .trans_delay = 5};
    const gs_fm33256b_config_t fram = {.spi_slave = GS_A3200_SPI_SLAVE_FRAM};

    gs_error_t error = gs_spi_asf_master_configure_slave(fram.spi_slave, &spi);
    if (error) {
        log_error("Failed configuring SPI for FRAM (FM33256B), error: %s", gs_error_string(error));
        return error;
    }

    error = gs_fm33256b_init(0, &fram);
    if (error) {
        log_error("Failed initializing FRAM (FM33256B), error: %s", gs_error_string(error));
        return error;
    }

    return init_rtc();
}

// Default implementation - can be overwritten
__attribute__((weak)) size_t gs_a3200_get_default_stack_size(void)
{
    return GS_A3200_DEFAULT_STACK_SIZE;
}

// Overwrite libembed function - we have external memory
void * gs_mem_get_ext_ram_size(size_t * size)
{
    *size = EXTERNAL_RAM_SIZE;
    return GS_TYPES_UINT2PTR(EXTERNAL_RAM_ADDR);
}

static void * bsp_init_task(void * param)
{
    /* Disable buffering on standard out */
    setvbuf(stdout, NULL, _IONBF, 0);

    /* SPI1 device drivers */
    gs_a3200_init_spi1();

    /* Init temperature sensors */
    gs_a3200_lm71_init();

    /* Init FRAM and RTC (FM33256B chip) */
    gs_a3200_init_fram();

    /* Init SPI0 device */
    if (GS_A3200_BOARD_REVISION >= 3) {
        gs_a3200_init_spi0(false);
    }

    /* Init UARTs */
    gs_a3200_uart_init(2, true, GS_UART_DEFAULT_BPS);
    gs_a3200_uart_init(4, true, GS_UART_DEFAULT_BPS);

    /* Log latest boot- and reset-cause. */
    gs_sys_log_causes(NULL, NULL, NULL);
    gs_sys_clear_reset_cause();

    /* Init I2C device for gyroscope, magnetometer and GSSB devices */
    gs_a3200_init_twi(2, GS_AVR_I2C_MASTER, 5, 150000);

    /* Init ADC channels for current measurements */
    gs_a3200_adc_channels_init();

    /* User call-back hook - initialiazation complete, start application task(s) */
    GS_A3200_CALL_HOOK(init_complete);

    gs_thread_exit(NULL);
}

gs_error_t gs_a3200_init_set_heap(bool external_heap)
{
    /* Reset & start watchdog (will be re-initialized by SWWD in SDK) */
    wdt_disable();
    wdt_clear();
    wdt_opt_t wdtopt = { .us_timeout_period = 5000000 };
    wdt_enable(&wdtopt);

    /* Resetting/Clearing all interrupts, so they are disabled when booting a RAM image -
       It is also disabled in swload, but having it here will allow a RAM image to disable it regardless 
       if the SW in flash does */
#ifndef __linux
    gs_sys_avr32_reset_all_interrupt_settings();
#endif

    /* Initialize interrupt handling.
       This function call is needed, when using libasf startup.S file */
    INTC_init_interrupts();

    /* Init CLK */
    sysclk_init();

    /* Init pwr channels */
    gs_a3200_pwr_switch_init();

    if (!GS_A3200_RAM_IMAGE) {
        /* Init SDRAM, do this before malloc is called - It is configured already when booting from RAM */
        sdramc_init(sysclk_get_cpu_hz());
    }

    /* We trust that the SDRAM is working by now - ok to use/access external memory */

    if (external_heap) {
        // switch heap to external memory - use all memory, except for the reserved space for software image
        gs_mem_heap_set(GS_TYPES_UINT2PTR(EXTERNAL_RAM_ADDR + SWLOAD_LINK_SIZE),
                        (EXTERNAL_RAM_SIZE - SWLOAD_LINK_SIZE));
    }

    if (GS_A3200_SDK) {
        /* Start software watchdog, which takes care of the hardware watchdog */
        gs_a3200_sdk_watchdog_init();

        /* Init LED */
        gs_a3200_led_init();
        gs_a3200_led_on(GS_A3200_LED_CPU_OK);
    }

    return GS_OK;
}

void gs_a3200_run(const gs_a3200_hooks_t * hooks)
{
    gs_a3200_hooks = hooks;

    /* Start init task at highest priority */
    gs_thread_create("INIT",
                     (GS_A3200_SDK) ? gs_a3200_sdk_init_task : bsp_init_task,
                     NULL, gs_a3200_get_default_stack_size(), GS_THREAD_PRIORITY_CRITICAL, 0, NULL);

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle task. */
    for (;;) {
        gs_sys_reset(GS_SYS_RESET_NO_MEM);
    }
}
