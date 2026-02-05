/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include "../local.h"
#include <gs/a3200dock/param/host/adcs8_dock.h>
#include <gs/a3200dock/drivers/sc16is750.h>
#include <gs/param/host_table.h>
#include <gs/a3200/adc_refs.h>
#include <gs/util/check.h>
#include <gpio.h>

GS_CHECK_STATIC_ASSERT(GS_A3200_DOCK_TABLE_ID == GS_A3200DOCK_ADCS8_DOCK_TABLE_ID, bad_table_id);

/* Define dock current measurement */
#define GS_A3200DOCK_ADCS8_SENSE_RESISTOR_GSSB_C     0.22
#define GS_A3200DOCK_ADCS8_ADC_CURSENSE_GSSB_C       4

#define GS_A3200DOCK_ADCS8_SENSE_RESISTOR_GSSB_D     0.22
#define GS_A3200DOCK_ADCS8_ADC_CURSENSE_GSSB_D       7

typedef enum {
    ADCS8_SWITCH_GSSBC = 1,
    ADCS8_SWITCH_GSSBD = 2,
    ADCS8_SWITCH_RW0 = 3,
    ADCS8_SWITCH_RW1 = 4,
    ADCS8_SWITCH_RW2 = 5,
    ADCS8_SWITCH_RW3 = 6
} adcs8_switch_t ;

static void adcs8_table_change(uint16_t addr, gs_param_table_instance_t * tinst)
{
    switch(addr) {
        case GS_A3200DOCK_ADCS8_DOCK_RW0_EN:
            sc16is750_gpio_set(ADCS8_SWITCH_RW0, GS_A3200DOCK_ADCS8_DOCK_RW0_EN_GET_NC());
            break;
        case GS_A3200DOCK_ADCS8_DOCK_RW1_EN:
            sc16is750_gpio_set(ADCS8_SWITCH_RW1, GS_A3200DOCK_ADCS8_DOCK_RW1_EN_GET_NC());
            break;
        case GS_A3200DOCK_ADCS8_DOCK_RW2_EN:
            sc16is750_gpio_set(ADCS8_SWITCH_RW2, GS_A3200DOCK_ADCS8_DOCK_RW2_EN_GET_NC());
            break;
        case GS_A3200DOCK_ADCS8_DOCK_RW3_EN:
            sc16is750_gpio_set(ADCS8_SWITCH_RW3, GS_A3200DOCK_ADCS8_DOCK_RW3_EN_GET_NC());
            break;
        case GS_A3200DOCK_ADCS8_DOCK_GSSB_C_EN:
            sc16is750_gpio_set(ADCS8_SWITCH_GSSBC, GS_A3200DOCK_ADCS8_DOCK_GSSB_C_EN_GET_NC());
            break;
        case GS_A3200DOCK_ADCS8_DOCK_GSSB_D_EN:
            sc16is750_gpio_set(ADCS8_SWITCH_GSSBD, GS_A3200DOCK_ADCS8_DOCK_GSSB_D_EN_GET_NC());
            break;
    }
}

static gs_error_t adcs8_update_adc(int16_t adc[])
{
    GS_A3200DOCK_ADCS8_DOCK_GSSB_C_I_SET(ADC_TO_MV * adc[GS_A3200DOCK_ADCS8_ADC_CURSENSE_GSSB_C] / GS_A3200DOCK_ADCS8_SENSE_RESISTOR_GSSB_C / 50);
    GS_A3200DOCK_ADCS8_DOCK_GSSB_D_I_SET(ADC_TO_MV * adc[GS_A3200DOCK_ADCS8_ADC_CURSENSE_GSSB_D] / GS_A3200DOCK_ADCS8_SENSE_RESISTOR_GSSB_D / 50);
    return GS_OK;
}

static gs_error_t adcs8_init(void)
{
    static gs_a3200dock_adcs8_dock_memory_t adcs8;

    gs_param_table_instance_t * tinst;
    const gs_param_table_create_t create = {.name = GS_A3200_DOCK_TABLE_NAME,
                                            .id = GS_A3200_DOCK_TABLE_ID,
                                            .rows = gs_a3200dock_adcs8_dock_table,
                                            .row_count = GS_A3200DOCK_ADCS8_DOCK_TABLE_ROWS,
                                            .memory = &adcs8,
                                            .memory_size = sizeof(adcs8),
                                            .callback = adcs8_table_change,
                                            .stores = GS_A3200_PARAM_STORE};
    gs_param_table_create(&create, &tinst);
    gs_param_table_load_fallback(tinst, GS_PARAM_F_NO_CALLBACK);

    sc16is750_init ();
    gs_param_table_call_callback_for_all_addresses(tinst);



    return GS_OK;
}

static gs_error_t adcs8_get_bool(gs_a3200dock_parameter_t param, bool *data)
{
    switch(param) {

    case GS_A3200DOCK_PARAM_RW0_EN:
            return GS_A3200DOCK_ADCS8_DOCK_RW0_EN_GET(data);

    case GS_A3200DOCK_PARAM_RW1_EN:
            return GS_A3200DOCK_ADCS8_DOCK_RW1_EN_GET(data);

    case GS_A3200DOCK_PARAM_RW2_EN:
            return GS_A3200DOCK_ADCS8_DOCK_RW2_EN_GET(data);

    case GS_A3200DOCK_PARAM_RW3_EN:
            return GS_A3200DOCK_ADCS8_DOCK_RW3_EN_GET(data);

    case GS_A3200DOCK_PARAM_GSSB_C_EN:
            return GS_A3200DOCK_ADCS8_DOCK_GSSB_C_EN_GET(data);

    case GS_A3200DOCK_PARAM_GSSB_D_EN:
            return GS_A3200DOCK_ADCS8_DOCK_GSSB_D_EN_GET(data);
    default:
            break;
    }

    return GS_ERROR_NOT_SUPPORTED;
}

static gs_error_t adcs8_set_bool(gs_a3200dock_parameter_t param, bool data)
{
    switch(param) {

    case GS_A3200DOCK_PARAM_RW0_EN:
            return GS_A3200DOCK_ADCS8_DOCK_RW0_EN_SET(data);

    case GS_A3200DOCK_PARAM_RW1_EN:
            return GS_A3200DOCK_ADCS8_DOCK_RW1_EN_SET(data);

    case GS_A3200DOCK_PARAM_RW2_EN:
            return GS_A3200DOCK_ADCS8_DOCK_RW2_EN_SET(data);

    case GS_A3200DOCK_PARAM_RW3_EN:
            return GS_A3200DOCK_ADCS8_DOCK_RW3_EN_SET(data);

    case GS_A3200DOCK_PARAM_GSSB_C_EN:
            return GS_A3200DOCK_ADCS8_DOCK_GSSB_C_EN_SET(data);

    case GS_A3200DOCK_PARAM_GSSB_D_EN:
            return GS_A3200DOCK_ADCS8_DOCK_GSSB_D_EN_SET(data);

    default:
            break;
    }

            return GS_ERROR_NOT_FOUND;
}

static gs_error_t adcs8_get_uint8(gs_a3200dock_parameter_t param, uint8_t *data)
{
    return GS_ERROR_NOT_SUPPORTED;
}

static gs_error_t adcs8_set_uint8(gs_a3200dock_parameter_t param, uint8_t data)
{
    return GS_ERROR_NOT_SUPPORTED;
}

static gs_error_t adcs8_get_uint16(gs_a3200dock_parameter_t param, uint16_t *data)
{
    switch(param) {

    case GS_A3200DOCK_PARAM_GSSB_C_I:
            return GS_A3200DOCK_ADCS8_DOCK_GSSB_C_I_GET(data);

    case GS_A3200DOCK_PARAM_GSSB_D_I:
            return GS_A3200DOCK_ADCS8_DOCK_GSSB_D_I_GET(data);

    default:
            break;
    }

    return GS_ERROR_NOT_SUPPORTED;
}

static gs_error_t adcs8_set_uint16(gs_a3200dock_parameter_t param, uint16_t data)
{
    switch(param) {

    case GS_A3200DOCK_PARAM_GSSB_C_I:
            return GS_A3200DOCK_ADCS8_DOCK_GSSB_C_I_SET(data);

    case GS_A3200DOCK_PARAM_GSSB_D_I:
            return GS_A3200DOCK_ADCS8_DOCK_GSSB_D_I_SET(data);

    default:
            break;
    }

    return GS_ERROR_NOT_SUPPORTED;
}

const dock_t adcs8_dock = {
    .name = "adcs8",
    .type = GS_A3200DOCK_ADCS8,
    .init = adcs8_init,
    .get_bool = adcs8_get_bool,
    .set_bool = adcs8_set_bool,
    .update_adc = adcs8_update_adc,
    .get_uint8 = adcs8_get_uint8,
    .set_uint8 = adcs8_set_uint8,
    .get_uint16 = adcs8_get_uint16,
    .set_uint16 = adcs8_set_uint16,
};
