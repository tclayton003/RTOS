/*
 * idle.c
 *
 *  Created on: 22-08-2009
 *      Author: Administrator
 */

#include <avr32/io.h>
#include <stdio.h>
#include <sleep.h>
#include "compiler.h"
#include <conf_asf.h>

/*! \brief Sets the MCU in the specified sleep mode.
 *
 * \param mode Sleep mode:
 *   \arg \c AVR32_PM_SMODE_IDLE: Idle;
 *   \arg \c AVR32_PM_SMODE_FROZEN: Frozen;
 *   \arg \c AVR32_PM_SMODE_STANDBY: Standby;
 *   \arg \c AVR32_PM_SMODE_STOP: Stop;
 *   \arg \c AVR32_PM_SMODE_DEEP_STOP: DeepStop;
 *   \arg \c AVR32_PM_SMODE_STATIC: Static.
 */

void vApplicationIdleHook(void) {

	//*** Sleep mode
	// If there is a chance that any PB write operations are incomplete, the CPU
	// should perform a read operation from any register on the PB bus before
	// executing the sleep instruction.
	AVR32_INTC.ipr[0];  // Dummy read

	/* We changed back to idle mode, since PDC does not yet work properly with the frozen mode */
	SLEEP(MCU_SLEEP_MODE);

}
