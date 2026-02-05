Parameter System (libparam)
===========================

**4.8.1 (2020-03-19)**

- Improvement: Updated client and tests.

**4.7.3 (2019-12-03)**

- Bug: filestore auto-persist, if auto persist file already existed, later 'update' would fail because file was opened in Read mode.
- Improvement: Updated client and dependencies.

**4.7.2 (2019-10-16)**

- Improvement: Improved check on VMEM store size.
- Improvement: Added validation of clear of VMEM stores.

**4.7.1 (2019-05-16)**

- Improvement: Include source code in distribution.

**4.6.1 (2019-04-09)**

- Breaking: Updated telemetry appender to new log appender interface.

**4.5.4 (2019-02-26)**

- Improvement: Updated client.

**4.5.3 (2019-01-29)**

- Improvement: Register all used log group.

**4.5.2 (2018-12-19)**

- Feature: Added Log Telemetry appender: Provides telemetry of Log errors/warnings in Param.
- Feature: added support for new rparam load/save from/to named stores.
- Improvement: Updated documentation.

**4.5.1 (2018-11-21)**

- Breaking: removed 'rparam clear' functionality.
- Bug: Table 0 was not always read correctly through the Python API.

**4.4.1 (2018-09-20)**

- Feature: Allow for tables larger than 1K in vmem stores.
- Bug: prevent stripping leading spaces, when setting string parameters.
- Feature: Added support for anonymous table, see gs_param_table_init().

**4.3.1 (2018-08-13)**

- Feature: table create: added support for default data values as a data struct.

**4.2.2 (2018-05-30)**

- Breaking: Renamed Parameter IO API to Parameter Protocol (PP).
- Bug: fixed missing auto-persist of parameter changed from GOSH.
- Breaking: removed support for "all" in "param save <table> all".

**4.1.1 (2018-04-17)**

- Breaking: All new table stores, store values in network order.
- Feature: Added param commands: tableinfo, storeinfo.
- Feature: Added support for protected/locked tables in FRAM.
- Bug: Call callback function, when loading tables.
- Feature: Added binary file store.
- Feature: Re-introduced some of the old structures, in order to support "old" clients.
- Bug: Fixed auto-persist paramtres (value was not persisted).

**4.0.1 (2018-03-12)**

- Breaking: Number of APIs have been broken, especially regarding creating tables and setting up stores.
- Feature: Configure multiple stores per table, e.g. FRAM, FRAM (locked), File, etc.
- Feature: Variable store size, i.e. improving usage of storage media.
- Improvement: Genereal error handling and logging.

**2.0 (2015-05-08)**

- Breaking: Allow full table download to be split into multiple frames
- Bug: Init persist arrays.
- Feature: Added param_save and param_load to param_file backend
- Breaking: Serializer API, split API and introduced flags.
- Feature: Make deserializer support packet data format
- Bug: Add safety check on infinite serializer bug
- Feature: Added 'param tables' command

**1.0 (2015-03-16)**

- Feature: Supports RAM, FRAM, File system.
- Feature: Local host 'param' commands
- Feature: CSP Client/Server 'rparam' commands
- Feature: Arrays of parameters
- Feature: Table locking
- Feature: Serializer / Deserializer

.. toctree::
   lib/libparam_client/CHANGELOG
