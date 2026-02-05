/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include "../local.h"
#include <gs/a3200dock/param/host/adcs6_dock.h>
#include <gs/a3200dock/drivers/sc16is750.h>
#include <gs/param/host_table.h>
#include <gs/a3200/adc_refs.h>
#include <gs/util/check.h>
#include <gpio.h>

GS_CHECK_STATIC_ASSERT(GS_A3200_DOCK_TABLE_ID == GS_A3200DOCK_ADCS6_DOCK_TABLE_ID, bad_table_id);

/* Define dock current measurement */
#define GS_A3200DOCK_ADCS6_SENSE_RESISTOR_GPS     0.1
#define GS_A3200DOCK_ADCS6_ADC_CURSENSE_GPS       2

#define GS_A3200DOCK_ADCS6_SENSE_RESISTOR_MAG     0.22
#define GS_A3200DOCK_ADCS6_ADC_CURSENSE_MAG       4

#define GS_A3200DOCK_ADCS6_SENSE_RESISTOR_FSS     0.22
#define GS_A3200DOCK_ADCS6_ADC_CURSENSE_FSS       7

typedef enum {
    ADCS6_SWITCH_GPS = 0,
    ADCS6_SWITCH_FSS = 1,
    ADCS6_SWITCH_MAG = 2,
    ADCS6_SWITCH_RW0 = 3,
    ADCS6_SWITCH_RW1 = 4,
    ADCS6_SWITCH_RW2 = 5,
    ADCS6_SWITCH_RW3 = 6
} adcs6_switch_t ;

static void adcs6_table_change(uint16_t addr, gs_param_table_instance_t * tinst)
{
    switch(addr) {
        case GS_A3200DOCK_ADCS6_DOCK_GPS_EN:
            sc16is750_gpio_set(ADCS6_SWITCH_GPS, GS_A3200DOCK_ADCS6_DOCK_GPS_EN_GET_NC());
            break;
        case GS_A3200DOCK_ADCS6_DOCK_RW0_EN:
            sc16is750_gpio_set(ADCS6_SWITCH_RW0, GS_A3200DOCK_ADCS6_DOCK_RW0_EN_GET_NC());
            break;
        case GS_A3200DOCK_ADCS6_DOCK_RW1_EN:
            sc16is750_gpio_set(ADCS6_SWITCH_RW1, GS_A3200DOCK_ADCS6_DOCK_RW1_EN_GET_NC());
            break;
        case GS_A3200DOCK_ADCS6_DOCK_RW2_EN:
            sc16is750_gpio_set(ADCS6_SWITCH_RW2, GS_A3200DOCK_ADCS6_DOCK_RW2_EN_GET_NC());
            break;
        case GS_A3200DOCK_ADCS6_DOCK_RW3_EN:
            sc16is750_gpio_set(ADCS6_SWITCH_RW3, GS_A3200DOCK_ADCS6_DOCK_RW3_EN_GET_NC());
            break;
        case GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN:
            sc16is750_gpio_set(ADCS6_SWITCH_FSS, GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN_GET_NC());
            break;
        case GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN:
            sc16is750_gpio_set(ADCS6_SWITCH_MAG, GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN_GET_NC());
            break;
    }
}

static gs_error_t adcs6_update_adc(int16_t adc[])
{
    GS_A3200DOCK_ADCS6_DOCK_GPS_I_SET(ADC_TO_MV * adc[GS_A3200DOCK_ADCS6_ADC_CURSENSE_GPS] / GS_A3200DOCK_ADCS6_SENSE_RESISTOR_GPS / 50);
    GS_A3200DOCK_ADCS6_DOCK_GSSB_C_I_SET(ADC_TO_MV * adc[GS_A3200DOCK_ADCS6_ADC_CURSENSE_MAG] / GS_A3200DOCK_ADCS6_SENSE_RESISTOR_MAG / 50);
    GS_A3200DOCK_ADCS6_DOCK_GSSB_D_I_SET(ADC_TO_MV * adc[GS_A3200DOCK_ADCS6_ADC_CURSENSE_FSS] / GS_A3200DOCK_ADCS6_SENSE_RESISTOR_FSS / 50);
    return GS_OK;
}


static gs_error_t adcs6_init(void)
{
    static gs_a3200dock_adcs6_dock_memory_t adcs6;

    gs_param_table_instance_t * tinst;
    const gs_param_table_create_t create = {.name = GS_A3200_DOCK_TABLE_NAME,
                                            .id = GS_A3200_DOCK_TABLE_ID,
                                            .rows = gs_a3200dock_adcs6_dock_table,
                                            .row_count = GS_A3200DOCK_ADCS6_DOCK_TABLE_ROWS,
                                            .memory = &adcs6,
                                            .memory_size = sizeof(adcs6),
                                            .callback = adcs6_table_change,
                                            .stores = GS_A3200_PARAM_STORE};
    gs_param_table_create(&create, &tinst);
    gs_param_table_load_fallback(tinst, GS_PARAM_F_NO_CALLBACK);

    sc16is750_init ();
    gs_param_table_call_callback_for_all_addresses(tinst);

    return GS_OK;
}

static gs_error_t adcs6_get_bool(gs_a3200dock_parameter_t param, bool *data)
{
    switch(param) {

    case GS_A3200DOCK_PARAM_GPS_EN:
            return GS_A3200DOCK_ADCS6_DOCK_GPS_EN_GET(data);

    case GS_A3200DOCK_PARAM_RW0_EN:
            return GS_A3200DOCK_ADCS6_DOCK_RW0_EN_GET(data);

    case GS_A3200DOCK_PARAM_RW1_EN:
            return GS_A3200DOCK_ADCS6_DOCK_RW1_EN_GET(data);

    case GS_A3200DOCK_PARAM_RW2_EN:
            return GS_A3200DOCK_ADCS6_DOCK_RW2_EN_GET(data);

    case GS_A3200DOCK_PARAM_RW3_EN:
            return GS_A3200DOCK_ADCS6_DOCK_RW3_EN_GET(data);

    case GS_A3200DOCK_PARAM_GSSB_C_EN:
            return GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN_GET(data);

    case GS_A3200DOCK_PARAM_GSSB_D_EN:
            return GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN_GET(data);
    default:
            break;
    }

    return GS_ERROR_NOT_SUPPORTED;
}

static gs_error_t adcs6_set_bool(gs_a3200dock_parameter_t param, bool data)
{
    switch(param) {

    case GS_A3200DOCK_PARAM_GPS_EN:
            return GS_A3200DOCK_ADCS6_DOCK_GPS_EN_SET(data);

    case GS_A3200DOCK_PARAM_RW0_EN:
            return GS_A3200DOCK_ADCS6_DOCK_RW0_EN_SET(data);

    case GS_A3200DOCK_PARAM_RW1_EN:
            return GS_A3200DOCK_ADCS6_DOCK_RW1_EN_SET(data);

    case GS_A3200DOCK_PARAM_RW2_EN:
            return GS_A3200DOCK_ADCS6_DOCK_RW2_EN_SET(data);

    case GS_A3200DOCK_PARAM_RW3_EN:
            return GS_A3200DOCK_ADCS6_DOCK_RW3_EN_SET(data);

    case GS_A3200DOCK_PARAM_GSSB_C_EN:
            return GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN_SET(data);

    case GS_A3200DOCK_PARAM_GSSB_D_EN:
            return GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN_SET(data);

    default:
            break;
    }

    return GS_ERROR_NOT_FOUND;
}

static gs_error_t adcs6_get_uint8(gs_a3200dock_parameter_t param, uint8_t *data)
{
    return GS_ERROR_NOT_SUPPORTED;
}

static gs_error_t adcs6_set_uint8(gs_a3200dock_parameter_t param, uint8_t data)
{
    return GS_ERROR_NOT_FOUND;
}

static gs_error_t adcs6_get_uint16(gs_a3200dock_parameter_t param, uint16_t *data)
{
    switch(param) {

	case GS_A3200DOCK_PARAM_GPS_I:
            return GS_A3200DOCK_ADCS6_DOCK_GPS_I_GET(data);

	case GS_A3200DOCK_PARAM_GSSB_C_I:
            return GS_A3200DOCK_ADCS6_DOCK_GSSB_C_I_GET(data);

	case GS_A3200DOCK_PARAM_GSSB_D_I:
            return GS_A3200DOCK_ADCS6_DOCK_GSSB_D_I_GET(data);

	default:
            break;
    }

    return GS_ERROR_NOT_SUPPORTED;
}

static gs_error_t adcs6_set_uint16(gs_a3200dock_parameter_t param, uint16_t data)
{
    switch(param) {

	case GS_A3200DOCK_PARAM_GPS_I:
            return GS_A3200DOCK_ADCS6_DOCK_GPS_I_SET(data);

	case GS_A3200DOCK_PARAM_GSSB_C_I:
            return GS_A3200DOCK_ADCS6_DOCK_GSSB_C_I_SET(data);

	case GS_A3200DOCK_PARAM_GSSB_D_I:
            return GS_A3200DOCK_ADCS6_DOCK_GSSB_D_I_SET(data);

	default:
            break;
    }

    return GS_ERROR_NOT_SUPPORTED;
}

const dock_t adcs6_dock = {
    .name = "adcs6",
    .type = GS_A3200DOCK_ADCS6,
    .init = adcs6_init,
    .update_adc = adcs6_update_adc,
    .get_bool = adcs6_get_bool,
    .set_bool = adcs6_set_bool,
    .get_uint8 = adcs6_get_uint8,
    .set_uint8 = adcs6_set_uint8,
    .get_uint16 = adcs6_get_uint16,
    .set_uint16 = adcs6_set_uint16,
};
