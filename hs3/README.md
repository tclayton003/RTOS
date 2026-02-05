# Flashing tool for AVR32UC3 devices

This tool should support flashing UC3 devices through most FTDI MPSSE based JTAG dongles.

## Installing

### Dependencies
```
pip install -e requirements.txt
```

### Permissions (Linux)

Copy `support/99-ftdi.rules` to `/etc/udev/rules.d/`

Run
```
udevadm control --reload-rules
udevadm trigger
```

In some cases you might need to disconnect and connect the target JTAG adapter.
