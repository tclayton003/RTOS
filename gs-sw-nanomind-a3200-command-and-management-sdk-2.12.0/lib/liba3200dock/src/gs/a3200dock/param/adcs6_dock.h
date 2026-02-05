#ifndef GS_A3200DOCK_PARAM_ADCS6_DOCK_H
#define GS_A3200DOCK_PARAM_ADCS6_DOCK_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions for \a adcs6_dock.
*/

#ifdef __cplusplus
extern "C" {
#endif

/** Table name */
#define GS_A3200DOCK_ADCS6_DOCK_TABLE_NAME               "adcs6_dock"
/** Table id */
#define GS_A3200DOCK_ADCS6_DOCK_TABLE_ID                 5
/** Legacy table id definition */
#define GS_A3200DOCK_ADCS6_DOCK_TABLE_MEM_ID             5
/** Table data size (bytes */
#define GS_A3200DOCK_ADCS6_DOCK_TABLE_SIZE               14 // bytes
/** Table rows (count) */
#define GS_A3200DOCK_ADCS6_DOCK_TABLE_ROWS               10

/**
   @defgroup param_addr Parameter addresses within the table.
   @{
*/
/** Address of gps_en: Power GPS */
#define GS_A3200DOCK_ADCS6_DOCK_GPS_EN                                   0                  // 0x0000 bool
/** Address of rw0_en: Power Wheel 0 */
#define GS_A3200DOCK_ADCS6_DOCK_RW0_EN                                   1                  // 0x0001 bool
/** Address of rw1_en: Power Wheel 1 */
#define GS_A3200DOCK_ADCS6_DOCK_RW1_EN                                   2                  // 0x0002 bool
/** Address of rw2_en: Power Wheel 2 */
#define GS_A3200DOCK_ADCS6_DOCK_RW2_EN                                   3                  // 0x0003 bool
/** Address of rw3_en: Power Wheel 3 */
#define GS_A3200DOCK_ADCS6_DOCK_RW3_EN                                   4                  // 0x0004 bool
/** Address of gssb_c_en: Power GSSB C  */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_C_EN                                5                  // 0x0005 bool
/** Address of gssb_d_en: Power GSSB D */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_D_EN                                6                  // 0x0006 bool
/** Address of gps_i: GPS current */
#define GS_A3200DOCK_ADCS6_DOCK_GPS_I                                    8                  // 0x0008 uint16
/** Address of gssb_c_i: GSSB C current */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_C_I                                10                  // 0x000a uint16
/** Address of gssb_d_i: GSSB D current */
#define GS_A3200DOCK_ADCS6_DOCK_GSSB_D_I                                12                  // 0x000c uint16
/**@}*/
#ifdef __cplusplus
}
#endif
#endif
