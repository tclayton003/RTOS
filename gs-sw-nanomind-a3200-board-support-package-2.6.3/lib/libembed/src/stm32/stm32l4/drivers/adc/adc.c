/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/util/error.h>
#include <gs/util/check.h>
#include <gs/util/mutex.h>
#include <gs/embed/stm32/stm32l4/error.h>
#include <gs/embed/stm32/stm32l4/drivers/adc/adc.h>
#include <gs/embed/stm32/stm32l4/drivers/gpio/gpio.h>
#include <stm32l4xx_hal.h>

typedef struct gs_stm32l4_adc_channel_handle {
    //! STM32 adc channel
    uint32_t stm32l4_adc_channel;
    //! STM32 sample time
    gs_stm32l4_adc_sample_time_t sample_time;
    //! Bool true if active
    bool inuse;
} gs_stm32l4_adc_channel_handle_t;

static ADC_HandleTypeDef adc_handle;
static ADC_ChannelConfTypeDef stm_channel_conf;
static gs_mutex_t adc_lock;
static uint32_t voltage_ref_mv;
static float voltage_mv_resolution;

#define NUMBER_OF_CHANNELS 22
static gs_stm32l4_adc_channel_handle_t channels[NUMBER_OF_CHANNELS];

gs_error_t gs_stm32l4_adc_init(uint16_t adc_clock_prescaler)
{
    uint32_t clock_prescaler_val;
    switch(adc_clock_prescaler) {
        case 1:
            clock_prescaler_val = ADC_CLOCK_ASYNC_DIV1;
            break;
        case 2:
            clock_prescaler_val = ADC_CLOCK_ASYNC_DIV2;
            break;
        case 4:
            clock_prescaler_val = ADC_CLOCK_ASYNC_DIV4;
            break;
        case 8:
            clock_prescaler_val = ADC_CLOCK_ASYNC_DIV8;
            break;
        case 16:
            clock_prescaler_val = ADC_CLOCK_ASYNC_DIV16;
            break;
        case 32:
            clock_prescaler_val = ADC_CLOCK_ASYNC_DIV32;
            break;
        case 64:
            clock_prescaler_val = ADC_CLOCK_ASYNC_DIV64;
            break;
        case 128:
            clock_prescaler_val = ADC_CLOCK_ASYNC_DIV128;
            break;
        case 256:
            clock_prescaler_val = ADC_CLOCK_ASYNC_DIV256;
            break;
        default:
            return GS_ERROR_NOT_SUPPORTED;
    }
    gs_error_t res = gs_mutex_create(&adc_lock);
    if (res != GS_OK) {
        return res;
    }

    __HAL_RCC_ADC_CLK_ENABLE();
    __HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_SYSCLK);

    adc_handle.Instance = ADC1;
    adc_handle.Init.ClockPrescaler        = clock_prescaler_val;           /* Asynchronous clock mode, input ADC clock not divided */
    adc_handle.Init.Resolution            = ADC_RESOLUTION_12B;            /* 12-bit resolution for converted data */
    adc_handle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;           /* Right-alignment for converted data */
    adc_handle.Init.ScanConvMode          = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
    adc_handle.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;           /* EOC flag picked-up to indicate conversion end */
    adc_handle.Init.LowPowerAutoWait      = DISABLE;                       /* Auto-delayed conversion feature disabled */
    adc_handle.Init.ContinuousConvMode    = DISABLE;                       /* Continuous mode disabled to have only 1 conversion at each conversion trig */
    adc_handle.Init.NbrOfConversion       = 1;                             /* Parameter discarded because sequencer is disabled */
    adc_handle.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
    adc_handle.Init.NbrOfDiscConversion   = 1;                             /* Parameter discarded because sequencer is disabled */
    adc_handle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;            /* Software start to trig the 1st conversion manually, without external event */
    adc_handle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE; /* Parameter discarded because software trigger chosen */
    adc_handle.Init.DMAContinuousRequests = DISABLE;                       /* DMA one-shot mode selected (not applied to this example) */
    adc_handle.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;      /* DR register is overwritten with the last conversion result in case of overrun */
    adc_handle.Init.OversamplingMode      = DISABLE;                       /* No oversampling */

    HAL_StatusTypeDef HAL_res = HAL_ADC_Init(&adc_handle);
    if (HAL_res != HAL_OK) {
        return gs_hal_status_to_error(HAL_res);
    }
    HAL_res = HAL_ADCEx_Calibration_Start(&adc_handle, ADC_SINGLE_ENDED);
    if (HAL_res != HAL_OK) {
        return gs_hal_status_to_error(HAL_res);
    }
    /* Set common conf */
    stm_channel_conf.Rank         = ADC_REGULAR_RANK_1;            /* Rank of sampled channel number */
    stm_channel_conf.SingleDiff   = ADC_SINGLE_ENDED;              /* Single-ended input channel */
    stm_channel_conf.OffsetNumber = ADC_OFFSET_NONE;               /* No offset subtraction */
    stm_channel_conf.Offset = 0;                                   /* Parameter discarded because offset correction is disabled */

    gs_stm32l4_adc_channel_config_t channel_conf = {
        .channel = GS_STM32L4_ADC_CHANNEL_TEMPSENSOR,
        .sample_time = ADC_SAMPLETIME_247CYCLES_5
    };

    res = gs_stm32l4_adc_config_channel(&channel_conf);
    if (res != GS_OK) {
        return res;
    }

    channel_conf.channel = GS_STM32L4_ADC_CHANNEL_VREFINT;
    res = gs_stm32l4_adc_config_channel(&channel_conf);
    if (res != GS_OK) {
        return res;
    }

    /* Measure Vref and compare to fabric calibration and calculate voltage resolution */
    return gs_stm32l4_adc_update_vref();
}

gs_error_t gs_stm32l4_adc_update_vref(void)
{
    uint32_t vrefint_data;
    /* Measuring and averaging over 128 samples internal V ref  */
    gs_error_t res = gs_stm32l4_adc_get_sample_avr(GS_STM32L4_ADC_CHANNEL_VREFINT, 128, &vrefint_data);
    if (res != GS_OK) {
        return res;
    }

    voltage_ref_mv = __HAL_ADC_CALC_VREFANALOG_VOLTAGE(vrefint_data, ADC_RESOLUTION_12B);
    /* Update global variable */
    voltage_mv_resolution = (float)voltage_ref_mv / 4095; // ADC resolution is 12 bit (4095 = 2^12 - 1)

    return GS_OK;
}

gs_error_t gs_stm32l4_adc_config_channel(gs_stm32l4_adc_channel_config_t * channel_conf)
{
    if (!adc_handle.Instance) {
        return GS_ERROR_PERM;
    }
    gs_stm32l4_adc_channel_handle_t * handle = &channels[channel_conf->channel];
    switch (channel_conf->channel) {
        case GS_STM32L4_ADC_CHANNEL_0: handle->stm32l4_adc_channel = ADC_CHANNEL_0; break;
        case GS_STM32L4_ADC_CHANNEL_1: handle->stm32l4_adc_channel = ADC_CHANNEL_1; break;
        case GS_STM32L4_ADC_CHANNEL_2: handle->stm32l4_adc_channel = ADC_CHANNEL_2; break;
        case GS_STM32L4_ADC_CHANNEL_3: handle->stm32l4_adc_channel = ADC_CHANNEL_3; break;
        case GS_STM32L4_ADC_CHANNEL_4: handle->stm32l4_adc_channel = ADC_CHANNEL_4; break;
        case GS_STM32L4_ADC_CHANNEL_5: handle->stm32l4_adc_channel = ADC_CHANNEL_5; break;
        case GS_STM32L4_ADC_CHANNEL_6: handle->stm32l4_adc_channel = ADC_CHANNEL_6; break;
        case GS_STM32L4_ADC_CHANNEL_7: handle->stm32l4_adc_channel = ADC_CHANNEL_7; break;
        case GS_STM32L4_ADC_CHANNEL_8: handle->stm32l4_adc_channel = ADC_CHANNEL_8; break;
        case GS_STM32L4_ADC_CHANNEL_9: handle->stm32l4_adc_channel = ADC_CHANNEL_9; break;
        case GS_STM32L4_ADC_CHANNEL_10: handle->stm32l4_adc_channel = ADC_CHANNEL_10; break;
        case GS_STM32L4_ADC_CHANNEL_11: handle->stm32l4_adc_channel = ADC_CHANNEL_11; break;
        case GS_STM32L4_ADC_CHANNEL_12: handle->stm32l4_adc_channel = ADC_CHANNEL_12; break;
        case GS_STM32L4_ADC_CHANNEL_13: handle->stm32l4_adc_channel = ADC_CHANNEL_13; break;
        case GS_STM32L4_ADC_CHANNEL_14: handle->stm32l4_adc_channel = ADC_CHANNEL_14; break;
        case GS_STM32L4_ADC_CHANNEL_15: handle->stm32l4_adc_channel = ADC_CHANNEL_15; break;
        case GS_STM32L4_ADC_CHANNEL_16: handle->stm32l4_adc_channel = ADC_CHANNEL_16; break;
        case GS_STM32L4_ADC_CHANNEL_17: handle->stm32l4_adc_channel = ADC_CHANNEL_17; break;
        case GS_STM32L4_ADC_CHANNEL_18: handle->stm32l4_adc_channel = ADC_CHANNEL_18; break;
        case GS_STM32L4_ADC_CHANNEL_VREFINT: handle->stm32l4_adc_channel = ADC_CHANNEL_VREFINT; break;
        case GS_STM32L4_ADC_CHANNEL_TEMPSENSOR: handle->stm32l4_adc_channel = ADC_CHANNEL_TEMPSENSOR;break;
        case GS_STM32L4_ADC_CHANNEL_VBAT: handle->stm32l4_adc_channel = ADC_CHANNEL_VBAT; break;
        default: return GS_ERROR_HANDLE;
    }
    handle->sample_time = channel_conf->sample_time;
    stm_channel_conf.Channel = handle->stm32l4_adc_channel;
    stm_channel_conf.SamplingTime = channel_conf->sample_time;
    HAL_StatusTypeDef HAL_res = HAL_ADC_ConfigChannel(&adc_handle, &stm_channel_conf);
    if (HAL_res != HAL_OK) {
        return gs_hal_status_to_error(HAL_res);
    }
    handle->inuse = true;

    if (channel_conf->channel < GS_STM32L4_ADC_CHANNEL_VREFINT) {
        gs_stm32l4_gpio_conf_t conf;
        conf.mode = GPIO_MODE_ANALOG_ADC_CONTROL;
        conf.pull = GPIO_NOPULL;
        return gs_stm32l4_gpio_init(channel_conf->gpio, &conf);
    } else {
        return GS_OK;
    }
}

static gs_error_t gs_stm32l4_adc_sample(gs_stm32l4_adc_channel_t channel, uint32_t * sample)
{
    gs_stm32l4_adc_channel_handle_t * handle = &channels[channel];
    if (!handle->inuse) {
        return GS_ERROR_HANDLE;
    }
    static gs_stm32l4_adc_channel_t last_channel = -1;
    HAL_StatusTypeDef HAL_res = GS_OK;
    if (last_channel != channel) {
        stm_channel_conf.Channel = handle->stm32l4_adc_channel;
        stm_channel_conf.SamplingTime = handle->sample_time;
        HAL_res = HAL_ADC_ConfigChannel(&adc_handle, &stm_channel_conf);
        last_channel = channel;
    }
    if (HAL_res == HAL_OK) {
        HAL_res = HAL_ADC_Start(&adc_handle);
        if (HAL_res == HAL_OK) {
            HAL_res = HAL_ADC_PollForConversion(&adc_handle, 10);
            if (HAL_res == HAL_OK) {
                *sample = HAL_ADC_GetValue(&adc_handle);
            }
        }
    }

    return gs_hal_status_to_error(HAL_res);
}

gs_error_t gs_stm32l4_adc_get_raw_sample(gs_stm32l4_adc_channel_t channel, uint32_t * sample)
{
    gs_error_t res = gs_mutex_lock(adc_lock);
    if (res != GS_OK) {
        return res;
    }
    res = gs_stm32l4_adc_sample(channel, sample);
    gs_mutex_unlock(adc_lock);
    return res;
}

gs_error_t gs_stm32l4_adc_get_sample_avr(gs_stm32l4_adc_channel_t channel,  uint8_t samples, uint32_t * sample_avr)
{
    GS_CHECK_ARG((samples == 1) || ((samples % 2) == 0));
    GS_CHECK_ARG(sample_avr != NULL);

    uint32_t raw_sample = 0;

    gs_error_t res = gs_mutex_lock(adc_lock);
    if (res != GS_OK) {
        return res;
    }
    /* Sample dummy sample due to an in erratta described problem with ADC
       If the delay between to consecutive conversions is over 1 ms, the first result is bad */
    gs_stm32l4_adc_sample(channel, &raw_sample);
    /* Take and sum new samples */
    uint32_t samples_sum = 0;
    for (uint8_t i = 0; i < samples; i++) {
        res = gs_stm32l4_adc_sample(channel, &raw_sample);
        if (res != GS_OK) {
            break;
        }
        samples_sum = samples_sum + raw_sample;
    }
    gs_mutex_unlock(adc_lock);

    *sample_avr = samples_sum / samples; // Dividing by samples

    return res;
}

gs_error_t gs_stm32l4_adc_get_voltage(gs_stm32l4_adc_channel_t channel, uint8_t samples, float * voltage_mv)
{
    uint32_t sample_avr;
    gs_error_t res = gs_stm32l4_adc_get_sample_avr(channel, samples, &sample_avr);
    if (res != GS_OK) {
        return res;
    }
    /* Calculates average and resulting voltage */
    *voltage_mv = sample_avr * voltage_mv_resolution;
    return res;
}

gs_error_t gs_stm32l4_adc_get_temperature(uint8_t samples, int * temperature)
{
    uint32_t sample_avr;
    gs_error_t res = gs_stm32l4_adc_get_sample_avr(GS_STM32L4_ADC_CHANNEL_TEMPSENSOR, samples, &sample_avr);
    if (res != GS_OK) {
        return res;
    }
    *temperature = __HAL_ADC_CALC_TEMPERATURE((voltage_mv_resolution * 4095), sample_avr, ADC_RESOLUTION_12B);
    return GS_OK;
}

