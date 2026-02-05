#ifndef GS_EMBED_GS_SYS_H
#define GS_EMBED_GS_SYS_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Reset- and boot-causes.

   The boot cause is typically retrieved from the MCU, and describes the hardware near cause of reset/boot.
   The reset cause describes the cause of a software triggered reset. If the boot is triggered by software, the boot cause will be #GS_SYS_BOOT_SW.

   The boot- and reset-causes can be logged using gs_sys_log_causes().
*/

#include <gs/util/types.h>

/**
   Generic GomSpace boot causes.
*/
typedef enum {
    /**
       Unknown cause.
    */
    GS_SYS_BOOT_UNKNOWN = 0,
    /**
       Brown out detection.

       - AVR32
         - BOD33: Brown-out 3.3V reset (AVR32_PM_RCAUSE_BOD33_MASK=0x00002000). The CPU was reset due to the supply voltage being lower than the 3.3V Supply Monitor (BOD33) threshold level.
         - BOD: Brown-out Reset (AVR32_PM_RCAUSE_BOD_MASK=0x00000002). The CPU was reset due to the core supply voltage being lower than the brown-out threshold level.
    */
    GS_SYS_BOOT_BROWN_OUT = 1,
    /**
       Power on.

       - AVR32
         - POR: Power-on Reset (AVR32_PM_RCAUSE_POR_MASK=0x00000001). The CPU was reset due to the core supply voltage being lower than the power-on threshold level, or due to the input voltage being lower than the minimum required input voltage for the voltage regulator.
    */
    GS_SYS_BOOT_POWER_ON = 2,
    /**
       Watchdog.

       - AVR32
         - WDT: Watchdog Reset (AVR32_PM_RCAUSE_WDT_MASK=0x00000008). The CPU was reset because of a watchdog time-out.
    */
    GS_SYS_BOOT_WATCHDOG = 3,
    /**
       Software triggered boot, see #gs_sys_generic_reset_cause_t for detailed reason.

       - AVR32
         - OCDRST: OCD Reset (AVR32_PM_RCAUSE_OCDRST_MASK=0x00000100). The CPU was reset because the RES strobe in the OCD Development Control register has been written to one.
    */
    GS_SYS_BOOT_SW = 4,
    /**
       Reset by the external reset pin.

       - AVR32
         - EXT: External Reset Pin (CHIP_RESET_CAUSE_EXTRST=0x00000004). The CPU was reset due to the RESET pin being asserted.
    */
    GS_SYS_BOOT_PIN_EXTERNAL = 5,
    /**
       Woken from sleep/standby mode.

       - AVR32: (AVR32_PM_RCAUSE_SLEEP_MASK=0x00000040).
    */
    GS_SYS_BOOT_SLEEP_WAKE = 6,
    /**
       Reset due to CPU error, eg illegal addressing.

       - AVR32
         - CPUERR: CPU Error (AVR32_PM_RCAUSE_CPUERR_MASK=0x00000080). The CPU was reset because had detected an illegal access.
    */
    GS_SYS_BOOT_ERROR = 7,
    /**
       Reset by JTAG/AWIRE (programmer).

       - AVR32
         - • JTAG: JTAG Reset (CHIP_RESET_CAUSE_JTAG=0x00001A10). The chip was reset by a JTAG system reset.
    */
    GS_SYS_BOOT_JTAG = 8,
} gs_sys_boot_cause_t;

/**
   Reset cause
   Generic values are defined by #gs_sys_generic_reset_cause_t.
*/
typedef uint32_t gs_sys_reset_cause_t;

/**
   Generic GomSpace reset causes.

   @note Starts at 0, application specific causes should start at 100
*/
typedef enum {
    /**
       Unknown cause.
    */
    GS_SYS_RESET_UNKNOWN = 0,
    /**
       CSP watchdog reset, i.e. no communication on one or more CSP interfaces.
    */
    GS_SYS_RESET_CSP_WATCHDOG = 1,
    /**
       Ground watchdog was not touched in time.
    */
    GS_SYS_RESET_GROUND_WATCHDOG = 2,
    /**
       Stack overflow. A task exceeded its stack space.
    */
    GS_SYS_RESET_STACK_OVERFLOW = 3,
    /**
       Software exception, e.g. access unaligned memory address, performed illegal operation.
    */
    GS_SYS_RESET_EXCEPTION = 4,
    /**
       Reset initiated by GOSH command.
    */
    GS_SYS_RESET_GOSH = 5,
    /**
       Reset initiated by a request (message) on the CSP interface.
    */
    GS_SYS_RESET_CSP = 6,
    /**
       Reset by due to filesystem been rebuild/created.
    */
    GS_SYS_RESET_FILE_SYSTEM = 7,
    /**
       Reset due to insufficient memory.
       Should only be used as a last resort, as it might lead to inifinte reboot.
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

   Example of log output (from A3200 trying to read address 0x7ffffff):

       0000000000.000000 N default: boot cause: software (4), platform boot cause: 256, reset cause: exception (4)
       0000000000.005852 W default: program crashed at addr: 0x8004a594 

   The *boot cause* is #gs_sys_boot_cause_t, and *platform boot cause* is the native platform cause. The ``reset cause`` is a 
   value from #gs_sys_reset_cause_t.

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
