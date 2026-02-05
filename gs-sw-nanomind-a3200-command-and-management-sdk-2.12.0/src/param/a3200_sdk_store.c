/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include "a3200_sdk_store.h"
#include <gs/a3200_sdk/param/scheduler.h>

const gs_param_store_vmem_map_t gs_a3200_sdk_param_store[] = {
    {GS_A3200_SDK_SCHEDULER_TABLE_NAME, 0x0000, 0x100},
    {.name = NULL}
};
