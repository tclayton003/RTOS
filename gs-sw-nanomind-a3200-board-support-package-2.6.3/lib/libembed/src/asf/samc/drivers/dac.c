/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/asf/samc/drivers/dac.h>
#include <sam0/drivers/dac/dac.h> // libasf
#include <gs/util/log.h>
#include <gs/embed/asf/samc/convert_asf_status_code.h>
#include <compiler.h> // MCU definitions

static struct dac_module dac_instance;
static uint16_t dac_value = 0;

gs_error_t gs_dac_init(int clock_source, uint16_t initialValue)
{
    // configure DAC
    {
        struct dac_config config;
        dac_get_config_defaults(&config);

        config.reference      = DAC_REFERENCE_INT1V; // this is internal bandgap reference - not 1V
        config.output         = DAC_OUTPUT_EXTERNAL;
        config.clock_source   = clock_source;

        enum status_code status = dac_init(&dac_instance, DAC, &config);
        if (status != STATUS_OK) {
            log_error("%s: dac_init() failed, returned status: %d",
                      __FUNCTION__, status);
            return gs_convert_asf_status_code(status);
        }
    }

    // configure channel - has not effect on SAMC21
    {
        struct dac_chan_config config;
        dac_chan_get_config_defaults(&config);

        dac_chan_set_config(&dac_instance, DAC_CHANNEL_0, &config);
    }

    dac_chan_enable(&dac_instance, DAC_CHANNEL_0);
    dac_enable(&dac_instance);

    gs_dac_write(initialValue);

    return 0;
}

gs_error_t gs_dac_write(uint16_t value)
{
    enum status_code status = dac_chan_write(&dac_instance, DAC_CHANNEL_0, value);
    if (status != STATUS_OK) {
        log_error("%s: dac_chan_write() failed, returned status code=%d",
                  __FUNCTION__, status);
        return gs_convert_asf_status_code(status);
    }

    dac_value = value; // shadow - can't read value back from DAC

#if GS_DAC_DEBUG
    Dac * const dac_module = dac_instance.hw;
    log_info("CTRLA: 0x%u", dac_module->CTRLA.reg);
    log_info("CTRLB: 0x%u", dac_module->CTRLB.reg);
    log_info("EVCTRL: 0x%u", dac_module->EVCTRL.reg);
    log_info("INTENCLR: 0x%u", dac_module->INTENCLR.reg);
    log_info("INTENSET: 0x%u", dac_module->INTENSET.reg);
    log_info("INTFLAG: 0x%u", dac_module->INTFLAG.reg);
    log_info("STATUS: 0x%u", dac_module->STATUS.reg);
    log_info("DATA: 0x%u", dac_module->DATA.reg);
    log_info("DATABUF: 0x%u", dac_module->DATABUF.reg);
    log_info("SYNCBUSY: 0x%u", dac_module->SYNCBUSY.reg);
    log_info("DBGCTRL: 0x%u", dac_module->DBGCTRL.reg);
#endif

    return GS_OK;
}

uint16_t gs_dac_read(void)
{
    return dac_value;
}
