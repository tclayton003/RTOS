#ifndef GS_A3200_LM71_H
#define GS_A3200_LM71_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Initialize SPI slaves for LM71 temperature sensors.
*/

#include <gs/thirdparty/lm71/lm71.h>
#include <gs/a3200/spi_slave.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Initialize SPI slaves for LM71 communication.

   @see gs/thirdparty/lm71/lm71.h for reading temperature.
*/
void gs_a3200_lm71_init(void);

/**
   Register commands.
*/
gs_error_t gs_a3200_lm71_register_commands(void);

#ifdef __cplusplus
}
#endif
#endif
