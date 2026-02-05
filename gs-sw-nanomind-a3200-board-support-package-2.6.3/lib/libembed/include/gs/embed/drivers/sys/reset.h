#ifndef GS_EMBED_GS_SYS_H
#define GS_EMBED_GS_SYS_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   MCU reset control and causes.

   There exists two GS causes, for describing a reset.
   The boot cause is typically retrieved from the MCU, and describes the hardware near cause of reset/boot.
   The reset cause describes the cause of a software triggered reset.
*/

#include <gs/util/types.h>

/**
   Generic GomSpace boot causes
 */
typedef enum {
    /**
       Unknown cause
     */
    GS_SYS_BOOT_UNKNOWN = 0,
    /**
       Brown out detection boot
     */
    GS_SYS_BOOT_BROWN_OUT = 1,
    /**
       Power on boot
     */
    GS_SYS_BOOT_POWER_ON = 2,
    /**
       Watchdog boot
     */
    GS_SYS_BOOT_WATCHDOG = 3,
    /**
        Software triggered boot @see gs_sys_generic_reset_cause_t
     */
    GS_SYS_BOOT_SW = 4,
    /**
       Reset by the external boot pin
     */
    GS_SYS_BOOT_PIN_EXTERNAL = 5,
    /**
       Woken from sleep/standby mode
     */
    GS_SYS_BOOT_SLEEP_WAKE = 6,
    /**
       Reset due to CPU error, such as illegal addressing
     */
    GS_SYS_BOOT_ERROR = 7,
    /**
       Reset by JTAG
     */
    GS_SYS_BOOT_JTAG = 8,
} gs_sys_boot_cause_t;

/**
   Reset cause
 */
typedef uint32_t gs_sys_reset_cause_t;

/**
   Generic GomSpace reset causes

   @note These starts at 0, application specific causes should start at 100
 */
typedef enum {
    /**
       Unknown cause / hardware triggered boot
     */
    GS_SYS_RESET_UNKNOWN = 0,
    /**
       CSP watchdog reset
     */
    GS_SYS_RESET_CSP_WATCHDOG = 1,
    /**
       Ground watchdog boot
     */
    GS_SYS_RESET_GROUND_WATCHDOG = 2,
    /**
       Stack overflow, which results in reset
     */
    GS_SYS_RESET_STACK_OVERFLOW = 3,
    /**
       Exception, which results in reset
     */
    GS_SYS_RESET_EXCEPTION = 4,
    /**
       Reset by user from GOSH
     */
    GS_SYS_RESET_GOSH = 5,
    /**
       Reset by user from CSP
     */
    GS_SYS_RESET_CSP = 6,
    /**
       Reset by file system
     */
    GS_SYS_RESET_FILE_SYSTEM = 7,
    /**
       Reset due to insufficient memory
     */
    GS_SYS_RESET_NO_MEM = 8,
} gs_sys_generic_reset_cause_t;

/**
   Generic GomSpace structure to hold reset cause during SW reset.
   This structure is used on platforms that does not have scratchpad
   registers to store the information. Instead it is kept in a special
   internal RAM section which is not overwritten during SW reset.
 */
typedef struct {
    //! reset cause
    uint32_t reset_cause;
    //! CRC protection of reset cause
    uint32_t reset_cause_crc;
    //! crash addr in case of crash
    uint32_t crash_addr;
    //! CRC protection of crash addr
    uint32_t crash_addr_crc;
} gs_reset_cause_store_t;

/**
   Get boot cause

   @param[out] platform_boot_cause platform specific boot cause (can be NULL if unwanted)
   @return generic GomSpace boot cause
*/
gs_sys_boot_cause_t gs_sys_get_boot_cause(uint32_t * platform_boot_cause);

/**
   Convert boot cause to string

   @param[in] boot_cause Generic GomSpace boot cause
   @return string containing the boot cause
*/
const char * gs_sys_boot_cause_to_string(gs_sys_boot_cause_t boot_cause);

/**
   Get reset cause

   @return generic GomSpace reset cause
*/
gs_sys_reset_cause_t gs_sys_get_reset_cause(void);

/**
   Convert reset cause to string

   @note This function calls 'gs_sys_app_reset_cause_to_string'

   @param[in] reset_cause reset cause to be converted
   @return string containing the reset cause
*/
const char * gs_sys_reset_cause_to_string(gs_sys_reset_cause_t reset_cause);

/**
   Convert application specific reset cause to string

   @note This function is weakly defined, and an implementation can be
         written in the application source code if extra reset causes should be added

   @note This function is called from 'gs_sys_reset_cause_to_string' and should
         never be called by itself

   @param[in] reset_cause application specific reset cause
   @return string containing the application specific reset cause
 */
const char * gs_sys_app_reset_cause_to_string(gs_sys_reset_cause_t reset_cause);

/**
   Get the address of the former program crash

   @return address of crash (address = UINT32_MAX if no crash forced the last reset)
 */
uint32_t gs_sys_get_crash_address(void);

/**
   Log the boot and reset causes and potentially the 'address of crash'

   @param[out] boot_cause generic GomSpace boot cause (can be NULL if unwanted)
   @param[out] reset_cause generic GomSpace reset cause (can be NULL if unwanted)
   @param[out] crash_addr address of a potential former program crash (can be NULL if unwanted)
 */
void gs_sys_log_causes(gs_sys_boot_cause_t * boot_cause, gs_sys_reset_cause_t * reset_cause, uint32_t * crash_addr);

/**
   Clear the reset cause and crash address ensuring that the these are not
   erroneously read on subsequent boots.

   @note The gs_sys_get_crash_addr() & gs_sys_get_reset_cause() will still
   return valid cause and addr. This API only clears the store holding these
   values during a SW reset.

   When e.g. a FLASH image boots a RAM image, this should not have been called,
   since this makes it impossible for the RAM image to know the reset reason.
   The RAM image needs to call this function to clear reset cause during boot.
   The same applies for the FLASH image (if not RAM booting) to ensure that the
   reset reason will not linger during reboots.
 */
void gs_sys_clear_reset_cause(void);

/**
   Reset system

   @param[in] reset_cause sets the reset cause
*/
void gs_sys_reset(gs_sys_reset_cause_t reset_cause);

#endif
