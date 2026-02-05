A3200 (liba3200)
================

**3.8.1 (2019-12-17)**

- Feature: Added commands for onboard power switches.
- Bug: hmc5843 selftest was dependent on prior command sent to hmc5843.

**3.7.1 (2019-10-16)**

- Improvement: Increase delay between read in gyro self test to allow gyro to stabilize.
- Improvement: Fail gyro self test if FT value is zero. 
- Improvement: Added hmc5843 selftest
- Improvement: Added configUSE_NEWLIB_REENTRANT=1 in conf/FreeRTOSConfig.h to prevent crash from multiple tasks/thread sprintf floats.
- Improvement: Added gs_a3200_init_set_heap(), which controls location of heap (internal or external).
- Feature: Added commands for reading LM71.
- Improvement: Added gs_a3200_get_default_stack_size() as weak function, to allow stack size tuning in order to run in internal memory.
- Improvement: Generalized gs_a3200_init_twi() and gs_a3200_init_can() and made them public.

**3.6.1 (2019-07-03)**

- Feature: Append "(RAM)" to model, when compiling RAM image.
- Limitation: Disabled FreeRTOS runtime stats until implemented for FreeRTOS 10.2.0.
- Improvement: Improved FileSystem logging during boot/mount (logs goes to FRAM if enabled).
- Breaking: Removed gs_s3200_driver_register_commands(), replaced by libembed gs_embed_register_driver_commands().

**3.5.2 (2019-06-12)**

- Feature: Only initialize KISS UART, if KISS is enabled (in the board table).
- Improvement: Always build commands.

**3.5.1 (2019-04-09)**

- Feature: Added support for configuring log groups/appenders, using the "log_mask" parameter in the "configuration" table.

**3.4.1 (2019-02-26)**

- Breaking: Renamed LM70 to LM71.

**3.3.6 (2018-01-30)**

- Breaking: Removed A3200_DOCK_TABLE from A3200 protected storage.
- Improvement: Re-arranged boot order to enable persistent logging earlier.

**3.3.5 (2018-12-19)**

- Improvement: Updated documentation.

**3.3.4 (2018-11-21)**

- Improvement: Reflect actual Flash mounting status in telemetry.

**3.3.3 (2018-09-21)**

- Feature: Integrated latest liba3200dock and libthirdparty.

**3.3.1 (2018-08-13)**

- Breaking: dock_type in board table change from uint8_t to string.
- Improvement: spn_fl512s_read_data clears return buffer in case of SPI timeout instead of returning random data.
- Breaking: Cleaned up options and re-organized code to better support BSP and SDK builds (minor code changes).
- Breaking: Moved linker-scripts to conf (standard).
- Breaking: Removed server task - moved to a3200-sdk.
- Feature: Removed KEEP from linker-scrips..
- Breaking: Restructured - moved fl512s driver to libthirdparty and moved UFFS mounting procedure to libstorage.

**3.2.1 (2018-05-30)**

- Breaking: Refactored hmc5843 API, prefixed with `gs` and return gs_error_t error codes.
- Breaking: In the telemetry table boot_count and boot_cause is renamed to bootcount and bootcause to adhere to standard
- Breaking: In the configuration table fs_disable flag is removed, this invalidates FRAM copies of this table.
- Breaking: Changed spn_fl512s return codes to gs_error_t.

**3.1.1 (2018-04-18)**

- Breaking: Changed uptime in telemetry from mS to seconds.
- Feature: Added param 4 store configuration.

**3.0.1 (2018-03-14)**

- Breaking: Changed table layout.
- Breaking: Changed hook interface - provide callbacks before and after file system is mounted.
- Feature: Use gsbuildtools.

.. toctree::
   lib/liba3200_client/CHANGELOG
