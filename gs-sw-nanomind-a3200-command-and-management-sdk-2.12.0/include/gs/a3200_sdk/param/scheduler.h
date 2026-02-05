#ifndef GS_A3200_SDK_PARAM_SCHEDULER_H
#define GS_A3200_SDK_PARAM_SCHEDULER_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions for \a scheduler.
*/

#ifdef __cplusplus
extern "C" {
#endif

/** Table name */
#define GS_A3200_SDK_SCHEDULER_TABLE_NAME                "scheduler"
/** Table id */
#define GS_A3200_SDK_SCHEDULER_TABLE_ID                  10
/** Legacy table id definition */
#define GS_A3200_SDK_SCHEDULER_TABLE_MEM_ID              10
/** Table data size (bytes */
#define GS_A3200_SDK_SCHEDULER_TABLE_SIZE                15 // bytes
/** Table rows (count) */
#define GS_A3200_SDK_SCHEDULER_TABLE_ROWS                8

/**
   @defgroup param_addr Parameter addresses within the table.
   @{
*/
/** Address of enabled: Enable/disable scheduling */
#define GS_A3200_SDK_SCHEDULER_ENABLED                                   0                  // 0x0000 bool
/** Address of delayed_start: Delay before scheduling is started after boot/re-load */
#define GS_A3200_SDK_SCHEDULER_DELAYED_START                             2                  // 0x0002 uint16
/** Address of bpx_col_intv: BPX HK collection interval */
#define GS_A3200_SDK_SCHEDULER_BPX_COL_INTV                              4                  // 0x0004 uint16
/** Address of bpx_csp_addr: BPX CSP address */
#define GS_A3200_SDK_SCHEDULER_BPX_CSP_ADDR                              6                  // 0x0006 uint8
/** Address of eps_col_intv: EPS HK collection interval */
#define GS_A3200_SDK_SCHEDULER_EPS_COL_INTV                              8                  // 0x0008 uint16
/** Address of eps_csp_addr: EPS CSP address */
#define GS_A3200_SDK_SCHEDULER_EPS_CSP_ADDR                             10                  // 0x000a uint8
/** Address of adsb_col_intv: ADSB collection interval */
#define GS_A3200_SDK_SCHEDULER_ADSB_COL_INTV                            12                  // 0x000c uint16
/** Address of adsb_csp_addr: ADSB CSP address */
#define GS_A3200_SDK_SCHEDULER_ADSB_CSP_ADDR                            14                  // 0x000e uint8
/**@}*/
#ifdef __cplusplus
}
#endif
#endif
