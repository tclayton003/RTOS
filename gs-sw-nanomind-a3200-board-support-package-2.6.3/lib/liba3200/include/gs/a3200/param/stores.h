#ifndef GS_A3200_PARAM_STORES_H
#define GS_A3200_PARAM_STORES_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Parameter stores for A3200 tables.
*/

#include <gs/a3200/param/host/board.h>
#include <gs/a3200/param/host/configuration.h>
#include <gs/a3200/param/host/telemetry.h>
#include <gs/a3200/param/names.h>
#include <gs/param/store/vmem/vmem.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   A3200 persistent store.
   Minimum size is 0x500 bytes.
*/
extern const gs_param_store_vmem_map_t gs_a3200_param_store[];

/**
   A3200 protected store.
   Minimum size is 0x200 bytes.
*/
extern const gs_param_store_vmem_map_t gs_a3200_param_store_protected[];

/**
   A3200 MCU flash store.
   Minimum size is 0x400 bytes.
*/
extern const gs_param_store_vmem_map_t gs_a3200_param_store_mcu_flash[];

#ifdef __cplusplus
}
#endif
#endif
