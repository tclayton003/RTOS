# Copyright (C) 2012-2022 Jeppe Johansen <jeppe@j-software.dk>
import time
import typing

from avr32jtag import AVR32Jtag
from jtag_adapter import JtagAdapter

FSR_FRDY_MASK = 0x00000001
FSR_FRDY_OFFSET = 0
FSR_PROGE_MASK = 0x00000008
FSR_PROGE_OFFSET = 3
FSR_LOCKE_MASK = 0x00000004
FSR_LOCKE_OFFSET = 2
FSR_FSZ_MASK = 0x0000E000
FSR_FSZ_OFFSET = 13
PR_FSZ_MASK = 0x0000000F
PR_FSZ_OFFSET = 0
PR_PSZ_MASK = 0x00000700
PR_PSZ_OFFSET = 8
FCMD_FCMD_MASK = 0x0000001F
FCMD_FCMD_OFFSET = 0
FCMD_PAGEN_MASK = 0x00FFFF00
FCMD_PAGEN_OFFSET = 8
FCMD_KEY_MASK = 0xFF000000
FCMD_KEY_OFFSET = 24
FGPFR_LOCK_MASK = 0x0000FFFF
FGPFR_LOCK_OFFSET = 0
USER_PAGE_OFFSET = 0x00800000
WRITE_PROTECT_KEY = 0xA5000000
CMD_WRITE_PAGE = 1
CMD_ERASE_PAGE = 2
CMD_CLEAR_PAGE_BUFFER = 3
CMD_LOCK_REGION = 4
CMD_UNLOCK_REGION = 5
CMD_ERASE_ALL = 6
CMD_WRITE_GP_FUSE_BIT = 7
CMD_ERASE_GP_FUSE_BIT = 8
CMD_SET_SECURITY_BIT = 9
CMD_PROGRAM_GP_FUSE_BYTE = 10
CMD_WRITE_USER_PAGE = 13
CMD_ERASE_USER_PAGE = 14

FLASH_BASE = 0x80000000
HFLASHC_BASE = 0xFFFE0000


class AVR32(AVR32Jtag):
    def __init__(self, adapter: JtagAdapter, flash_base: int = FLASH_BASE):
        AVR32Jtag.__init__(self, adapter)

        self.FCR = HFLASHC_BASE + 0x0
        self.FCMD = HFLASHC_BASE + 0x4
        self.FSR = HFLASHC_BASE + 0x8
        self.PR = HFLASHC_BASE + 0xC
        self.VR = HFLASHC_BASE + 0x10
        self.FGPFRHI = HFLASHC_BASE + 0x14
        self.FGPFRLO = HFLASHC_BASE + 0x18

        self._flash_base = flash_base
        self._page_size = None
        self._device_size = None

    def _WriteCommand(self, command):
        self._WriteMemoryWord(self.FCMD, command)

    def _GetRegister(self, reg):
        return self._ReadMemoryWord(reg)

    def _WaitFlashReady(self):
        starttime = time.time()
        while (time.time() - starttime) < 1:
            fsrReg = self._GetRegister(self.FSR)
            # If LOCKE bit in FSR set
            if (fsrReg & FSR_LOCKE_MASK) >> FSR_LOCKE_OFFSET:
                raise Exception(
                    "Programming of at least one locked lock region has happend since the last read of FSR")
            # If PROGE bit in FSR set
            if (fsrReg & FSR_PROGE_MASK) >> FSR_PROGE_OFFSET:
                raise Exception(
                    "An invalid command and/or bad keywords were written in the Flash Command Register")
            # Read FRDY bit in FSR
            if (fsrReg & FSR_FRDY_MASK) >> FSR_FRDY_OFFSET:
                return  # FLASH ready for next operation
        
        raise Exception("Timeout while waiting for flash controller to be ready")

    def _ClearPageBuffer(self):
        command = WRITE_PROTECT_KEY | CMD_CLEAR_PAGE_BUFFER
        self._WaitFlashReady()
        self._WriteCommand(command)
        self._WaitFlashReady()

    @property
    def DeviceSize(self) -> int:
        if not self._device_size:
            self._GetInternalFlashSize()
        return self._device_size

    @property
    def FlashBase(self) -> int:
        return self._flash_base

    @property
    def PageSize(self) -> int:
        if not self._page_size:
            self._GetInternalFlashSize()
        return self._page_size

    def _GetInternalFlashSize(self):
        prReg = self._GetRegister(self.PR)

        # psz = (prReg & PR_PSZ_MASK) >> PR_PSZ_OFFSET
        # self._page_size = 4 * (32 << psz)
        self._page_size = 512

        fsz = (prReg & PR_FSZ_MASK) >> PR_FSZ_OFFSET
        size = 0
        if fsz == 0:
            size = 4 * 1024
        elif fsz == 1:
            size = 8 * 1024
        elif fsz == 2:
            size = 16 * 1024
        elif fsz == 3:
            size = 32 * 1024
        elif fsz == 4:
            size = 48 * 1024
        elif fsz == 5:
            size = 64 * 1024
        elif fsz == 6:
            size = 96 * 1024
        elif fsz == 7:
            size = 128 * 1024
        elif fsz == 8:
            size = 192 * 1024
        elif fsz == 9:
            size = 256 * 1024
        elif fsz == 10:
            size = 384 * 1024
        elif fsz == 11:
            size = 512 * 1024
        elif fsz == 12:
            size = 768 * 1024
        elif fsz == 13:
            size = 1024 * 1024
        elif fsz == 14:
            size = 2048 * 1024
        else:
            raise Exception("Unknown flash size")
        self._device_size = size

    def UnlockRegion(self, offset, size):
        if (offset >= USER_PAGE_OFFSET) and (offset < USER_PAGE_OFFSET + self.PageSize):
            return  # the user page doesn't need unlocking
        if (offset >= self.DeviceSize) or (offset+size > self.DeviceSize):
            raise Exception(
                "Region to be unlock lies outside flash address space")
        lastpagetounlock = (offset+size) // self.PageSize
        # compute start offset of page to write to
        page = offset & ~(self.PageSize - 1)
        pagenr = offset // self.PageSize
        while pagenr <= lastpagetounlock:
            command = WRITE_PROTECT_KEY | CMD_UNLOCK_REGION
            # include the correct page number in the command
            command |= ((pagenr << FCMD_PAGEN_OFFSET) & FCMD_PAGEN_MASK)
            # Unlocking page: pagenr
            self._WaitFlashReady()
            self._WriteCommand(command)  # execute unlock page command
            self._WaitFlashReady()
            page += self.PageSize
            offset = page
            pagenr = offset // self.PageSize

    def UnlockEntireFlash(self):
        self.UnlockRegion(0, self.DeviceSize)

    def EraseSequence(self):
        self._WaitFlashReady()
        command = WRITE_PROTECT_KEY | CMD_ERASE_ALL
        self._WriteCommand(command)
        self._WaitFlashReady()

    def EraseUserPage(self):
        command = WRITE_PROTECT_KEY | CMD_ERASE_USER_PAGE
        self._WaitFlashReady()
        self._WriteCommand(command)
        self._WaitFlashReady()

    def EraseRegionSequence(self, offset, size):
        if (offset >= USER_PAGE_OFFSET) and (offset < USER_PAGE_OFFSET + self.PageSize):
            self.EraseUserPage()

            lastpagetoerase = (offset+size) // self.PageSize
            # compute start offset of page to write to
            page = offset & ~(self.PageSize - 1)
            pagenr = offset // self.PageSize
            while (pagenr <= lastpagetoerase):
                Command = WRITE_PROTECT_KEY | CMD_ERASE_PAGE
                Command |= ((pagenr << FCMD_PAGEN_OFFSET) & FCMD_PAGEN_MASK)
                # include the correct page number in the command
                self._WaitFlashReady()
                self._WriteCommand(Command)  # execute page erase command
                self._WaitFlashReady()

                page += self.PageSize
                offset = page
                pagenr = offset // self.PageSize

    def SetGPFuseByte(self, index, value):
        if index > 3 or index < 0:
            raise Exception("Invalid fuse byte")
        
        command = WRITE_PROTECT_KEY | CMD_PROGRAM_GP_FUSE_BYTE | (index << FCMD_PAGEN_OFFSET) | ((value & 0xFF) << (FCMD_PAGEN_OFFSET+3))

        self._WaitFlashReady()
        self._WriteCommand(command)
        self._WaitFlashReady()

    def WriteFuses(self, fuses: int):
        """Assumes fuses are 32 bits"""
        for i in range(4):
            self.SetGPFuseByte(i, (fuses >> (8*i)) & 0xFF)

    def _ProgramPage(self, address: int, cmd: int, offset: int, data: bytes):
        bufferPacket = data
        if (offset > 0) or (offset+len(data) < self.PageSize):
            bufferPacket = self._ReadBlockWords(address, self.PageSize // 4)
            bufferPacket[offset:offset+len(data)] = data

        self._ClearPageBuffer()
        self._WriteBlockWords(address, bufferPacket)

        command = WRITE_PROTECT_KEY | cmd
        self._WaitFlashReady()
        self._WriteCommand(command)
        self._WaitFlashReady()

    def ProgramUserPage(self, page_offset: int, DataBuffer: bytes):
        self._ProgramPage(self.FlashBase + USER_PAGE_OFFSET, CMD_WRITE_USER_PAGE, page_offset, DataBuffer)

    def ProgramSequence(self, address: int, data_buffer: bytes, progress_callback: typing.Callable[[int, int], None]):
        if (address >= self.FlashBase + USER_PAGE_OFFSET) and (self.FlashBase + address < USER_PAGE_OFFSET + self.PageSize):
            self.ProgramUserPage(address - (self.FlashBase + USER_PAGE_OFFSET), data_buffer)
            return

        if (address >= self.FlashBase + self.DeviceSize) or (address+len(data_buffer) > self.FlashBase + self.DeviceSize):
            raise Exception("Region to be programmed lies outside flash address space")

        page_number = (address - self.FlashBase) // self.PageSize
        first_page = page_number
        last_page = (address + len(data_buffer) + self.PageSize - 1 - self.FlashBase) // self.PageSize

        index = 0
        bytes_left = len(data_buffer)

        while bytes_left > 0:
            bytes_in_page = min(bytes_left, self.PageSize - address % self.PageSize)

            # Write bytes
            command = CMD_WRITE_PAGE
            command |= page_number << FCMD_PAGEN_OFFSET
            self._ProgramPage(page_number * self.PageSize + self.FlashBase, command, address % self.PageSize, data_buffer[index:index + bytes_in_page])

            if progress_callback:
                progress_callback(page_number - first_page, last_page - first_page)

            address = 0
            page_number += 1
            index += bytes_in_page
            bytes_left -= bytes_in_page

    def ReadMemory(self, address: int, size: int) -> bytes:
        aligned_address = address & 0xFFFFFFFC
        aligned_end = (address + size + 3) & 0xFFFFFFFC

        data_offset = address - aligned_address

        data = self._ReadBlockWords(aligned_address, (aligned_end - aligned_address) // 4)
        return data[data_offset:data_offset+size]
