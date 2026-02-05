#ifndef A3200_SDK_SRC_PARAM_A3200_SDK_STORE_H
#define A3200_SDK_SRC_PARAM_A3200_SDK_STORE_H
/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/param/store/vmem/vmem.h>

/**
   A3200-SDK persistent store name
*/
#define GS_A3200_SDK_PARAM_STORE             "a3200-sdk"

/**
   A3200 SDK protected store name
*/
#define GS_A3200_SDK_PARAM_STORE_PROTECTED   GS_A3200_SDK_PARAM_STORE"-pro"

/**
   A3200 SDK store layout.
*/
extern const gs_param_store_vmem_map_t gs_a3200_sdk_param_store[];

#endif
