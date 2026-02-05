Housekeeping System (libhk)
===========================

**4.5.3 (2020-05-11)**

- Bug: Fixed possible memory leak if hk persist failed opening file (LIBHK-120).
- Improvement: Updated dependencies.

**4.5.2 (2019-10-16)**

- Bug: Fixed that the feature for temporarily disabling auto-beacons did not countdown as expected (LIBHK-119).

**4.5.1 (2019-07-01)**

- Feature: When defining a beacon it is now possible to select a single index in an array parameter (LIBHK-117).
- Feature: Added function in API to set priority on all beacons
- Bug: Fixing issue where it was not possible to generate beacons after lowering priority (LIBHK-114).
- Bug: Fixing issue where collector threads where temporarily locked out during configration loading (LIBHK-116).
- Improvement: Improved logging when generating beacons.

**4.4.1 (2019-05-20)**

- Improvement: Added note in documentation about having mulitple beacons using the same table.
- Bug: Fixed an issue that could cause a crash if the 'hk_srv' command was called before initialization was done.
- Improvement: Include source code in distribution.

**4.3.1 (2019-04-10)**

- Improvement: Updated dependencies.

**4.2.3 (2019-04-10)**

- Bug: Fixed issue that could cause unexpected behaviour when collected tables where removed.
- Bug: The TTL used to signal when to clean up tables not being collected was not reset correctly.
- Feature: Default table download timeout has increased from 100 milliseconds to 200 milliseconds.
- Feature: It is now possible to tweak the table download timeout pr. table being collected.

**4.2.2 (2019-04-08)**

- Bug: Fixed issue that could cause a crash if collected tables where removed.

**4.2.1 (2019-02-26)**

- Feature: Added host commands (moved from libhk_client).

**4.1.4 (2019-01-29)**

- Improvement: Register all used log groups.

**4.1.3 (2018-12-20)**

- Improvement: Updated documentation.

**4.1.2 (2018-12-03)**

- Improvement: Fixed commands documentation.
- Bug: Requesting a beacon with tables not being correctly collected could lead to a crash.
- Bug: Running a 'hk_srv table show' command could cause a crash.
- Bug: Timestamp in requested beacons would be set to the requested time.

**4.1.1 (2018-11-22)**

- Feature: Added flag to completely enable/disable the housekeeping system.
- Bug: Fixed a memory leak that would occur if a remote table became unavailable.
- Breaking: Renamed root Command to better match overall scheme (hk_client -> hk, hk -> hk_srv).
- Feature: Added support for configuration being reloaded from client via CSP.
- Bug: Fixed issue where server could crash if .json was malformed.

**4.0.1 (2017-10-17)**

- Breaking: major rework of configuration of the housekeeping system.

**3.3.1 (2018-08-13)**

- Feature: Updated dependencies to latest version.

**3.2.1 (2018-05-30)**

- Improvement: To limit risk of running out of memory the collector store_intv is capped at 14400s/4 hours. If store_intv is 0, tables are truncated every 600s, but persisting is omitted.
- Bug: Fixed an issue where the node would crash when trying to persist housekeeping data.

**3.1.2 (2018-04-18)**

- Feature: Added param 4 store configuration.

**3.0.2 (2018-03-11)**

**2.0.0 (2018-06-22)**

.. toctree::
   lib/libhk_client/CHANGELOG
