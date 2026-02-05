A3200 (liba3200_client)
=======================

**3.5.3 (2019-12-17)**

- Improvement: Updated documentation.

**3.5.2 (2019-10-16)**

- Improvement: Updated documentation.

**3.5.1 (2019-04-09)**

- Improvement: Updated dependencies.

**3.3.7 (2019-02-26)**

- Breaking: Renamed LM70 to LM71.
- Feature: configuration table, changed default value for logging to FRAM to true.

**3.3.6 (2019-01-30)**

- Improvement: Updated API documentation.

**3.3.5 (2018-12-19)**

- Feature: Added support for disabling mounting of Flash 0 and 1.

**3.3.4 (2018-11-21)**

- Feature: Added support for disabling mounting of Flash 0 and 1.

**3.3.2 (2018-09-20)**

- Feature: Added SPI slave ids for LM71 (1) and extern gyro.

**3.3.1 (2018-08-13)**

- Breaking: Renamed tables: conf -> configuration, telem -> telemetry.
- Breaking: Removed CSP port definitions, use defines in libgscsp.
- Breaking: Restructured - moved fl512s driver to libthirdparty.

**3.2.1 (2018-05-30)**

- Improvement: added logical SPI slave definition for A3200 boards.
- Breaking: spn_fl512s - changed return codes to gs_error_t.
