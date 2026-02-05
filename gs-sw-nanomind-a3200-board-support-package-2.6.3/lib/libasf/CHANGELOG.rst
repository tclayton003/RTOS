ASF (libasf)
============

**3.6.1 (2019-10-16)**

- Improvement: AVR32 TWIM/I2C: improved logging.
- Breaking: Removed newlib port/stubs for avr32 and samc (moved to libembed).

**3.5.1 (2019-07-03)**

- Feature: Added FreeRTOS 10.2.0.

**3.4.1 (2019-04-08)**

- Improvement: Updated tools dependencies.

**3.3.6 (2019-02-26)**

- Improvement: Removed/reduced configuration used for compile check's, not used on real targets.

**3.3.5 (2019-01-28)**

- Feature: Changed log group for twim/twis to "i2c".

**3.3.4 (2018-12-19)**

- Improvement: TWIM NACK warning moved to debug.
- Improvement: Updated documentation.

**3.3.3 (2018-11-21)**

- Improvement: Updated default log settings.

**3.3.2 (2018-09-10)**

- Bug: Removed log warnings from ISR, readded warnings on TWI NACK and ARB, enable idle interrupt on commandready

**3.3.1 (2018-08-13)**

- Improvement: Ensure twim_pdc_transfer release locks in case of error.

**3.2.1 (2018-05-30)**

- Breaking: AVR32: Moved exception handler to libembed.
- Improvement: Changed SPI driver use standard SPI log group.

**3.1.1 (2018-04-17)**

- Feature: SAMC21: Added Invalidate Cache command, missing in ASF implementation.

**3.0.1 (2018-03-06)**

- Breaking: Removed libcsp dependency
- Breaking: Moved GomSpace code to libembed.
- Feature: Added support fo AT32UC3C1512C.
