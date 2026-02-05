Embedded File Storage (libstorage)
==================================

**3.7.1 (2020-03-18)**

- Improvement: Added recursive remove to VFS API and as option to GOSH command.
- Bug: Fixed undefined behavior when file paths exceeded 50 characters.

**3.6.1 (2019-10-16)**

- Improvement: Updated dependencies.
- Breaking: Re-factored code for mounting, making vfs_mount() the actual mounting function.
- Improvement: Use endian definition from libutil.
- Improvement: Added VFS required define for newlib ('HAVE_OPENDIR=1'), moved from a3200-sdk.
- Breaking: Changed filesystem syscalls to implement re-entrant versions - requires libembed 1.8.

**3.5.2 (2019-07-03)**

- Improvement: Changed command/gosh definitions to const.

**3.5.1 (2019-04-08)**

- Improvement: Updated dependencies.

**3.4.4 (2019-01-29)**

- Improvement: Register all used log groups.

**3.4.3 (2018-12-19)**

- Improvement: Updated documentation.

**3.4.2 (2018-11-21)**

- Improvement: Improved output from commands (gosh).

**3.4.1 (2018-10-08)**

- Breaking: gs_fl512s_uffs_mount() now supports mounting two flash dies.

**3.3.2 (2018-09-20)**

- Improvement: Print output to stream, instead of stdout.

**3.3.1 (2018-08-13)**

- Breaking: Removed support for FAT (SD), and removed unused drivers: FAT (SD), UFFS.
- Breaking: Re-structured include and source, divided into 3rd part and GomSpace code.
- Breaking: Re-structured by moving mounting procedure to this lib from liba3200.
- Breaking: Re-structured - including driver for fl512s from libthirdparty instead of liba3200.
- Feature: Added support for mounting both dies of the fl512s.

**3.2.1 (2018-05-30)**

- Breaking: Updated internal use of reset API from libembed, which now takes a reset cause.
- Breaking: updated to new spn_fl512s API (changed return codes to gs_error_t).

**3.1.1 (2018-04-17)**

- Improvement: Removed libgosh and libgscsp dependencies.

**3.0.1 (2018-03-06)**

**2.0.0 (2018-05-04)**
