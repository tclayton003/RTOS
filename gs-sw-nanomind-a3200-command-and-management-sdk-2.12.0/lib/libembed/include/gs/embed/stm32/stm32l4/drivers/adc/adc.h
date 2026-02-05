#ifndef LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_ADC_ADC_H_
#define LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_ADC_ADC_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   @brief STM32L4 ADC API

   This API includes initialization routine of ADC and both raw sample functions
   together with voltage conversion and averaging functions.
*/

#include <stm32l4xx_hal.h>
#include <gs/util/drivers/gpio/gpio.h>

/**
   ADC channels.
*/
typedef enum gs_stm32l4_adc_channel {
    GS_STM32L4_ADC_CHANNEL_0 = 0,
    GS_STM32L4_ADC_CHANNEL_1 = 1,
    GS_STM32L4_ADC_CHANNEL_2 = 2,
    GS_STM32L4_ADC_CHANNEL_3 = 3,
    GS_STM32L4_ADC_CHANNEL_4 = 4,
    GS_STM32L4_ADC_CHANNEL_5 = 5,
    GS_STM32L4_ADC_CHANNEL_6 = 6,
    GS_STM32L4_ADC_CHANNEL_7 = 7,
    GS_STM32L4_ADC_CHANNEL_8 = 8,
    GS_STM32L4_ADC_CHANNEL_9 = 9,
    GS_STM32L4_ADC_CHANNEL_10 = 10,
    GS_STM32L4_ADC_CHANNEL_11 = 11,
    GS_STM32L4_ADC_CHANNEL_12 = 12,
    GS_STM32L4_ADC_CHANNEL_13 = 13,
    GS_STM32L4_ADC_CHANNEL_14 = 14,
    GS_STM32L4_ADC_CHANNEL_15 = 15,
    GS_STM32L4_ADC_CHANNEL_16 = 16,
    GS_STM32L4_ADC_CHANNEL_17 = 17,
    GS_STM32L4_ADC_CHANNEL_18 = 18,
    GS_STM32L4_ADC_CHANNEL_VREFINT = 19,
    GS_STM32L4_ADC_CHANNEL_TEMPSENSOR = 20,
    GS_STM32L4_ADC_CHANNEL_VBAT = 21,
} gs_stm32l4_adc_channel_t;

/**
   ADC sample time.
   Number of prescaled ADC clock cycles for multiplexing to right source and charging internal capacitor before sampling.
   Can be set for each channel.
*/
typedef enum gs_stm32l4_adc_sample_time {
    //! 2.5 clock cycles
    GS_STM32L4_SAMPLE_TIME_2_5_CYCLES = ADC_SAMPLETIME_2CYCLES_5,
    //! 6.5 clock cycles
    GS_STM32L4_SAMPLE_TIME_6_5_CYCLES = ADC_SAMPLETIME_6CYCLES_5,
    //! 12.5 clock cycles
    GS_STM32L4_SAMPLE_TIME_12_5_CYCLES = ADC_SAMPLETIME_12CYCLES_5,
    //! 24.5 clock cycles
    GS_STM32L4_SAMPLE_TIME_24_5_CYCLES = ADC_SAMPLETIME_24CYCLES_5,
    //! 47.5 clock cycles
    GS_STM32L4_SAMPLE_TIME_47_5_CYCLES = ADC_SAMPLETIME_47CYCLES_5,
    //! 92.5 clock cycles
    GS_STM32L4_SAMPLE_TIME_92_5_CYCLES = ADC_SAMPLETIME_92CYCLES_5,
    //! 247.5 clock cycles
    GS_STM32L4_SAMPLE_TIME_247_5_CYCLES = ADC_SAMPLETIME_247CYCLES_5,
    //! 640.5 clock cycles
    GS_STM32L4_SAMPLE_TIME_640_5_CYCLES = ADC_SAMPLETIME_640CYCLES_5,
} gs_stm32l4_adc_sample_time_t;

/**
   ADC channel configuration.
*/
typedef struct gs_stm32l4_adc_channel_config {
    //! GPIO number (must match chosen channel, see datasheet for MCU)
    gs_gpio_t gpio;
    //! Channel number
    gs_stm32l4_adc_channel_t channel;
    //! Sample time
    gs_stm32l4_adc_sample_time_t sample_time;
} gs_stm32l4_adc_channel_config_t;

/**
   Init ADC
   @param[in] adc_clock_prescaler Sets the prescaler from system clock (must be 2^n [1, 2, 4, ... , 256])
   @return_gs_error_t
*/
gs_error_t gs_stm32l4_adc_init(uint16_t adc_clock_prescaler);

/**
   Measure and updates Vref
   It recalculates the voltage resolution of the ADC
   @note averaging over 128 samples
   @return_gs_error_t
 */
gs_error_t gs_stm32l4_adc_update_vref(void);

/**
   Configures ADC channel, including pin configuration.
   @note First, ADC has to be initialized by gs_stm32l4_adc_init()
   @param[in]  channel_conf Configuration for channel
   @return_gs_error_t
*/
gs_error_t gs_stm32l4_adc_config_channel(gs_stm32l4_adc_channel_config_t * channel_conf);

/**
   Reads one sample from specific ADC channel
   @note First, channel needs to be configured by gs_stm32l4_adc_config_channel()
   @param[in]  channel Identifier for channel to read
   @param[out] sample Output sample pointer
   @return_gs_error_t
*/
gs_error_t gs_stm32l4_adc_get_raw_sample(gs_stm32l4_adc_channel_t channel, uint32_t * sample);

/**
   Reads raw samples and makes an average
   @note First, channel needs to be configured by gs_stm32l4_adc_config_channel()
   @param[in]  channel Identifier for channel to read
   @param[in]  samples Number of samples in filter (must be 2^n [1, 2, 4, ... , 128])
   @param[out] sample_avr Output averaged sample pointer
   @return_gs_error_t
*/
gs_error_t gs_stm32l4_adc_get_sample_avr(gs_stm32l4_adc_channel_t channel,  uint8_t samples, uint32_t * sample_avr);

/**
   Reads and converts samples to voltage from specific ADC channel
   @note First, channel needs to be configured by gs_stm32l4_adc_config_channel()
   @param[in]  channel Identifier for channel to read
   @param[in]  samples Number of samples in filter (must be 2^n [1, 2, 4, ... , 128])
   @param[out] voltage_mv Output voltage pointer
   @return_gs_error_t
*/
gs_error_t gs_stm32l4_adc_get_voltage(gs_stm32l4_adc_channel_t channel, uint8_t samples, float * voltage_mv);

/**
   Reads the internal temperature sensor
   @note channel is configured in gs_stm32l4_adc_init(), so no configuration is needed
   @param[in]  samples Number of samples in filter (must be 2^n [1, 2, 4, ... , 128])
   @param[out] temperature Output temperature pointer [Celsius]
   @return_gs_error_t
*/
gs_error_t gs_stm32l4_adc_get_temperature(uint8_t samples, int * temperature);

#endif /* LIB_LIBEMBED_INCLUDE_GS_EMBED_STM32_STM32L4_DRIVERS_ADC_ADC_H_ */
