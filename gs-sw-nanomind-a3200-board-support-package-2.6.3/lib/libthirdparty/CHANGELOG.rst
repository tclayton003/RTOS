Thirdparty (libthirdparty)
==========================

**2.9.1 (2019-10-16)**

- Bug: tps382x watchdog service interval outside spec
- Improvement: spn_fl512 API: Error handling in both API and commands. Commands now supports both dies.
- Breaking: spn_fl512s_read_status_register() return error code and status register by reference.

**2.8.4 (2019-07-19)**

- Improvement: Cleanup of HTPA driver and optimised wait time between sensor readouts

**2.8.3 (2019-05-29)**

- Improvement: Added gps log groupe - the log groupe is now resigterd from the gps task 

**2.8.2 (2019-05-16)**

- Improvement: Added the capability to initialise the rm3100 with different I2C addresses than the default.
- Improvement: Added source code to distribution.

**2.8.1 (2019-04-08)**

- Improvement: Added 'info' API to FRAM interfaces, for getting detailed information.
- Improvement: Updated dependencies.

**2.7.1 (2019-02-26)**

- Breaking: Renamed LM70 API to LM71.
- Bug: LM71 (former LM70) failed to read negative temperatures - bug introduced in 2.4.1 (when driver was added).
- Feature: Added support for calibration and current/power measurements for INA226

**2.6.1 (2019-01-30)**

- Feature: Added driver for ISIS antenna system
- Improvement: FRAM read/write: added check on address and length. Added read back check in commands for erase and write.
- Feature: Added VARF command for Novatel 615
- Improvement: Register all used log groups.

**2.5.2 (2018-12-19)**

- Improvement: Updated documentation.

**2.5.1 (2018-11-21)**

- Feature: Added driver for Hyperion ST200 star tracker
- Breaking: Rework of AF Wheel driver with better error handling

**2.4.1 (2018-09-20)**

- Feature: Added driver for LM70 temperature sensor.
- Feature: Added driver for INA226-Q1 supply monitor.

**2.3.1 (2018-08-13)**

- Feature: Added driver for FL512S (moved from liba3200).

**2.2.2 (2018-05-30)**

- Bug: Fixed read command in Novatel 615.
- Improvement: unit-test - updated to use I2C plugin driver from libutil.
- Feature: Added driver for TSP382x watchdog.

**2.1.2 (2018-04-17)**

- Feature: TI TMP100 temperature sensor driver

**2.0.1 (2018-03-06)**

- Feature: CY15B102Q driver (including VMEM).
- Feature: HTPA32 driver

**1.0.0 (2017-06-06)**

- Feature: Novatel_615 driver
