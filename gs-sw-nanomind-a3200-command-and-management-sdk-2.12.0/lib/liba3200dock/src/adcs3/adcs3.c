/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include "../local.h"
#include <gs/a3200dock/param/host/adcs3_dock.h>
#include <gs/param/host_table.h>
#include <gs/a3200/adc_refs.h>
#include <gs/util/check.h>
#include <gpio.h>

GS_CHECK_STATIC_ASSERT(GS_A3200_DOCK_TABLE_ID == GS_A3200DOCK_ADCS3_DOCK_TABLE_ID, bad_table_id);

/* Define dock current measurement */
#define GS_A3200DOCK_ADCS3_SENSE_RESISTOR_GPS     0.025
#define GS_A3200DOCK_ADCS3_ADC_CURSENSE_GPS       2

#define GS_A3200DOCK_ADCS3_SENSE_RESISTOR_WDE     0.025
#define GS_A3200DOCK_ADCS3_ADC_CURSENSE_WDE       3

/* Define dock ADC switch channels */
#define GS_A3200DOCK_ADCS3_SWITCH_GPS_PIN         AVR32_ADCIN0_PIN
#define GS_A3200DOCK_ADCS3_SWITCH_WDE_PIN         AVR32_ADCIN1_PIN

typedef enum {
    ADCS3_SWITCH_GPS = 0,
    ADCS3_SWITCH_WDE = 1,
} mb_switch_t;

static void adcs3_switch_init(void)
{
    gpio_configure_pin(GS_A3200DOCK_ADCS3_SWITCH_GPS_PIN, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
    gpio_configure_pin(GS_A3200DOCK_ADCS3_SWITCH_WDE_PIN, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
}

static void adcs3_switch_set(mb_switch_t mb_switch, bool state)
{
    switch (mb_switch) {
        case ADCS3_SWITCH_GPS:
            if (state) {
                gpio_set_pin_high(GS_A3200DOCK_ADCS3_SWITCH_GPS_PIN);
                gpio_enable_module_pin(AVR32_USART1_TXD_PIN, AVR32_USART1_TXD_FUNCTION);
            } else {
                gpio_set_pin_low(GS_A3200DOCK_ADCS3_SWITCH_GPS_PIN);
                gpio_configure_pin(AVR32_USART1_TXD_PIN, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
            }
            break;
        case ADCS3_SWITCH_WDE:
            if (state) {
                gpio_set_pin_high(GS_A3200DOCK_ADCS3_SWITCH_WDE_PIN);
            } else {
                gpio_set_pin_low(GS_A3200DOCK_ADCS3_SWITCH_WDE_PIN);
            }
            break;
    }
}

static void adcs3_table_change(uint16_t addr, gs_param_table_instance_t * tinst)
{
    switch(addr) {
        case GS_A3200DOCK_ADCS3_DOCK_GPS_EN: {
            adcs3_switch_set(ADCS3_SWITCH_GPS, GS_A3200DOCK_ADCS3_DOCK_GPS_EN_GET_NC());
            break;
        }
        case GS_A3200DOCK_ADCS3_DOCK_WDE_EN: {
            adcs3_switch_set(ADCS3_SWITCH_WDE, GS_A3200DOCK_ADCS3_DOCK_WDE_EN_GET_NC());
            break;
        }
    }
}

static gs_error_t adcs3_update_adc(int16_t adc[])
{
    GS_A3200DOCK_ADCS3_DOCK_WDE_I_SET(ADC_TO_MV * adc[GS_A3200DOCK_ADCS3_ADC_CURSENSE_WDE] / GS_A3200DOCK_ADCS3_SENSE_RESISTOR_WDE / 100);
    GS_A3200DOCK_ADCS3_DOCK_GPS_I_SET(ADC_TO_MV * adc[GS_A3200DOCK_ADCS3_ADC_CURSENSE_GPS] / GS_A3200DOCK_ADCS3_SENSE_RESISTOR_GPS / 100);
    return GS_OK;
}

static gs_error_t adcs3_init(void)
{
    static gs_a3200dock_adcs3_dock_memory_t adcs3;

    adcs3_switch_init();

    gs_param_table_instance_t * tinst;
    const gs_param_table_create_t create = {.name = GS_A3200_DOCK_TABLE_NAME,
                                            .id = GS_A3200_DOCK_TABLE_ID,
                                            .rows = gs_a3200dock_adcs3_dock_table,
                                            .row_count = GS_A3200DOCK_ADCS3_DOCK_TABLE_ROWS,
                                            .memory = &adcs3,
                                            .memory_size = sizeof(adcs3),
                                            .callback = adcs3_table_change,
                                            .stores = GS_A3200_PARAM_STORE};
    gs_param_table_create(&create, &tinst);
    gs_param_table_load_fallback(tinst, 0);

    return GS_OK;
}

static gs_error_t adcs3_get_bool(gs_a3200dock_parameter_t param, bool *data)
{
    switch(param) {
    case GS_A3200DOCK_PARAM_GPS_EN:
            return GS_A3200DOCK_ADCS3_DOCK_GPS_EN_GET(data);
    case GS_A3200DOCK_PARAM_WDE_EN:
            return GS_A3200DOCK_ADCS3_DOCK_WDE_EN_GET(data);
        default:
            break;
    }

    return GS_ERROR_NOT_SUPPORTED;
}

static gs_error_t adcs3_set_bool(gs_a3200dock_parameter_t param, bool data)
{
    switch(param) {
    case GS_A3200DOCK_PARAM_GPS_EN:
            return GS_A3200DOCK_ADCS3_DOCK_GPS_EN_SET(data);
    case GS_A3200DOCK_PARAM_WDE_EN:
            return GS_A3200DOCK_ADCS3_DOCK_WDE_EN_SET(data);
        default:
            break;
    }

    return GS_ERROR_NOT_SUPPORTED;
}

static gs_error_t adcs3_get_uint8(gs_a3200dock_parameter_t param, uint8_t *data)
{
    return GS_ERROR_NOT_SUPPORTED;
}

static gs_error_t adcs3_set_uint8(gs_a3200dock_parameter_t param, uint8_t data)
{
    return GS_ERROR_NOT_SUPPORTED;
}

static gs_error_t adcs3_get_uint16(gs_a3200dock_parameter_t param, uint16_t *data)
{
    switch(param) {
	case GS_A3200DOCK_PARAM_GPS_I:
            return GS_A3200DOCK_ADCS3_DOCK_GPS_I_GET(data);
	case GS_A3200DOCK_PARAM_WDE_I:
            return GS_A3200DOCK_ADCS3_DOCK_WDE_I_GET(data);
        default:
            break;
    }

    return GS_ERROR_NOT_SUPPORTED;
}

static gs_error_t adcs3_set_uint16(gs_a3200dock_parameter_t param, uint16_t data)
{
    switch(param) {
	case GS_A3200DOCK_PARAM_GPS_I:
            return GS_A3200DOCK_ADCS3_DOCK_GPS_I_SET(data);
	case GS_A3200DOCK_PARAM_WDE_I:
            return GS_A3200DOCK_ADCS3_DOCK_WDE_I_SET(data);
        default:
            break;
    }

    return GS_ERROR_NOT_SUPPORTED;
}

const dock_t adcs3_dock = {
    .name = "adcs3",
    .type = GS_A3200DOCK_ADCS3,
    .init = adcs3_init,
    .update_adc = adcs3_update_adc,
    .get_bool = adcs3_get_bool,
    .set_bool = adcs3_set_bool,
    .get_uint8 = adcs3_get_uint8,
    .set_uint8 = adcs3_set_uint8,
    .get_uint16 = adcs3_get_uint16,
    .set_uint16 = adcs3_set_uint16,
};
