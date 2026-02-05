#ifndef GS_A3200DOCK_PARAM_ADCS3_DOCK_H
#define GS_A3200DOCK_PARAM_ADCS3_DOCK_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions for \a adcs3_dock.
*/

#ifdef __cplusplus
extern "C" {
#endif

/** Table name */
#define GS_A3200DOCK_ADCS3_DOCK_TABLE_NAME               "adcs3_dock"
/** Table id */
#define GS_A3200DOCK_ADCS3_DOCK_TABLE_ID                 5
/** Legacy table id definition */
#define GS_A3200DOCK_ADCS3_DOCK_TABLE_MEM_ID             5
/** Table data size (bytes */
#define GS_A3200DOCK_ADCS3_DOCK_TABLE_SIZE               6 // bytes
/** Table rows (count) */
#define GS_A3200DOCK_ADCS3_DOCK_TABLE_ROWS               4

/**
   @defgroup param_addr Parameter addresses within the table.
   @{
*/
/** Address of gps_en: Power GPS */
#define GS_A3200DOCK_ADCS3_DOCK_GPS_EN                                   0                  // 0x0000 bool
/** Address of wde_en: Power Wheels */
#define GS_A3200DOCK_ADCS3_DOCK_WDE_EN                                   1                  // 0x0001 bool
/** Address of gps_i: GPS current */
#define GS_A3200DOCK_ADCS3_DOCK_GPS_I                                    2                  // 0x0002 uint16
/** Address of wde_i: Wheel current */
#define GS_A3200DOCK_ADCS3_DOCK_WDE_I                                    4                  // 0x0004 uint16
/**@}*/
#ifdef __cplusplus
}
#endif
#endif
