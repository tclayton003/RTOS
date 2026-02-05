#ifndef GS_EMBED_ASF_SAMC_DRIVERS_CAN_CAN_H
#define GS_EMBED_ASF_SAMC_DRIVERS_CAN_CAN_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   SAMC21 CAN interface.

   @note Only 1 filter/mask can be set, using gs_can_set_standard_filter_mask() or gs_can_set_extended_filter_mask()
*/

#include <gs/util/drivers/can/can.h>

/**
 * CAN control.
 */
typedef enum gs_can_control {
    /**
     * CAN driver is purely controlled by the \a enable attribute in the configuration.
     */
    GS_CAN_CONTROLLED_BY_CONFIG = 0,
    /**
     * CAN driver controls an external transceiver through an output pin.
     */
    GS_CAN_CONTROL_EXTERNAL_TRANSCEIVER,
    /**
     * CAN driver is enabled/disabled through an input pin (only detected once,when gs_can_init() is called).
     */
    GS_CAN_CONTROLLED_BY_EXTERNAL_ENABLE,
} gs_can_control_t;

/**
 * Configuration.
 */
typedef struct gs_can_config {
    /**
     * CAN enable.
     * Some boards have a PIN to control an external transceiver. If the PIN is configured as an output, and CAN isn't enabled - the transceiver will be disabled.
     */
    bool enable;
    /**
     * Control mode.
     */
    gs_can_control_t control;
    /**
     * GPIO pin depending on \a control
     */
    uint32_t gpio;
    /**
     * Active pin level on GPIO for enable/disable CAN driver or transceiver.
     */
    bool active_gpio_level;
    /**
     * Bit per second.
     */
    uint32_t bps;
} gs_can_config_t;

/**
 * Get default configuration parameters.
 * @param config
 */
void gs_can_get_default_config(struct gs_can_config * config);

/**
   Initialize CAN device/interface.
*/
gs_error_t gs_can_init(uint8_t hdl, const struct gs_can_config * can_config, bool * enabled);

/**
 * Print current status and accumulated interrupt events.
 */
void gs_can_print_status(uint8_t hdl);

/**
 * Enable/disable external transceiver if configured.
 */
gs_error_t gs_can_set_transceiver(uint8_t hdl, bool enable);

bool gs_can_debug();
void gs_can_set_debug(bool value);

/**
   Clear stat counters.
*/
void gs_can_clear_stats(uint8_t hdl);

#endif
