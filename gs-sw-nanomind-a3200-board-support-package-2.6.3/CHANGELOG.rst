A3200 BSP
=========

**2.6.3 (2023-07-05)**

- Improvement: Update tools and documentation to support alternative to AVR Dragon programmer.

**2.6.2 (2021-06-09)**

- Improvement: Update tools library

**2.6.1 (2019-12-17)**

- Improvement: Updated and added checkout commands to cover more of the manual checkout.
- Improvement: Changed size summary to sysv format to get details on section sizes.
- Bug: Increaased stack size for router task, so it doesn't crash with CSP debug enabled (GOSH: 'debug csp trace').
- Improvement: Include HTML documentation.
- Improvement: Updated libraries.

**2.5.2 (2019-10-29)**

- Improvement: Updated documentation.

**2.5.1 (2019-10-18)**

- Limitation: if enabling FreeRTOS configUSE_NEWLIB_REENTRANT (default), use AVR32 toolchain: 3.4.2_gs1 or later - otherwise crash occurs, when a task terminates (libembed).
- Limitation: Disabled FreeRTOS runtime stats until implemented for FreeRTOS 10.2.0.
- Improvement: Updated example code and documentation.
- Improvement: New AVR32 toolchain 3.4.2_gs1, with newlib patches for bug in reent and missing locking in file operations.
- Feature: Added commands for production checkout test.
- Feature: Added CSP communication over CAN and I2C.

**2.4.1 (2019-06-12)**

- Improvement: Updated example code and documentation.
- Improvement: Integrated toolchain setup in A3200 BSP manual.
- Breaking: Replaced Eclipse with Visual Code editor.

**2.3.1 (2019-05-20)**

- Improvement: Updated libraries.

**2.2.1 (2019-04-11)**

- Improvement: Added SPI FLASH example.
- Improvement: Updated libraries.

**2.1.1 (2019-02-27)**

- Breaking: Renamed LM70 to LM71.
- Improvement: Updated libraries.

**2.0.4 (2019-02-01)**

- Improvement: Updated documentation and libraries.

**2.0.3 (2018-11-23)**

- Improvement: Updated libraries.

**2.0.2 (2018-09-21)**

- Bug: Missing avrisp.rules file was added to dist. tarball.

**2.0.1 (2018-08-14)**

- Breaking: Updated libraries and re-structured source modules.

**1.3 (2016-11-23)**

- Feature: Added missing avrisp.rules to distribution

**1.2 (2016-03-02)**

- Feature: Added program.py for flash programming
- Feature: Manual updated

**1.1 (2015-09-10)**

- Feature: Manual updated

**1.0 (2015-07-31)**

- Feature: Imported source code for first release
- Feature: Documentation included in doc folder (pdf format)
