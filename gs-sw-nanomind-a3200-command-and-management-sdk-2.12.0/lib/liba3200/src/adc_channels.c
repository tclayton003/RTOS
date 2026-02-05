/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
 * ADC channel driver
 */

#include <gs/a3200/adc_channels.h>
#include "local.h"
#include <adcifa.h>

static adcifa_sequencer_opt_t adcifa_sequence_opt;

static SemaphoreHandle_t adc_lock;

void gs_a3200_adc_channels_init(void)
{
    adc_lock = xSemaphoreCreateMutex();

    adcifa_opt_t adc_config_t;
    adcifa_sequencer_conversion_opt_t adcifa_sequence_conversion_opt[GS_A3200_ADC_NCHANS];

    /* External ADC inputs */
#if GS_A3200_BOARD_REVISION >= 3
    gpio_enable_module_pin(AVR32_ADCIN0_PIN, AVR32_ADCIN0_FUNCTION);
    gpio_enable_module_pin(AVR32_ADCIN1_PIN, AVR32_ADCIN1_FUNCTION);
    gpio_enable_module_pin(AVR32_ADCIN2_PIN, AVR32_ADCIN2_FUNCTION);
    gpio_enable_module_pin(AVR32_ADCIN3_PIN, AVR32_ADCIN3_FUNCTION);
    gpio_enable_module_pin(AVR32_ADCIN4_PIN, AVR32_ADCIN4_FUNCTION);
    gpio_enable_module_pin(AVR32_ADCIN5_PIN, AVR32_ADCIN5_FUNCTION);
    gpio_enable_module_pin(AVR32_ADCIN6_PIN, AVR32_ADCIN6_FUNCTION);
    gpio_enable_module_pin(AVR32_ADCIN7_PIN, AVR32_ADCIN7_FUNCTION);
#endif

    /* Onboard Current measurements */
#if GS_A3200_BOARD_REVISION >= 3
    gpio_enable_module_pin(AVR32_ADCIN11_PIN, AVR32_ADCIN11_FUNCTION);
#endif
    gpio_enable_module_pin(AVR32_ADCIN12_PIN, AVR32_ADCIN12_FUNCTION);
    gpio_enable_module_pin(AVR32_ADCIN13_PIN, AVR32_ADCIN13_FUNCTION);
    gpio_enable_module_pin(AVR32_ADCIN14_PIN, AVR32_ADCIN14_FUNCTION);

    /* Reference
     * Note that 2.5 V external reference is routed both to REFP and REF0 */
    gpio_enable_module_pin(AVR32_ADCREFP_PIN, AVR32_ADCREFP_FUNCTION);
    gpio_enable_module_pin(AVR32_ADCREFN_PIN, AVR32_ADCREFN_FUNCTION);
#if GS_A3200_BOARD_REVISION >= 3
    gpio_enable_module_pin(AVR32_ADCREF0_PIN, AVR32_ADCREF0_FUNCTION);
#endif

    /* Configure the ADC */
    adc_config_t.frequency = 200000;
    adc_config_t.reference_source = ADCIFA_ADCREF0;
    adc_config_t.sample_and_hold_disable = false;
    adc_config_t.single_sequencer_mode = true;
    adc_config_t.free_running_mode_enable = false;
    adc_config_t.sleep_mode_enable = true;
    adc_config_t.mux_settle_more_time = true;

    /* Get ADCIFA Factory Configuration */
    adcifa_get_calibration_data(&AVR32_ADCIFA, &adc_config_t);

    /* Calibrate offset first */
    adcifa_calibrate_offset(&AVR32_ADCIFA, &adc_config_t, sysclk_get_peripheral_bus_hz(&AVR32_ADCIFA));

    /* Configure ADCIFA core */
    adcifa_configure(&AVR32_ADCIFA, &adc_config_t, sysclk_get_peripheral_bus_hz(&AVR32_ADCIFA));

    /* Sequencer configuration */
    adcifa_sequence_opt.convnb = GS_A3200_ADC_NCHANS;
    adcifa_sequence_opt.resolution = ADCIFA_SRES_12B;
    adcifa_sequence_opt.trigger_selection = ADCIFA_TRGSEL_SOFT;
    adcifa_sequence_opt.start_of_conversion = ADCIFA_SOCB_ALLSEQ;
    adcifa_sequence_opt.sh_mode = ADCIFA_SH_MODE_DYNAMIC;
    adcifa_sequence_opt.half_word_adjustment = ADCIFA_HWLA_NOADJ;
    adcifa_sequence_opt.software_acknowledge = ADCIFA_SA_NO_EOS_SOFTACK;

    /* ADCIFA conversions for sequencer 0*/
#if GS_A3200_BOARD_REVISION >= 3
    adcifa_sequence_conversion_opt[0].channel_p = AVR32_ADCIFA_INP_ADCIN0;
    adcifa_sequence_conversion_opt[0].channel_n = AVR32_ADCIFA_INN_GNDANA;
    adcifa_sequence_conversion_opt[0].gain = ADCIFA_SHG_1;
    adcifa_sequence_conversion_opt[1].channel_p = AVR32_ADCIFA_INP_ADCIN1;
    adcifa_sequence_conversion_opt[1].channel_n = AVR32_ADCIFA_INN_GNDANA;
    adcifa_sequence_conversion_opt[1].gain = ADCIFA_SHG_1;
    adcifa_sequence_conversion_opt[2].channel_p = AVR32_ADCIFA_INP_ADCIN2;
    adcifa_sequence_conversion_opt[2].channel_n = AVR32_ADCIFA_INN_GNDANA;
    adcifa_sequence_conversion_opt[2].gain = ADCIFA_SHG_1;
    adcifa_sequence_conversion_opt[3].channel_p = AVR32_ADCIFA_INP_ADCIN3;
    adcifa_sequence_conversion_opt[3].channel_n = AVR32_ADCIFA_INN_GNDANA;
    adcifa_sequence_conversion_opt[3].gain = ADCIFA_SHG_1;
    adcifa_sequence_conversion_opt[4].channel_p = AVR32_ADCIFA_INP_ADCIN4;
    adcifa_sequence_conversion_opt[4].channel_n = AVR32_ADCIFA_INN_GNDANA;
    adcifa_sequence_conversion_opt[4].gain = ADCIFA_SHG_1;
    adcifa_sequence_conversion_opt[5].channel_p = AVR32_ADCIFA_INP_ADCIN5;
    adcifa_sequence_conversion_opt[5].channel_n = AVR32_ADCIFA_INN_GNDANA;
    adcifa_sequence_conversion_opt[5].gain = ADCIFA_SHG_1;
    adcifa_sequence_conversion_opt[6].channel_p = AVR32_ADCIFA_INP_ADCIN6;
    adcifa_sequence_conversion_opt[6].channel_n = AVR32_ADCIFA_INN_GNDANA;
    adcifa_sequence_conversion_opt[6].gain = ADCIFA_SHG_1;
    adcifa_sequence_conversion_opt[7].channel_p = AVR32_ADCIFA_INP_ADCIN7;
    adcifa_sequence_conversion_opt[7].channel_n = AVR32_ADCIFA_INN_GNDANA;
    adcifa_sequence_conversion_opt[7].gain = ADCIFA_SHG_1;
    adcifa_sequence_conversion_opt[8].channel_p = AVR32_ADCIFA_INP_GNDANA;
    adcifa_sequence_conversion_opt[8].channel_n = AVR32_ADCIFA_INN_ADCIN11;
    adcifa_sequence_conversion_opt[8].gain = ADCIFA_SHG_1;
    adcifa_sequence_conversion_opt[9].channel_p = AVR32_ADCIFA_INP_GNDANA;
    adcifa_sequence_conversion_opt[9].channel_n = AVR32_ADCIFA_INN_ADCIN12;
    adcifa_sequence_conversion_opt[9].gain = ADCIFA_SHG_1;
    adcifa_sequence_conversion_opt[10].channel_p = AVR32_ADCIFA_INP_GNDANA;
    adcifa_sequence_conversion_opt[10].channel_n = AVR32_ADCIFA_INN_ADCIN13;
    adcifa_sequence_conversion_opt[10].gain = ADCIFA_SHG_1;
    adcifa_sequence_conversion_opt[11].channel_p = AVR32_ADCIFA_INP_GNDANA;
    adcifa_sequence_conversion_opt[11].channel_n = AVR32_ADCIFA_INN_ADCIN14;
    adcifa_sequence_conversion_opt[11].gain = ADCIFA_SHG_1;
#else
    adcifa_sequence_conversion_opt[0].channel_p = AVR32_ADCIFA_INP_GNDANA;
    adcifa_sequence_conversion_opt[0].channel_n = AVR32_ADCIFA_INN_ADCIN12;
    adcifa_sequence_conversion_opt[0].gain = ADCIFA_SHG_1;
    adcifa_sequence_conversion_opt[1].channel_p = AVR32_ADCIFA_INP_GNDANA;
    adcifa_sequence_conversion_opt[1].channel_n = AVR32_ADCIFA_INN_ADCIN13;
    adcifa_sequence_conversion_opt[1].gain = ADCIFA_SHG_1;
    adcifa_sequence_conversion_opt[2].channel_p = AVR32_ADCIFA_INP_GNDANA;
    adcifa_sequence_conversion_opt[2].channel_n = AVR32_ADCIFA_INN_ADCIN14;
    adcifa_sequence_conversion_opt[2].gain = ADCIFA_SHG_1;
#endif

    /* Configure ADCIFA sequencer 0 */
    adcifa_configure_sequencer(&AVR32_ADCIFA, 0, &adcifa_sequence_opt, adcifa_sequence_conversion_opt);
}

gs_error_t gs_a3200_adc_channels_sample(int16_t *adc_values)
{
    if (adc_values == 0)
        return GS_ERROR_ARG;

    xSemaphoreTake(adc_lock, 1000);

    adcifa_start_sequencer(&AVR32_ADCIFA, 0);

    uint32_t counter = 1000;
    while (!(adcifa_get_values_from_sequencer(&AVR32_ADCIFA, 0, &adcifa_sequence_opt, adc_values) == ADCIFA_STATUS_COMPLETED) && counter) {
        delay_us(1);
        counter--;
    }
    if (counter == 0) {
        log_error("ADC Timeout");
        xSemaphoreGive(adc_lock);
        return GS_ERROR_TIMEOUT;
    }

    ADCIFA_clear_eos_sequencer_0();

    /* Account for inverted sign on the INPN channels */
    for (int i = 8; i < 12; i++) {
        adc_values[i] = -adc_values[i];
    }

    xSemaphoreGive(adc_lock);

    return GS_OK;
}
