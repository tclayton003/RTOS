#ifndef LIBPARAM_SRC_STORE_VMEM_VMEM_LOCAL_H
#define LIBPARAM_SRC_STORE_VMEM_VMEM_LOCAL_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/param/store/vmem/vmem.h>
#include <gs/param/internal/types.h>

/**
   Turn store name into VMEM entry name.
*/
void gs_param_store_vmem_format_vmem_entry(const char * store, char * buf, size_t bufsize);

/**
   Find VMEM entry based on store name.
*/
const gs_vmem_t * gs_param_store_vmem_get_vmem(const gs_param_store_t * store);

/**
   Find VMEM and slot entry.
*/
gs_error_t gs_param_store_vmem_get_vmem_slot(const gs_param_store_t * store, const char * slot,
                                             const gs_vmem_t ** return_vmem, const gs_param_store_vmem_map_t ** return_slot);

/**
   Clear slot - by writing zeros.
*/
gs_error_t gs_param_store_vmem_clear_slot(const gs_param_store_t * store, const char * slot, uint32_t flags);

/**
   Lock store.
*/
gs_error_t gs_param_store_vmem_lock(const gs_param_store_t * store, bool on, uint32_t flags);

/**
   Check table vs. store - and configure auto-persist.
*/
gs_error_t gs_param_store_vmem_check(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags);

/**
   Load auto-persist parameters from store.
*/
gs_error_t gs_param_store_vmem_load_auto_persist(const gs_param_store_t * store, gs_param_table_instance_t * tinst, const char * slot, uint32_t flags);

/**
   Show information about the store.
*/
gs_error_t gs_param_store_vmem_info(const gs_param_store_t * store, uint8_t detail_level, FILE * out);

#endif
