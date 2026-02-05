Log stores (liblog)
===================

**3.6.1 (2019-10-16)**

- Improvement: Updated tests.

**3.5.2 (2019-05-16)**

- Improvement: Include source code in distribution.

**3.5.1 (2019-04-09)**

- Improvement: Optimized memory logger (reduced number of reads/writes).
- Improvement: Added locking to prevent corruption.
- Feature: Added support for 'clear'.

**3.4.2 (2018-12-19)**

- Improvement: Updated documentation.

**3.4.1 (2018-11-21)**

- Breaking: The Remote log service has been removed. Replaced by libutil Log appender + Rgosh
- Breaking: Re-factored the vmem log store to plug into the new Log Appender Framework.
- Feature:  New memory Log appender which can be live in multiple instances.

**3.3.2 (2018-09-20)**

- Improvement: Prepared commands for remote-gosh.

**3.3.1 (2018-08-13)**

- Feature: Build shared library.

**3.2.1 (2018-05-30)**

- Breaking: Moved log API (front-end) to libutil, moved csp log hook to libgscsp.

**3.1.1 (2018-04-17)**

- Improvement: Removed dependency to libgosh

**3.0.1 (2018-03-06)**

**1.0 (2015-05-08)**

- Feature: Log output to console/stdout
- Feature: Store logs in FRAM (using VMEM interface).
- Feature: Adjustable log levels (ala gstreamer)
- Feature: Support for libcsp debug hook
- Feature: Very low memory footprint (1 ptr + 2 bytes per log domain)
- Feature: Replaces driver debug, csp debug and the former cdh log system
