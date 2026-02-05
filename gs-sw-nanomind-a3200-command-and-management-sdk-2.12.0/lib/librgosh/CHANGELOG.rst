Remote GOSH (librgosh)
======================

**1.2.3 (2020-03-24)**

- Bug: Free resources in case of failure, i.e. connections, memory.
- Improvement: Increased timeout on CSP receive, to compensate for retransmissions on RDP layer.

**1.2.2 (2020-03-18)**

- Improvement: Updated client.

**1.2.1 (2020-01-23)**

- Improvement: Updated dependencies.

**1.1.3 (2019-10-16)**

- Bug: Reduced max CSP package size from 226 to 180 bytes, so it can be send via the AX100.

**1.1.2 (2019-05-19)**

- Improvement: Include source in distribution.

**1.1.1 (2019-04-10)**

- Improvement: Updated dependencies.

**1.0.3 (2019-01-28)**

- Improvement: Register all used log groups.
- Improvement: Updated API doc.

**1.0.2 (2018-12-21)**

- Bug: z7000 file seek was incompatible with rgosh 

**1.0.1 (2018-12-19)**

- Improvement: Updated documentation.

**1.0.0 (2018-11-21)**

- Breaking: Updated client and client protocol messages
- Improvement: Send multiple RGOSH responses when long messages are transfered.
- Improvement: Logging cleaned-up
- Bug: Handle NULL values for Group/Key/Value pairs.
- Improvement: Balance workload better between RGOSH worker threads.
- Bug: Support RDP connections - reduce the MTU

**0.1.2 (2018-08-07)**

- Feature: Support for running GOSH commands remotely.

.. toctree::
   lib/librgosh_client/CHANGELOG
