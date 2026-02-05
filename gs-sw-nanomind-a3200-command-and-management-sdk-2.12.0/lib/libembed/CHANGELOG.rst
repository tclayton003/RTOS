Embed (libembed)
================

**1.9.1 (2020-03-20)**

- Breaking: ASF/AVR32: Removed deprecated/unused gs_spi_{read, write, setup_chip, select_chip, unselect_chip}.
- Bug: STM32: Fixed UART device 3 callback to make it usable.
- Improvement: AVR32 Locate can_mob_ram in named section, so it can be placed at HSB_SRAM.
- Improvement: AVR32: Use the timeout given by user for SPI / I2C
- Improvement: STM32: Optimized UART driver.
- Breaking: AVR32: Removed deprecated/unused gs_spi_set_cs_timeout().
  
**1.8.1 (2019-10-16)**

- Limitation: if enabling FreeRTOS configUSE_NEWLIB_REENTRANT (default), use AVR32 toolchain: 3.4.2_gs1 or later - otherwise crash occurs, when a task terminates.
- Improvement: Updated dependencies and documentation.
- Improvement: Close stdin/stdout/stderr to prevent memory leak in gs_thread_exit() (if configUSE_NEWLIB_REENTRANT)
- Breaking: AVR32: gs_mcu_flash API: Disable write to internal flash, if not running from MCU flash image.
- Improvement: AVR32: Changed gs_delay_us() to use cpu_delay_us()
- Improvement: Implement retargetable locks for newlib file operations. This allows for multi-threaded file operations. Requires toolchain 3.4.2_gs1 or later.
- Breaking: Consolidated newlib stubs for STM32, AVR32 and samc -> src/newlib/syscalls.

**1.7.4 (2019-07-03)**

- Feature: Added support for FreeRTOS 10.
- Bug: fixed minor issues in AVR32 CAN driver, e.g. return codes, ISR logging.
- Improvement: AV32 CAN driver statistics, command: ``driver can <device>``.

**1.7.3 (2019-06-13)**

- Bug: Disabling and clearing GPIO interrupts before booting RAM image

**1.7.2 (2019-05-23)**

- Bug: Fixed bug in UART RX queue handling, which potentially could result in lost characters (present in all versions). Not present if using callbacks.

**1.7.1 (2019-05-16)**

- Improvement: STM32: Rewritten I2C driver to make it work under higher load on the MCU.
- Improvement: Include source code in distribution.

**1.6.1 (2019-04-08)**

- Improvement: Updated dependencies and documentation.

**1.5.5 (2019-03-21)**

- Bug: STM32: Fixed race condition in CAN driver.
- Improvement: STM32: SPI driver handles slaves faster and does not init GPIO when using chip select callback.

**1.5.4 (2019-02-26)**

- Bug: Removed AVR32 GPIO definitions due to some clashes with gpio.h in libasf.
- Improvement: STM32 linker-scripts: Removed KEEP for log groups.

**1.5.3 (2019-01-29)**

- Improvement: Register all used log group.
- Feature: Replaced log group "reset" with "default" group.
- Feature: SAMC: set default minimum stack size to 2000 bytes.
- Bug: STM32: Made I2C slave driver capable of receiving pure master-writes.

**1.5.2 (2018-12-19)**

- Improvement: Updated documentation.

**1.5.1 (2018-11-21)**

- Bug: Made gs_time_uptime() thread safe (added call in gs_thread_create() for initialization of mutex).
- Breaking: Removed gs_time_rel_s() and gs_time_rel_s_isr().
- Improvement: STM32: Made support for 20 SPI slaves instead of 8.

**1.4.1 (2018-09-20)**

- Feature: Add gs_thread_create_with_stack to manually place stack buffer.
- Feature: avr32: Add support for using GPIO as SPI chipselect.

**1.3.1 (2018-08-13)**

- Feature: avr32: added support for gs_getcwd().
- Feature: avr32: added avr32_reset_all_interrupt_settings to use when booting from RAM.
- Improvement: avr32: Optimized gs_spi_master_transactions().

**1.2.1 (2018-05-30)**

- Breaking: Updated reset cause and system reset API.
- Feature: Added support for microsecond delays on STM32
- Improvement: Use standard log groups for I2C, SPI and CAN.
- Feature: split "free" (memory) comamnd into intern/extern for improved error handling.
- Breaking: SAMC21 - refactored APIs to comply with standard APIs.

**1.1.2 (2018-04-17)**

- Feature: Support for different SPI clock polarity/phase.
- Feature: Adde gs_freertos_convert_ms_to_ticks().
- Breaking: Changed I2C API to comply with standard.
- Feature: Added VMEM driver for MCU flash.
- Feature: SAMC21 changed UART driver from polled to interrupt.

**1.0.1 (2018-03-09)**
