/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <compiler.h>
#include <component/dsu.h>
#include <gs/util/gosh/command.h>
#include <malloc.h>
#include <gclk.h> // asf
#include <clock_feature.h> // asf
#include <gs/util/string.h>
#include <gs/util/log.h>
#include <gs/util/time.h>
#include <gs/util/thread.h>
#include <gs/util/delay.h>
#include <sam0/drivers/nvm/nvm.h>
#include <sam0/drivers/system/reset/reset_sam_c/reset.h>
#include <sam0/drivers/bod/bod.h>
#include <gs/embed/freertos.h>
#include <gs/embed/asf/samc/mcu.h>

//#include "param/param_store_flash.h"

static int cmd_show_did(gs_command_context_t * ctx)
{
    Dsu * dsu = DSU;
    DSU_DID_Type did = dsu->DID;

    printf("Device Select: %u, Revision Number: %u (%c), Die Number: %u, Series: %u, Family: %u, Processor: %u\r\n",
           did.bit.DEVSEL, did.bit.REVISION, ('A' + did.bit.REVISION), did.bit.DIE, did.bit.SERIES, did.bit.FAMILY, did.bit.PROCESSOR);

    return GS_OK;
}

static int cmd_show_clock(gs_command_context_t * ctx)
{
    printf("CPU/Core clock: %" PRIu32 " Hz\r\n", system_cpu_clock_get_hz());

    for (uint8_t i = 0; i < GCLK_GEN_NUM_MSB; ++i) {
        printf("GCLK%u: enabled: %d, Hz: %" PRIu32 "\r\n",
               i,
               system_gclk_gen_is_enabled(i),
               system_gclk_gen_get_hz(i));
    }

    const struct clock_source {
        const char * name;
        int8_t value;
    } clock_sources[] = {
        {"OSC48M", SYSTEM_CLOCK_SOURCE_OSC48M},
        {"OSC32K", SYSTEM_CLOCK_SOURCE_OSC32K},
        {"XOSC", SYSTEM_CLOCK_SOURCE_XOSC},
        {"XOSC32K", SYSTEM_CLOCK_SOURCE_XOSC32K},
        {"ULP32K", SYSTEM_CLOCK_SOURCE_ULP32K},
        {"GCLKIN", SYSTEM_CLOCK_SOURCE_GCLKIN},
        {"GCLKGEN1", SYSTEM_CLOCK_SOURCE_GCLKGEN1},
        {"DPLL", SYSTEM_CLOCK_SOURCE_DPLL},
    };
    for (unsigned int i = 0; i < GS_ARRAY_SIZE(clock_sources); ++i) {
        printf("%-10.10s: enabled: %d, Hz: %" PRIu32 "\r\n",
               clock_sources[i].name,
               system_clock_source_is_ready(clock_sources[i].value),
               system_clock_source_get_hz(clock_sources[i].value));
    }

    return GS_OK;
}

static int cmd_show_fuse(gs_command_context_t * ctx)
{
    // from libasf/sam0/drivers/nvm/nvm.h
    struct nvm_fusebits fusebits;
    enum status_code status = nvm_get_fuses(&fusebits);
    if (status != STATUS_OK) {
        printf("Failed to read fusebites, status=%d\r\n", status);
        return GS_ERROR_IO;
    }

    printf("Bootloader size: %d\r\n", fusebits.bootloader_size);
    printf("EEPROM emulation area size: %d\r\n", fusebits.eeprom_size);
    printf("BODVDD Threshold level at power on: %u\r\n", fusebits.bodvdd_level);
    printf("BODVDD Enable at power on: %d\r\n", fusebits.bodvdd_enable);
    printf("BODVDD Action at power on: %d\r\n", fusebits.bodvdd_action);
    printf("BODVDD Hysteresis at power on: %d\r\n", fusebits.bodvdd_hysteresis);
    printf("WDT Enable at power on: %d\r\n", fusebits.wdt_enable);
    printf("WDT Always-on at power on: %d\r\n", fusebits.wdt_always_on);
    printf("WDT Period at power on: %u\r\n", fusebits.wdt_timeout_period);
    printf("WDT Window mode time-out at power on: %d\r\n", fusebits.wdt_window_timeout);
    printf("WDT Early warning interrupt time offset at power on: %d\r\n", fusebits.wdt_early_warning_offset);
    printf("WDT Window mode enabled at power on: %d\r\n", fusebits.wdt_window_mode_enable_at_poweron);
    printf("NVM Lock bits: 0x%x\r\n", fusebits.lockbits);

    return GS_OK;
}

static int cmd_crash(gs_command_context_t * ctx)
{
    if (ctx->argc < 2) {
        return GS_ERROR_ARG;
    }

    if (strcasecmp("segfault", ctx->argv[1]) == 0) {
        printf("Crash in few mS ......\r\n");
        char * ptr = 0;
        // cppcheck-suppress nullPointer
        char a = *ptr;
        printf("   .. shouldn't get here - read %d\r\n", a);
        return GS_OK;
    }

    if (strcasecmp("stack", ctx->argv[1]) == 0) {
        printf("Stack overrun - crash in few mS ......\r\n");
        char xtreme[4000];
        memset(xtreme, 0, sizeof(xtreme));
        printf("   .. shouldn't get here - didn't crash\r\n");
        return GS_OK;
    }

    if (strcasecmp("alloc", ctx->argv[1]) == 0) {
        unsigned int size;
        if (ctx->argc < 3) {
            return GS_ERROR_ARG;
        }
        if (sscanf(ctx->argv[2], "%u", &size) != 1) {
            return GS_ERROR_ARG;
        }
        printf("Allocate %u bytes ...\r\n", size);
        char * ptr = pvPortMalloc(size);
        printf("Allocation ok: ptr: %p\r\n", ptr);
        return GS_OK;
    }

    return GS_ERROR_ARG;
}

static int cmd_sleep_ms(gs_command_context_t * ctx)
{
    uint32_t value;
    if ((ctx->argc != 2) || (gs_string_to_uint32(ctx->argv[1], &value) != GS_OK)) {
        return GS_ERROR_ARG;
    }

    const uint32_t start_ms = gs_time_rel_ms();
    gs_thread_sleep_ms(value);
    const uint32_t end_ms = gs_time_rel_ms();
    printf("elapsed time: %"PRIu32 " mS\r\n", gs_time_diff_ms(start_ms, end_ms));
    return GS_OK;
}

static int cmd_delay_us(gs_command_context_t * ctx)
{
    uint32_t value;
    if ((ctx->argc != 2) || (gs_string_to_uint32(ctx->argv[1], &value) != GS_OK)) {
        return GS_ERROR_ARG;
    }

    const uint32_t start_ms = gs_time_rel_ms();
    gs_delay_us(value);
    const uint32_t end_ms = gs_time_rel_ms();
    printf("elapsed time: %"PRIu32 " mS\r\n", gs_time_diff_ms(start_ms, end_ms));
    return GS_OK;
}

static int cmd_show_boot_cause(gs_command_context_t * ctx)
{
    enum system_reset_cause cause = system_get_reset_cause();

    const char * text = NULL;
    switch (cause) {
        case SYSTEM_RESET_CAUSE_SOFTWARE:
            text = "The system was last reset by a software reset";
            break;
        case SYSTEM_RESET_CAUSE_WDT:
            text = "The system was last reset by the watchdog timer";
            break;
        case SYSTEM_RESET_CAUSE_EXTERNAL_RESET:
            text = "The system was last reset because the external reset line was pulled low";
            break;
        case SYSTEM_RESET_CAUSE_BODVDD:
            text = "The system was last reset by VDD brown out detector";
            break;
        case SYSTEM_RESET_CAUSE_BODCORE:
            text = "The system was last reset by VDDCORE brown out detector";
            break;
        case SYSTEM_RESET_CAUSE_POR:
            text = "The system was last reset by the POR (Power on reset)";
            break;
    }

    if (text == NULL) {
        text = "Unknown cause";
    }

    printf("Reset cause: %d - %s\r\n", cause, text);
    return GS_OK;
}

#undef ENABLE  // thanks a lot ASF for this define

static int cmd_bod(gs_command_context_t * ctx)
{
    {
        SUPC_BODVDD_Type value;
        value.reg = SUPC->BODVDD.reg;

        printf("BOD VDD: register: 0x%" PRIx32 "\r\n", value.reg);
        printf("BOD VDD: Enable: %d\r\n", value.bit.ENABLE);
        printf("BOD VDD: Hysteresis Enable: %d\r\n", value.bit.HYST);
        printf("BOD VDD: Action when Threshold Crossed: %d\r\n", value.bit.ACTION);
        printf("BOD VDD: Configuration in Standby mode: %d\r\n", value.bit.STDBYCFG);
        printf("BOD VDD: Run during Standby: %d\r\n", value.bit.RUNSTDBY);
        printf("BOD VDD: Configuration in Active mode: %d\r\n", value.bit.ACTCFG);
        printf("BOD VDD: Prescaler Select: %d\r\n", value.bit.PSEL);
        printf("BOD VDD: Threshold Level for VDD: %d\r\n", value.bit.LEVEL);
    }
    {
        SUPC_BODCORE_Type value;
        value.reg = SUPC->BODCORE.reg;

        printf("BOD CORE: register: 0x%" PRIx32 "\r\n", value.reg);
        printf("BOD CORE: Enable: %d\r\n", value.bit.ENABLE);
        printf("BOD CORE: Hysteresis Enable: %d\r\n", value.bit.HYST);
        printf("BOD CORE: Action when Threshold Crossed: %d\r\n", value.bit.ACTION);
        printf("BOD CORE: Configuration in Standby mode: %d\r\n", value.bit.STDBYCFG);
        printf("BOD CORE: Run during Standby: %d\r\n", value.bit.RUNSTDBY);
        printf("BOD CORE: Configuration in Active mode: %d\r\n", value.bit.ACTCFG);
        printf("BOD CORE: Prescaler Select: %d\r\n", value.bit.PSEL);
        printf("BOD CORE: Threshold Level for VDD: %d\r\n", value.bit.LEVEL);
    }

    gs_mcu_check_configuration();

    return GS_OK;
}

static const gs_command_t GS_COMMAND_SUB subcmd_mcu[] = {
    {
        .name = "did",
        .help = "Show Device Identification",
        .handler = cmd_show_did,
    },
    {
        .name = "clock",
        .help = "Show clock configuration",
        .handler = cmd_show_clock,
    },
    {
        .name = "fuse",
        .help = "Show fuse states",
        .handler = cmd_show_fuse,
    },
    {
        .name = "delay_us",
        .help = "Delay X uS",
        .handler = cmd_delay_us,
    },
    {
        .name = "sleep_ms",
        .help = "Sleep X mS",
        .handler = cmd_sleep_ms,
    },
    {
        .name = "bootcause",
        .help = "Show boot cause in GOMspace term, reset cause in MCU terms",
        .handler = cmd_show_boot_cause,
    },
    {
        .name = "bod",
        .help = "Show Brown-Out Detector",
        .handler = cmd_bod,
    },
    {
        .name = "crash",
        .help = "Crash command for testing error handling",
        .usage = "<segfault | stack | alloc <bytes>>",
        .handler = cmd_crash,
        .mode = GS_COMMAND_FLAG_HIDDEN,
    }
};

static const gs_command_t GS_COMMAND_ROOT cmd_mcu[] = {
    {
        .name = "mcu",
        .help = "MCU specific commands",
        .chain = GS_COMMAND_INIT_CHAIN(subcmd_mcu),
    }
};

gs_error_t gs_samc_register_commands(void)
{
    return GS_COMMAND_REGISTER(cmd_mcu);
}
