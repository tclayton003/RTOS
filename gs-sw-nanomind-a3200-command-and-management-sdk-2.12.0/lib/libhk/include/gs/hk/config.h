#ifndef GS_HK_CONFIG
#define GS_HK_CONFIG
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/util/error.h>

/**
   @file

   @brief Housekeeping system Configuration.
*/

/**
 * Check if the Housekeeping system has been initialized.
 *
 */
bool gs_hk_config_is_initialized();

/**
 * Check if loading of configuration data is done.
 *
 */
bool gs_hk_config_is_loaded();

/**
 * Initialize Housekeeping system parameter table and prepare for loading of
 * beacon type configurations.
 *
 * Must be called before loading beacon type configuration and before
 * starting Housekeeping system tasks.
 *
 * IMPORTANT: It is recommended to wait/sleep for a grace-period after calling
 * this function and before calling gs_hk_create_config_init_thread(). This is
 * to allow an operator to disable the Housekeeping system completely before in
 * the event of a critical issue with uploaded beacon configurations.
 *
 */
void gs_hk_config_init();

/**
 * Start thread that loads beacon type configurations from files in \<data_path\>/beacons. Sampling
 * of tables needed for beacons is automatically configured as beacon types
 * are loaded. The function gs_hk_config_init() must be called prior to this function.
 *
 * @see gs_hk_config_init
 *
 * @return_gs_error_t
 */
gs_error_t gs_hk_create_config_init_thread(void);

/**
 * Reload configuration from files in \<data_path\>/beacons. The
 * function gs_hk_config_init() must be called prior to calling this function.
 *
 * @return_gs_error_t
 */
gs_error_t gs_hk_config_reload();


#endif
