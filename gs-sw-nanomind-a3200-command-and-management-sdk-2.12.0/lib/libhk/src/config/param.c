/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/hk/param/stores.h>

const gs_param_store_vmem_map_t gs_hk_param_store[] = {
    {"hkcfg",   0x0000,    0x100}, // -> GS_HK_HKCFG_TABLE_NAME
    {.name = NULL}
};
