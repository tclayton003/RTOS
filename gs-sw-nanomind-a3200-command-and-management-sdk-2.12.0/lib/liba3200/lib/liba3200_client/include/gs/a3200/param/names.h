#ifndef GS_A3200_PARAM_NAMES_H
#define GS_A3200_PARAM_NAMES_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Parameter table names on A3200.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
   A3200 persistent store.
*/
#define GS_A3200_PARAM_STORE             "a3200"

/**
   A3200 protected store.
*/
#define GS_A3200_PARAM_STORE_PROTECTED   GS_A3200_PARAM_STORE"-protected"

/**
   A3200 flash store (MCU).
*/
#define GS_A3200_PARAM_STORE_MCU_FLASH   GS_A3200_PARAM_STORE"-flash"

/**
   Table name of A3200 dock table.
   Content of table depends on dock-type, and defined in liba3200dock.
*/
#define GS_A3200_DOCK_TABLE_NAME         "dock"

#ifdef __cplusplus
}
#endif
#endif
