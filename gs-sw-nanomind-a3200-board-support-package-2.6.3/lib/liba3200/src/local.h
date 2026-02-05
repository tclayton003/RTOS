#ifndef LIBA3200_SRC_LOCAL_H
#define LIBA3200_SRC_LOCAL_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/a3200/a3200.h>
#include <conf_a3200.h> // holds defines for enabled/disabled functionality
#include <board.h>
#include <gs/asf/avr32/asf.h>
#include <gs/embed/asf/avr32/drivers/gpio/gpio.h>
#include <gs/embed/freertos.h>
#include <gs/util/log.h>
#include <gs/util/thread.h>
#if GS_A3200_SDK
#include <gs/a3200/param/host/board.h>
#include <gs/a3200/param/host/configuration.h>
#include <gs/a3200/param/host/telemetry.h>
#include <gs/param/host_table.h>
#endif

// External memory
#define EXTERNAL_RAM_ADDR     0xD0000000
#define EXTERNAL_RAM_SIZE     0x02000000 // 32MB

// Location of software image in external memory - determined by linkage
#define SWLOAD_LINK_ADDRESS   EXTERNAL_RAM_ADDR
#define SWLOAD_LINK_SIZE      0x00100000 // 1MB

// user application hooks
extern const gs_a3200_hooks_t * gs_a3200_hooks;
#define GS_A3200_CALL_HOOK(hook)   if (gs_a3200_hooks && gs_a3200_hooks->hook) {(gs_a3200_hooks->hook)();}

// Basic initialization (BSP)
void gs_a3200_init_spi0(bool decode);
void gs_a3200_init_spi1(void);
gs_error_t gs_a3200_init_fram(void);

// SDK initialization
void * gs_a3200_sdk_init_task(void * param);
void gs_a3200_sdk_watchdog_init(void);
void gs_a3200_sdk_param_init_board(void);
void gs_a3200_sdk_param_init(void);
void * gs_a3200_sdk_swload_task(void * param);
gs_error_t gs_a3200_sdk_param_get_dock(char * buf, size_t buf_size);

#endif
