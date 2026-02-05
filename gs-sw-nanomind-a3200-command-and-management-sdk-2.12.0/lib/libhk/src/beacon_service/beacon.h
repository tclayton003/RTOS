#ifndef _HK_BEACON_H_
#define _HK_BEACON_H_
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

/**
   @file

   @brief Data types and functions used in housekeeping (Beacon).

   The data types defined here are internal data types to the housekeeping system.
*/

#include <stdio.h>
#include <csp/csp.h>
#include <gs/hk/internal/internal_config.h>

/**
 * Send out beacon
 * @param conn pointer to active conn or NULL for default ground station destination
 * @param bcn pointer to beacon configuration to send
 * @param time timestamp to search from (t0) or zero to send out current value
 * @param fp if not NULL beacon will be written to this files instead of tx'ed using CSP.
 * @return_gs_error_t
 */
gs_error_t hk_service_send(csp_conn_t* conn, gs_hk_beacon_cfg_t* bcn,
                           uint32_t time, FILE* fp);


/**
 * Create beacon CSP packet.
 *
 * @param [in] bcn beacon type configuration
 * @param [in] requested_time Requested timestamp of beacon. Use 0 to indicate current time.
 * @param [in] satid Satellite ID.
 * @return pointer to CSP packet or NULL if the packet could not be allocated.
 */
csp_packet_t* create_bcn_packet(gs_hk_beacon_cfg_t* bcn, uint32_t requested_time, uint16_t satid);

/**
 * Initialize beacon module.
 *
 * @return_gs_error_t
 */
void gs_hk_beacon_init(void);

#endif
