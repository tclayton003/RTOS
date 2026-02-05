/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/asf/samc/drivers/clock.h>
#include <clock.h> // asf

// NOTE: logging/stdout NOT allowed
gs_error_t gs_clock_configure_internal_48_mhz(void)
{
    // Configure internal OSC48 to run with defaults - 4MHz
    {
        struct system_clock_source_osc48m_config config;
        system_clock_source_osc48m_get_config_defaults(&config);

        config.on_demand = false;

        system_clock_source_osc48m_set_config(&config);
        system_clock_source_enable(SYSTEM_CLOCK_SOURCE_OSC48M); // not really needed
    }

    // Configure GCLK[1] to output 1MHz, using OSC48M as source
    {
        struct system_gclk_gen_config config;
        system_gclk_gen_get_config_defaults(&config);

        config.source_clock    = SYSTEM_CLOCK_SOURCE_OSC48M;
        config.division_factor = 4;
        config.output_enable = false;
        system_gclk_gen_set_config(GCLK_GENERATOR_1, &config);
        system_gclk_gen_enable(GCLK_GENERATOR_1);
    }

    // Link GCLK[1] output to DPLL input
    {
        struct system_gclk_chan_config config;
        system_gclk_chan_get_config_defaults(&config);

        config.source_generator = GCLK_GENERATOR_1;
        system_gclk_chan_set_config(OSCCTRL_GCLK_ID_FDPLL, &config);
        system_gclk_chan_enable(OSCCTRL_GCLK_ID_FDPLL);
    }

    // Configure DPLL to output 48 MHz clock
    {
        struct system_clock_source_dpll_config config;
        system_clock_source_dpll_get_config_defaults(&config);
        config.reference_clock = SYSTEM_CLOCK_SOURCE_DPLL_REFERENCE_CLOCK_GCLK;
        config.reference_divider = 1;
        config.reference_frequency = 1000000; //Gen[1] has been divided to 1 MHz
        config.output_frequency = 48000000;
        system_clock_source_dpll_set_config(&config);
    }

    /* Configure the NVM controller according to supply voltage and CPU speed */
    system_flash_set_waitstates(1);

    // Enable DPLL
    system_clock_source_enable(SYSTEM_CLOCK_SOURCE_DPLL);

    // Link DPLL output to GLK[0] input - master/main clock
    {
        struct system_gclk_gen_config config;
        system_gclk_gen_get_config_defaults(&config);

        config.source_clock    = SYSTEM_CLOCK_SOURCE_DPLL;
        config.division_factor = 1;
        config.output_enable = false;
        system_gclk_gen_set_config(GCLK_GENERATOR_0, &config);
    }

    // Wait for clock to be ready
    while(!system_clock_source_is_ready(SYSTEM_CLOCK_SOURCE_DPLL));

    return GS_OK;
}
