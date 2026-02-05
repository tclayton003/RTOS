#ifndef GS_A3200_A3200_H
#define GS_A3200_A3200_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Initialization and boot API for A3200.
*/

#include <gs/util/error.h>
#include <gs/embed/asf/drivers/i2c/i2c.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Default stack size (in bytes).
   @see gs_a3200_get_default_stack_size()
*/
#define GS_A3200_DEFAULT_STACK_SIZE (16 * 1024)

/**
   Return default stack size.

   This function is implemented as a \a weak function, allowing it to be replaced by custom code.

   @return Default stack size (bytes)
*/
size_t gs_a3200_get_default_stack_size(void);

/**
   Initial (low-level) a3200 initialization.
   @param[in] external_heap use \a true for switching heap from internal to external memory.
   @return_gs_error_t
*/
gs_error_t gs_a3200_init_set_heap(bool external_heap);

/**
   Initial (low-level) a3200 initialization.
   @return_gs_error_t
*/
static inline gs_error_t gs_a3200_init(void)
{
    return gs_a3200_init_set_heap(true);
}

/**
   Hooks for the init sequence.
*/
typedef struct {

    /**
       SDK: Hook called to setup VMEM and parameter store.

       Hook is called from the context of the \a init task, started by gs_a3200_run().
    */
    void (*init_vmem_param)(void);
    
    /**
       Initialization is complete (SDK: file system may not yet be mounted).

       Hook is called from the context of the \a init task, started by gs_a3200_run().
    */
    void (*init_complete)(void);

    /**
       SDK: File system is mounted - a3200 is fully booted.

       Hook is called from the context of the \a init task, started by gs_a3200_run().
    */
    void (*fs_mounted)(void);

} gs_a3200_hooks_t;

/**
   Spawn a task to run the initialization and launch application tasks - through hooks.
   Once the initialization is complete, the task terminates.
   @note this function will never return.
   @param[in] hooks user hooks.
*/
void gs_a3200_run(const gs_a3200_hooks_t * hooks) __attribute__((noreturn));

/**
   Initialize TWI/I2C device.

   @param[in] device device to initialize, 2 or 4 supported.
   @param[in] mode master or multi-master.
   @param[in] addr master address.
   @param[in] bps bits per second, 400000 or 150000 Kbps.
   @return_gs_error_t
*/
gs_error_t gs_a3200_init_twi(uint8_t device, gs_avr32_i2c_mode_t mode, uint8_t addr, uint32_t bps);

/**
   Initialize CAN device.

   @param[in] device device to initialize, only 0 supported.
   @param[in] enable \a true to enable device, \a false to disable the device.
   @param[in] bps bits per second, valid values are 250000, 500000 or 1000000 Mbps.
   @param[in] fallback_bps value used if the \a bps cannot be applied.
   @return_gs_error_t
*/
gs_error_t gs_a3200_init_can(uint8_t device, bool enable, uint32_t bps, uint32_t fallback_bps);
    
#ifdef __cplusplus
}
#endif
#endif
