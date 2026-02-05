#ifndef GS_PARAM_STORE_STORE_H
#define GS_PARAM_STORE_STORE_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Store API.
*/

#include <gs/param/host_table.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Standard stores names.
   @{
*/
#define GS_PARAM_STORE_PERSISTENT    "persistent"
#define GS_PARAM_STORE_PROTECTED     "protected"
#define GS_PARAM_STORE_READ_ONLY     "read-only"
#define GS_PARAM_STORE_AUTO_PERSIST  "auto-persist"
/** @} */

/**
   Prefix for a auto-persist store.
*/
#define GS_PARAM_STORE_AP_PREFIX     "ap-"
    
/**
   Store flags.
   Flags must be in range: bit 24 - 31, to avoid clash with other parts of the parameter system.
   @{
*/
    /**
       Skip loading auto-persist parameters.
    */
#define GS_PARAM_STORE_F_SKIP_AUTO_PERSIST   0x1000
    /**
       Configure auto-persist store and callback.
       This is set during gs_param_table_create(), when \a check is performned on any configured store(s).
    */
#define GS_PARAM_STORE_F_SETUP_AUTO_PERSIST  0x2000
/** @} */

/**
   Store interface.
*/
typedef struct gs_param_store gs_param_store_t;

/**
   Store driver interface.
*/
typedef struct {
    /**
       Check is called for each store configured on a table during table create, see gs_param_table_create().
       The store driver should validate mapping (e.g. offset, size) against stores and boot-strap auto-persist (if flag #GS_PARAM_STORE_F_SETUP_AUTO_PERSIST is set).
    */
    gs_error_t (*check)(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags);
    /**
       Save table to a store slot.
       @return GS_ERROR_DATA if indication of the store being lock, due to reading wrong data back.
    */
    gs_error_t (*save)(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags);
    /**
       Load table from a store slot.
    */
    gs_error_t (*load)(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags);
    /**
       Load auto-persist paramters from a store slot.
    */
    gs_error_t (*load_auto_persist)(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags);
    /**
       Clear/invalidate a store slot, e.g. invalidates CRC, so table can't be loaded.
    */
    gs_error_t (*clear)(const gs_param_store_t * store, const char * slot, uint32_t flags);
    /**
       Lock store.
    */
    gs_error_t (*lock)(const gs_param_store_t * store, bool on, uint32_t flags);
    /**
       Show information about the store, e.g. including slots.
       @see gs_param_store_info().
    */
    gs_error_t (*info)(const gs_param_store_t * store, uint8_t detail_level, FILE * out);
} gs_param_store_driver_t;

/**
   Logical store mapping - mapping a name to a store driver.
*/
struct gs_param_store {
    /**
       Name of store.
    */
    const char * name;
    /**
       Driver interface.
    */
    const gs_param_store_driver_t * drv;
    /**
       Driver data.
    */
    const void * drv_data;
};

/**
   Register store mappings.
*/
gs_error_t gs_param_store_register(const gs_param_store_t * map);

/**
   Clear/invalidate store slot/entry - invalidates CRC, so table can't be loaded.

   @param[in] store store
   @param[in] slot slot to clear
   @param[in] flags flags (future).
   @return_gs_error_t
*/
gs_error_t gs_param_store_clear(const char * store, const char * slot, uint32_t flags);

/**
   Lock store.

   @note Locking a store may result in locking multiple stores as it depends on the physical store.

   @param[in] store store
   @param[in] on    On=true, Off=false
   @param[in] flags flags (future).
   @return_gs_error_t
*/
gs_error_t gs_param_store_lock(const char * store, bool on, uint32_t flags);

/**
   Show store information.

   @param[in] store store. If NULL, show overal store info - else show store specific info.
   @param[in] out   output stream.
   @param[in] detail_level level of information, 0 minimum information (single line), 1+ extra/detailed information (multiple lines, preferably key = value formatted). See output from \"param tableinfo \<table\>\".
   @return_gs_error_t
*/
gs_error_t gs_param_store_info(const char * store, uint8_t detail_level, FILE * out);

#ifdef __cplusplus
}
#endif
#endif
