#ifndef GS_HK_PARAM_STORES_H
#define GS_HK_PARAM_STORES_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/hk/param/host/hkcfg.h>
#include <gs/param/store/vmem/vmem.h>

/**
   A3200 persistent store.
*/
#define GS_HK_PARAM_STORE             "hk"

/**
   HK persistent store.
   Minimum size is 0x400 bytes.
*/
extern const gs_param_store_vmem_map_t gs_hk_param_store[];

#endif
