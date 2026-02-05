#ifndef GS_PARAM_STORE_VMEM_VMEM_H
#define GS_PARAM_STORE_VMEM_VMEM_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Store driver for VMEM (libutil) based components.
*/

#include <gs/param/store/store.h>
#include <gs/util/vmem.h>
#include <gs/util/string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Store to slot mappping.
*/
typedef struct gs_param_store_vmem_map {
    /**
       Name of slot.
    */
    const char *name;
    /**
       Start of slot.
    */
    uint16_t    start;
    /**
       Size of slot.
       Size should be set to max expected table size over time, making it possible to add parameters to the table without moving
       all tables around.
    */
    uint16_t    size;
} gs_param_store_vmem_map_t;

/**
   Store driver using VMEM.
   A VMEM entry is divided into a number of slots through gs_param_store_vmem_map_t.
   Checksum and control information for the stored table, is placed first in the slot, allowing the table to be
   extended without loosing the existing parameters.

   Slots can have different sizes, and should be set to max expected table size over time.
*/
extern const gs_param_store_driver_t gs_param_store_vmem_driver_checksum_first;

/**
   Legacy store driver using VMEM.
   This driver supports the existing slot layout, where checksum and control information is placed last in the slot.
   These slots are normally 0x400 bytes, but the driver do support different sizes.
*/
extern const gs_param_store_driver_t gs_param_store_vmem_driver_checksum_last;

#ifdef __cplusplus
}
#endif
#endif
