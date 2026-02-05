File Transfer Protocol (libftp)
===============================

**5.2.4 (2020-05-18)**

- Bug: Fixed issue resuming a file upload.

**5.2.3 (2020-04-17)**

- Bug: issue with temporary file upload if ftp done is lost - for now not using temp file.

**5.2.2 (2020-03-20)**

- Bug: fixed issue with resume of a file transfer, causing the overall transfer to fail. Occurred if resume was done too quickly.
- Feature: ftp transfer is now done to a temporary file, and renamed on successful transfer.
- Improvement: Reduced ftp server timeout (CSP read) from 180 to 60 seconds.

**5.2.1 (2019-10-16)**

- Improvement: Updated client and dependencies.

**5.1.3 (2019-07-03)**

- Bug: mkdir - always create directory on server with permission 0777 (fixes endianess problem between client and server, which causes invalid/none permission on created directory).

**5.1.2 (2019-05-19)**

- Improvement: Include source code in distribution.

**5.1.1 (2019-04-09)**

- Improvement: Updated dependencies.

**5.0.5 (2019-02-26)**

- Feature: Added gs_ftp_csp_service_handler_set_stack_size().

**5.0.4 (2019-01-29)**

- Improvement: Register all used log group.

**5.0.3 (2018-12-19)**

- Bug: Updating client
- Improvement: Updated documentation.

**5.0.2 (2018-11-29)**

- Feature: Updated client

**5.0.1 (2018-11-21)**

- Breaking: Updated client

**4.3.3 (2018-09-21)**

- Improvement: Updated client and dependencies.

**4.3.1 (2018-08-13)**

- Improvement: Updated client and dependencies.

**4.2.1 (2018-05-30)**

- Improvement: Updated client and dependencies.

**4.1.1 (2018-04-18)**

- Improvement: Removed dependency to libgosh

**4.0.1 (2018-03-06)**

**3.0.0 (2017-07-02)**

**2.0 (2015-05-07)**

- Feature: 32-bit size field
- Feature: Added ftp zip/unzip
- Improvement: Do not sync for every chunk on linux for faster transfers

**1.0 (2015-03-09)**

- Feature: FTP client work on nanomind a3200
- Feature: FTP server works on linux too
- Feature: Updated newlib backend

.. toctree::
   lib/libftp_client/CHANGELOG
