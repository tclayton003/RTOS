#!/usr/bin/env python
# encoding: utf-8
# Copyright (c) 2013-2018 GomSpace A/S. All rights reserved.

import os.path
import subprocess
from optparse import OptionParser

# Command line arguments
usage = "usage: %prog [options]"
parser = OptionParser(usage=usage)
parser.add_option("-p", "--part", default="uc3c0512c")
(options, args) = parser.parse_args()

# Interpret arguments
part = options.part
elf = "build/nanomind-bsp.elf"

# Search for image
if not os.path.isfile(elf):
    parser.error("Invalid filename %s" % elf)

# Command to execute
cmdargs = 'avr32program --part %(part)s program -finternal@0x80000000 -F elf -O 0 -E -R -r %(elf)s' % locals()

# Execute command
print cmdargs
subprocess.call(cmdargs, shell=True)
