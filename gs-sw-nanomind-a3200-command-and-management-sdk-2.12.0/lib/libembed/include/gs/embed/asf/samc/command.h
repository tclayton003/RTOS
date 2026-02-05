#ifndef GS_EMBED_ASF_SAMC_COMMAND_H
#define GS_EMBED_ASF_SAMC_COMMAND_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   SAMC commands.
*/

#include <gs/embed/command.h>

/**
   Register SAMC commands.
   @return_gs_error_t
*/
gs_error_t gs_samc_register_commands(void);

/**
   Register SAMC CAN commands.
   @return_gs_error_t
*/
gs_error_t gs_samc_register_can_commands(void);

/**
   Register SAMC I2C commands.
   @return_gs_error_t
*/
gs_error_t gs_samc_register_i2c_commands(void);

/**
   Register SAMC SPI commands.
   @return_gs_error_t
*/
gs_error_t gs_samc_register_spi_commands(void);

/**
   Register SAMC internal temperature sense commands.
   @return_gs_error_t
*/
gs_error_t gs_samc_register_tsense_ext_commands(void);

/**
   Register SAMC external temperature sense commands.
   @return_gs_error_t
*/
gs_error_t gs_samc_register_tsense_int_commands(void);

#endif
