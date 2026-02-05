#!/bin/env python3
# Copyright (C) 2012-2022 Jeppe Johansen <jeppe@j-software.dk>
import argparse
import logging
import sys
import time
from elftools.elf.elffile import ELFFile
from avr32 import AVR32, FLASH_BASE
from ftdi_jtag_adapter import FTDIJtagAdapter
from jtag_adapter import JtagAdapter

ADAPTERS = {
    "busblaster_v25": lambda frequency: FTDIJtagAdapter(0x0403, 0x6010, 0x0000, 0x0010, frequency=frequency, nSRST=(0x0200, 0x0800), nTRST=(0x0100, 0x0400)),
    "digilent_hs3":   lambda frequency: FTDIJtagAdapter(0x0403, 0x6014, 0x0080, 0x0080, frequency=frequency, nSRST=(0x2000, 0x1000)),
    "openmoko_dbv3":  lambda frequency: FTDIJtagAdapter(0x1457, 0x5118, 0x0000, 0x0010, frequency=frequency, nSRST=(0x0800, 0x0400), nTRST=(0x0200, 0x0100)),
}

def get_adapter(config, frequency) -> JtagAdapter:
    if config in ADAPTERS:
        return ADAPTERS[config](frequency)
    raise Exception("Unknown adapter: " + config)


def program_segment(dev: AVR32, address: int, mem_size: int, data: bytes, do_erase: bool, verify: bool):
    assert not do_erase, "No segment erase support yet"
    assert mem_size == len(data), "Data should be as long as memory segment"

    def progress(current, total):
        if sys.stdout.isatty():
            sys.stdout.write("\r  [%d%%] Wrote page %d out of %d" % (int(100*(current+1)/total), current + 1, total))
            sys.stdout.flush()
        else:
            print("  [%d%%] Wrote page %d out of %d" % (int(100*(current+1)/total), current + 1, total))

    try:
        dev.ProgramSequence(address, data, progress)

        if verify:
            if sys.stdout.isatty():
                sys.stdout.write("\n Verifying...")
                sys.stdout.flush()
            else:
                print(" Verifying...")
            
            readback = dev.ReadMemory(address, len(data))

            error_offset = -1
            for i in range(len(data)):
                if readback[i] != data[i]:
                    error_offset = i
                    break
            if error_offset >= 0:
                raise Exception("Verification failed. Difference in read data at address: %d. Expected %02x, but read %02x." % (address+error_offset, data[error_offset], readback[error_offset]))
    finally:
        if sys.stdout.isatty():
            sys.stdout.write("\n")
            sys.stdout.flush()


def main(programmer, flash=None, no_verify=False, chip_erase=True, detect=False, reset=False, fuses=None, dump=None):
    adapter = get_adapter(programmer, 12e6)
    try:
        adapter.SetSignal("TCK", False)

        # Do the AVR32 reset sequence
        adapter.SRST('0')
        time.sleep(0.1)
        adapter.SRST('z')

        if detect:
            adapter.DetectDevices()
            print(f"Detection completed. Found {len(adapter.Devices)} devices")
            for i, dev in enumerate(adapter.Devices):
                print(f"{i} Device: {dev.IDCode:8X} - IR Length: {dev.IRLength}")

        if flash or chip_erase or reset or fuses or dump:
            dev = AVR32(adapter)

            print(f" Chip Size: {dev.DeviceSize}. Page size: {dev.PageSize}")

            if dump:
                print("Dumping flash contents")
                with open(dump, "wb") as f:
                    f.write(dev._ReadBlockWords(dev.FlashBase, dev.DeviceSize // 4))

            if chip_erase:
                print("Performing chip erase")
                dev.ChipErase()

            if flash:
                if not chip_erase:
                    print("Erasing flash")
                    dev.EraseSequence()

                print("Flashing ELF file")
                with open(flash, "rb") as file:
                    elf = ELFFile(file)

                    for seg in elf.iter_segments("PT_LOAD"):
                        print(" Writing segment. Address: %8x Size: %x" % (seg.header["p_paddr"], seg.header["p_memsz"]))
                        program_segment(dev, seg.header["p_paddr"], seg.header["p_memsz"], seg.data(), not chip_erase, not no_verify)

            if fuses:
                print("Writing fuses")
                dev.WriteFuses(int(fuses, 0))

            if reset:
                print("Resetting...")
                adapter.SetSignal("TCK", True)

                adapter.SRST('0')
                time.sleep(0.1)
                adapter.SRST('z')

                adapter.SetSignal("TCK", False)
    finally:
        adapter.Close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--programmer", "-p", default="digilent_hs3", type=str, choices=ADAPTERS.keys(), help="Which JTAG adapter to use")
    parser.add_argument("--chip_erase", "-E", action="store_true", help="Perform full chip erase.")
    parser.add_argument("--reset", "-R", action="store_true", help="Perform chip reset after programming.")
    parser.add_argument("--dump", "-D", metavar="filename", default=None, type=str, help="Read the current FLASH contents (if not protected) out into a binary file.")
    parser.add_argument("--detect", "-d", action="store_true", help="Do detection of devices on JTAG bus")
    parser.add_argument("--flash", "-f", default=None, type=str, help="Path to ELF file to be programmed")
    parser.add_argument("--no-verify", "-V", action="store_true", help="Skip verifying flash")
    parser.add_argument("--fuses", "-GP", default=None, type=str, help="Program fuses")
#    parser.add_argument("--verbose", "-v",action="store_true", help="Verbose log output")
    args = parser.parse_args()

#    level = logging.DEBUG if args.verbose else logging.INFO
#    del args.verbose
#
#    logging.basicConfig(
#        filename='log_file_name.log',
#        level=level,
#        format= '[%(asctime)s] %(name)s %(levelname)s - %(message)s',
#        datefmt='%H:%M:%S'
#    )
#    logging.root.setLevel(level)

    main(**vars(args))
