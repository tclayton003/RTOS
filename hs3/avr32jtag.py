# Copyright (C) 2012-2022 Jeppe Johansen <jeppe@j-software.dk>
import logging
import struct

from jtag_adapter import JtagAdapter, JtagState

# JTAG instructions
IDCODE = 0x01
AVR_RESET = 0x0C
CHIP_ERASE = 0x0F
NEXUS_ACCESS = 0x10
MEMORY_WORD_ACCESS = 0x11
MEMORY_BLOCK_ACCESS = 0x12
CANCEL_ACCESS = 0x13
MEMORY_SERVICE = 0x14
MEMORY_SIZED_ACCESS = 0x15
SYNC = 0x17
AVR_HALT = 0x1C
BYPASS = 0x1F

SLV_OCD = 0x1
SLV_HSB = 0x4
SLV_HSB_ALT = 0x5
SLV_MSU = 0x6

IRLENGTH = 5

IDCODES = {
    0x200003F: "AT32UC3C0512C",
    0x200103F: "AT32UC3C0256C",
    0x200203F: "AT32UC3C0128C",
    0x200303F: "AT32UC3C064C",
    0x200403F: "AT32UC3C1512C",
    0x200503F: "AT32UC3C1256C",
    0x200603F: "AT32UC3C1128C",
    0x200703F: "AT32UC3C164C",
    0x200803F: "AT32UC3C2512C",
    0x200903F: "AT32UC3C2256C",
    0x200A03F: "AT32UC3C2128C",
    0x200B03F: "AT32UC3C264C",
}

class AVR32Jtag:
    def __init__(self, adapter: JtagAdapter):
        self._tap = adapter

        self.log = logging.getLogger("AVR32 JTAG")

        # Check device
        self._tap.SoftReset()
        self._tap.IR(IDCODE, IRLENGTH)
        self._idcode = self._tap.DR(0, 32)

        self._tap.SoftReset()

        self.model = self._idcode & 0x0FFFFFFF
        self.revision = (self._idcode >> 28) & 0xF

        chip = IDCODES.get(self.model, 'Unknown')

        print(f"Detected ID code: {self._idcode:8X} = {chip}")

        if chip == "Unknown":
            raise Exception("Could not detect chip type")
    
    def ChipErase(self):
        while True:
            ret = self._tap.IR(CHIP_ERASE,
                               IRLENGTH, JtagState.UpdateIR)
            self.log.debug(f" IP: {ret:2X}")
            if ret & 0x4:
                self.log.debug("Busy")
                continue
            if ret & 0x8:
                self.log.warning("Last operation failed")
            if ret & 0x10:
                raise Exception("Chip is protected")
            break
        self._tap.SetState(JtagState.Idle)

    def Reset(self, reset_bit: int):
        ret = self._tap.IR(AVR_RESET, IRLENGTH)
        self.log.debug(f" IP: {ret:2X}")
        if ret & 0x10:
            raise Exception("Chip is protected")
        
        ret = self._tap.DR(reset_bit, 1)
        self.log.debug(f" DP: {ret:2X}")

    def _ReadMemoryWord(self, address):
        while True:
            ret = self._tap.IR(MEMORY_WORD_ACCESS,
                               IRLENGTH, JtagState.UpdateIR)
            self.log.debug(f" IP: {ret:2X}")
            if ret & 0x4:
                self.log.debug("Busy")
                continue
            if ret & 0x8:
                self.log.warning("Last operation failed")
            if ret & 0x10:
                raise Exception("Chip is protected")
            break

        # Address phase
        dr = (((SLV_HSB << 30) | ((address & 0xFFFFFFFF) >> 2)) << 1) | 1
        while True:
            ret = self._tap.DR(dr, 35, JtagState.UpdateDR)
            self.log.debug(f" AP: {ret:9X}")
            if ret & 1:
                self.log.debug("Busy")
                continue
            if ret & 2:
                self.log.debug("Previous operation failed")
            break

        # Data phase
        while True:
            ret = self._tap.DR(0, 35, JtagState.UpdateDR)
            self.log.debug(f" DP: {ret:9X}")
            if (ret >> 32) & 1:
                self.log.debug("Busy")
                continue
            if (ret >> 32) & 2:
                raise Exception("Error while accessing memory")
            self._tap.SetState(JtagState.Idle)
            return ret & 0xFFFFFFFF

    def _WriteMemoryWord(self, address, value):
        while True:
            ret = self._tap.IR(MEMORY_WORD_ACCESS,
                               IRLENGTH, JtagState.UpdateIR)
            self.log.debug(f" IP: {ret:2X}")
            if ret & 0x4:
                self.log.debug("Busy")
                continue
            if ret & 0x8:
                self.log.warning("Last operation failed")
            if ret & 0x10:
                raise Exception("Chip is protected")
            break

        # Address phase
        dr = (((SLV_HSB << 30) | ((address & 0xFFFFFFFF) >> 2)) << 1)
        while True:
            ret = self._tap.DR(dr, 35, JtagState.UpdateDR)
            self.log.debug(f" AP: {ret:9X}")
            if ret & 1:
                self.log.debug("Busy")
                continue
            if ret & 2:
                self.log.debug("Previous operation failed")
            break

        # Data phase
        dr = (value & 0xFFFFFFFF) << 3
        while True:
            ret = self._tap.DR(dr, 35, JtagState.UpdateDR)
            self.log.debug(f" DP: {ret:9X}")
            if ret & 1:
                self.log.debug("Busy")
                continue
            if ret & 2:
                raise Exception("Error while accessing memory")
            self._tap.SetState(JtagState.Idle)
            break

    def _ReadBlockWords(self, address: int, word_count: int) -> bytes:
        result = bytearray(word_count * 4)

        # for i in range(0, word_count * 4, 4):
        #     res = self._ReadMemoryWord(address + i)
        #     struct.pack_into(">L", result, i, res & 0xFFFFFFFF)
        
        # return result

        res = self._ReadMemoryWord(address)
        struct.pack_into(">L", result, 0, res)

        if word_count > 1:
            while True:
                ret = self._tap.IR(MEMORY_BLOCK_ACCESS, IRLENGTH, JtagState.UpdateIR)
                self.log.debug(f" IP: {ret:2X}")
                if ret & 0x4:
                    self.log.debug("Busy")
                    continue
                if ret & 0x8:
                    raise Exception("MEMORY_WORD_ACCESS operation failed")
                if ret & 0x10:
                    raise Exception("Chip is protected")
                break

            word = 1
            while word < word_count:
                left = min(word_count - word, 32)

                # Schedule stuff
                total = left * 5
                for _ in range(left):
                    self._tap.SetState(JtagState.SelectDR)
                    self._tap.SetState(JtagState.ShiftDR)
                    ret = self._tap.ScheduleDRIn(5)

                res = self._tap.ReadScheduled(total)
                for idx in range(0, total, 5):
                    ret = res[idx+4]
                    if ret & 1:
                        self.log.debug("Busy")
                        continue
                    if ret & 2:
                        raise Exception("Error while accessing memory")

                    result[word*4:word*4+4] = reversed(res[idx:idx+4])
                    word += 1
                    
            self._tap.SetState(JtagState.Idle)
            self._tap.Flush()
        
            # for word in range(1, word_count):
            #     # Data phase
            #     # self._tap.SetState(JtagState.SelectDR)
            #     while True:
            #         ret = self._tap.DR(0, 34, JtagState.ShiftDR)
            #         self.log.debug(f" DP: {ret:9X}")
            #         if (ret >> 32) & 1:
            #             self.log.debug("Busy")
            #             continue
            #         if (ret >> 32) & 2:
            #             raise Exception("Error while accessing memory")
                    
            #         struct.pack_into(">L", result, word*4, ret & 0xFFFFFFFF)
            #         break

        return result

    def _WriteBlockWords(self, address: int, data :bytes):
        # for i in range(0, len(data), 4):
        #     value = struct.unpack_from(">L", data, i)[0]
        #     self._WriteMemoryWord(address + i, value)

        if len(data) <= 0:
            return
        if (len(data) % 4) != 0:
            raise Exception("Data to be written is not a multiple of 4 bytes")

        value = struct.unpack_from(">L", data, 0)[0]
        self._WriteMemoryWord(address, value)

        word_count = len(data) // 4

        if word_count > 1:
            while True:
                ret = self._tap.IR(MEMORY_BLOCK_ACCESS, IRLENGTH, JtagState.UpdateIR)
                self.log.debug(f" IP: {ret:2X}")
                if ret & 0x4:
                    self.log.debug("Busy")
                    continue
                if ret & 0x8:
                    raise Exception("MEMORY_WORD_ACCESS operation failed")
                if ret & 0x10:
                    raise Exception("Chip is protected")
                break

            self._tap.SetState(JtagState.Idle)
        
            for word in range(1, word_count):
                # Data phase
                self._tap.SetState(JtagState.SelectDR)
                while True:
                    ret = self._tap.DR(0, 2, JtagState.ShiftDR)

                    self.log.debug(f" DP: {ret:9X}")
                    if ret & 1:
                        self.log.debug("Busy")
                        self._tap.SetState(JtagState.SelectDR)
                        continue
                    if ret & 2:
                        raise Exception("Error while accessing memory")
                    break

                value = struct.unpack_from(">L", data, word * 4)[0]
                self._tap.DROut(value, 32, JtagState.UpdateDR)
        
            # for word in range(1, word_count):
            #     # Data phase
            #     self._tap.SetState(JtagState.SelectDR)
            #     while True:
            #         value = struct.unpack_from(">L", data, word * 4)[0]
            #         ret = self._tap.DR(value << 2, 2, JtagState.UpdateDR)

            #         self.log.debug(f" DP: {ret:9X}")
            #         if ret & 1:
            #             self.log.debug("Busy")
            #             continue
            #         if ret & 2:
            #             raise Exception("Error while accessing memory")
            #         break

        self._tap.SetState(JtagState.Idle)
        self._tap.Flush()