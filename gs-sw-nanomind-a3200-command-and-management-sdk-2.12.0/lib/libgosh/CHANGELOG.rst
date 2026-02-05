Gomspace Shell (libgosh)
========================

**3.5.4 (2019-12-03)**

- Bug: Fixed race-condition in gs_gosh_redirect_stdio(), causing random failure in redirecting stdio correctly.
- Improvement: Updated dependencies.

**3.5.3 (2019-10-16)**

- Improvement: Updated documentation.

**3.5.2 (2019-05-16)**

- Improvement: Included source code in distribution.

**3.5.1 (2019-04-09)**

- Bug: Fixed bug in remote server, failed on 32 bit platform

**3.4.3 (2018-01-29)**

- Improvement: Added support for stopping stdio redirect thread.
- Bug: Fixed file handle leaks in stdio redirect.
- Feature: Added initialization functons: gs_gosh_remote_init(), gs_gscript_init().

**3.4.2 (2018-12-19)**

- Improvement: Updated documentation.

**3.4.1 (2018-11-21)**

- Feature: Split libgosh into libgosh (server) and libgosh_client (client).

**3.3.1 (2018-08-13)**

- Feature: added support for CSP service handler interface.

**3.2.1 (2018-05-30)**

- Breaking: Moved internal struct/defines out of public headers (used between host and client side).

**3.1.1 (2018-04-17)**

- Breaking: deprecated headers to libutil.

**3.0.1 (2018-03-14)**

- Breaking: Moved command and console APIs to libutil.
- Improvement: Improved g-script loading, avoid blocking calling thread and cause watchdog reset.

**1.0 (2015-05-08)**

- Feature: Changed to generic console interface (stdin/stdout)
- Feature: Remove need for command_setup calls on all platforms (apart from matlab)
- Feature: Remove need for USART_CONSOLE
- Feature: Support runtime setup of console
- Feature: Added commands for vmem
- Improvement: CMP clock printout
- Feature: Clock debug command
- Feature: Simplify toolchain configuration
- Feature: Clear screen on <CTRL+L>
- Feature: Automatic check for time.h
- Feature: Network redirection (telnet like)
- Feature: Moved code from libgomspace into libgosh
- Breaking: Deleted old csp_if_sia header
- Feature: Added console_messages_waiting

.. toctree::
   lib/libgosh_client/CHANGELOG
