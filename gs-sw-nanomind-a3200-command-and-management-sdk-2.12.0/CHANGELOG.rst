A3200 SDK
=========

**2.12.0 (2020-06-12)**

- Improvement: Updated libcsp to new 1.6.0 (libgscsp 2.x->3.x)
- Improvement: Updated libraries liba3200, libasf, libftp, libhk
- Feature: house-keeping tables for BPX, eps etc default to invalid values if read fails

**2.11.2 (2020-04-22)**

- Improvement: Updated libs (libasf, libftp and more)
- Feature: Mission-lib is always included in SDK (No longer referred)

**2.11.1 (2020-03-20)**

- Improvement: Changed size summary to sysv format to get details on section sizes.
- Bug: BPX temperatures are in degrees celcius (°C)
- Improvement: Updated libraries.

**2.10.1 (2019-10-18)**

- Limitation: if enabling FreeRTOS configUSE_NEWLIB_REENTRANT (default), use AVR32 toolchain: 3.4.2_gs1 or later - otherwise crash occurs, when a task terminates (libembed).
- Limitation: Disabled FreeRTOS runtime stats until implemented for FreeRTOS 10.2.0.
- Improvement: Implement retargetable locks for newlib file operations. This allows for multi-threaded file operations. Requires toolchain 3.4.2_gs1 or later (libembed).
- Feature: Added generic scheduler for collecting telemetry from nodes, not supporting rparam internface.
- Bug: FreeRTOS: Enabled re-entrant per task/thread (configUSE_NEWLIB_REENTRANT=1 in FreeRTOSConfig.h) to prevent crashes when printing floats.
- Bug: Updated toolchain with patched newlib, that supports file-locking, prevent possible file corruption if multiple tasks accesses the file system.
- Improvement: Updated libraries and documentation.
- Breaking: Removed Health System (libhealth).

**2.9.1 (2019-07-03)**

- Bug: Updated to FreeRTOS 10.2.0 to fix crash issue, when running from RAM image.
- Limitation: Disabled FreeRTOS runtime stats until implemented for FreeRTOS 10.2.0.
- Improvement: Updated libraries and documentation.

**2.8.2 (2019-06-12)**

- Improvement: Updated libraries and documentation.

**2.8.1 (2019-05-20)**

- Improvement: Updated libraries and documentation.

**2.7.1 (2019-04-11)**

- Improvement: Updated libraries and documentation.

**2.6.1 (2019-02-27)**

- Improvement: Updated libraries and documentation.
- Feature: Added technology preview of new Health System (libhealth).

**2.5.1 (2019-01-30)**

- Improvement: Updated libraries and documentation. Log groups must be registered in order to change level.

**2.4.2 (2018-12-20)**

- Improvement: Updated libraries and documentation.

**2.4.1 (2018-11-23)**

- Bug: Missing avrisp.rules file was added to dist. tarball.
- Feature: Moved initialization of flight-planner to after file system mounted.
- Feature: Updated libraries.

**2.3.1 (2018-08-14)**

- Feature: Updated libraries.
- Feature: Use gs_csp_service_dispatcher(), instead of server task in liba3200.

**2.2.2 (2018-05-31)**

- Feature: Updated libraries.

**2.1.1 (2018-04-18)**

- Feature: Changed param 4 store configuration to use stores defined in liba3200, libhk.
- Feature: Added Doxygen generated API documentation to html manual.
- Feature: Added default clients.
- Breaking: The format used to store paramters in FRAM has been changed, preventing load of existing data.

**2.0.1 (2018-03-14)**

- Breaking: Removed libclients, replacing it very limited set of clients added.
- Breaking: updated  all libraries to latest version, e.g. libparam version 4.

**2.0.0 (2017-09-06)**

- Feature: Use gsbuildtools for building and generate documentation.

**1.1 (2017-03-08)**

**1.0 (2017-01-10)**
