# Copyright (C) 2012-2022 Jeppe Johansen <jeppe@j-software.dk>
import logging
import struct
import typing

from pyftdi.ftdi import Ftdi
from jtag_adapter import JtagAdapter, JtagState, NavigateState, NextState


DIR = 0x000B
INIT = 0x0008

MPSSE_SET_DATA_LOW   = 0x80
MPSSE_READ_DATA_LOW  = 0x81
MPSSE_SET_DATA_HIGH  = 0x82
MPSSE_READ_DATA_HIGH = 0x83
MPSSE_LOOPBACK_ON    = 0x84
MPSSE_LOOPBACK_OFF   = 0x85
MPSSE_SET_CLOCK      = 0x86
MPSSE_SEND_IMMEDIATE = 0x87
MPSSE_DISABLE_DIV5   = 0x8A
MPSSE_ENABLE_DIV5    = 0x8B
MPSSE_ENABLE_3PHASE  = 0x8C
MPSSE_DISABLE_3PHASE = 0x8D
MPSSE_ENABLE_RCK     = 0x96
MPSSE_DISABLE_RCK    = 0x97

MPSSE_CLK_ON_WRITE = 0x01
MPSSE_BITMODE      = 0x02
MPSSE_CLK_ON_READ  = 0x04
MPSSE_LSB_FIRST    = 0x08
MPSSE_DO_TDO       = 0x10
MPSSE_DO_TDI       = 0x20
MPSSE_DO_TMS       = 0x40


def get_transition_seq(src, dst):
    tms = 0
    cnt = 0

    if src == JtagState.Unknown or dst == JtagState.Unknown:
        return (0, 0)

    curr = src
    
    while curr != dst:
        sel = NavigateState[curr][int(dst)]
        tms = tms | (sel << cnt)
        curr = NextState[curr][sel == 1]
        cnt += 1

    return (tms, cnt)


class FTDIJtagAdapter(JtagAdapter):
    def __init__(self, vid, pid, init, dir, frequency=6e6, nSRST: typing.Tuple[int, int] = None, nTRST: typing.Tuple[int, int] = None):
        JtagAdapter.__init__(self)
        self._dev = Ftdi()

        self.log = logging.getLogger("FTDI_JTAG")

        self._transition_table = {}

        for from_state in JtagState:
            for to_state in JtagState:
                self._transition_table[(from_state, to_state)] = get_transition_seq(from_state, to_state)
        
        self._dir = DIR      | (dir & 0xFFF0)
        self._initial = INIT | (init & 0xFFF0)

        self._has_srst = False
        self._has_trst = False

        if nSRST is not None:
            self._has_srst = True
            self._srst = nSRST

            self._dir     |= nSRST[0] | nSRST[1]
            self._initial |= nSRST[1]

        if nTRST is not None:
            self._has_trst = True
            self._trst = nTRST

            self._dir     |= nTRST[0] | nTRST[1]
            self._initial |= nTRST[1]

        self._current = self._initial

        self._dev.open_mpsse(vid, pid, direction=self._dir, initial=self._initial, frequency=frequency)

        self._stateDefined = False
        self._currentState = -1

        self._dev.read_data_set_chunksize(512)
        self._dev.write_data_set_chunksize(512)

        self._MaxRW = min(self._dev.write_data_get_chunksize(), self._dev.read_data_get_chunksize())

        self._buffer = bytearray(self._MaxRW)
        self._buffer_size = 0

    def Close(self):
        self._dev.close()

    def _WriteGPIO(self, dir, value):
        self._AddBufferD([MPSSE_SET_DATA_LOW, value & 0xFF, dir & 0xFF])
        self._SendBuffer()
        self._dev.validate_mpsse()

        self._AddBufferD([MPSSE_SET_DATA_HIGH, (value >> 8) & 0xFF, (dir >> 8) & 0xFF])
        self._SendBuffer()
        self._dev.validate_mpsse()

    def _AddBuffer(self, byte):
        # self._EnsureSpace(1)
        self._buffer[self._buffer_size] = byte
        self._buffer_size += 1

    def _AddBufferD(self, data):
        count = len(data)
        # self._EnsureSpace(count)
        self._buffer[self._buffer_size:self._buffer_size+count] = data
        self._buffer_size += count
    
    def _AddBufferW(self, cmd: int):
        # self._EnsureSpace(2)
        struct.pack_into("<H", self._buffer, self._buffer_size, cmd)
        self._buffer_size += 2

    def _EnsureSpace(self, count: int):
        if self._buffer_size + count >= len(self._buffer):
            self._SendBuffer()

    def _SendBuffer(self):
        if self._buffer_size > 0:
            self._dev.write_data(self._buffer[0:self._buffer_size])
            self._buffer_size = 0

    def _CheckCommand(self):
        pass

    def _Read(self, size):
        reads = 0

        while True:
            res = self._dev.read_data_bytes(size)
            if len(res) >= size:
                break
            reads += 1

        if reads > 0:
            self.log.error("Multiple reads needed: %d", reads)

        self.log.debug("Read %d bytes out of %d" % (len(res), size))
        return res

    def _ClockTMSReadTDI(self, values: int, count: int, LastBit: bool) -> int:
        if count > 7:
            count = 7
        elif count <= 0:
            return 0

        self._AddBuffer(MPSSE_DO_TMS | MPSSE_DO_TDI | MPSSE_DO_TDO | MPSSE_CLK_ON_WRITE | MPSSE_LSB_FIRST | MPSSE_BITMODE)
        self._AddBuffer(count-1)
        self._AddBuffer(values | (int(LastBit) << 7))
        self._AddBuffer(MPSSE_SEND_IMMEDIATE)

        self._SendBuffer()

        return self._Read(1)[0]

    def _ClockTMS(self, value, count, lastBit = True):
        if count > 7:
            count = 7
        elif count <= 0:
            return

        self._AddBufferD([MPSSE_DO_TMS | MPSSE_CLK_ON_WRITE | MPSSE_LSB_FIRST | MPSSE_BITMODE, count-1, value | (int(lastBit) << 7)])

        # self._SendBuffer()

        # self._CheckCommand()

    def _IntSetState(self, state: JtagState, TDO: bool = False) -> bool:
        if not self._stateDefined:
            self._ClockTMS(0x7F, 7)
            self._currentState = JtagState.Reset
            self._stateDefined = True

        if self._currentState == state:
            return

        (val, x) = self._transition_table[(self._currentState, state)]

        self._currentState = state

        val = self._ClockTMSReadTDI(val, x, TDO)
        return ((val >> (8-x)) & 1) == 1

    def _IntSetState_NoRead(self, state: JtagState, TDO: bool = False) -> None:
        if not self._stateDefined:
            self._ClockTMS(0x7F, 7)
            self._currentState = JtagState.Reset
            self._stateDefined = True

        if self._currentState == state:
            return

        (val, x) = self._transition_table[(self._currentState, state)]

        self._currentState = state

        self._ClockTMS(val, x, TDO)

    def Flush(self):
        self._SendBuffer()

    def SRST(self, state: str):
        if self._srst is None:
            raise Exception("No SRST pin configured")
        
        data = self._current
        if state == '1':
            data |= self._srst[0]
            data &= ~self._srst[1]
        elif state == '0':
            data &= ~self._srst[0]
            data &= ~self._srst[1]
        elif state in ['Z', 'z']:
            data &= ~self._srst[0]
            data |= self._srst[1]
        self._current = data
        
        self._WriteGPIO(self._dir, data)

    def SetState(self, state: JtagState) -> None:
        self._IntSetState_NoRead(state)

    def Clock(self, count: int):
        self._ClockTMS(0, count)

    def _WriteBytes(self, data: typing.Union[bytes, bytearray]) -> bytearray:
        count = len(data)

        if count > self._MaxRW:
            count = self._MaxRW
        elif count <= 0:
            return bytearray()

        self._AddBuffer(MPSSE_DO_TDO | MPSSE_CLK_ON_WRITE | MPSSE_LSB_FIRST)
        self._AddBufferW(count-1)
        self._AddBufferD(data)

        # self._SendBuffer()

        # self._CheckCommand()

    def _WriteBits(self, data: int, count: int) -> int:
        if count > 8:
            count = 8
        elif count <= 0:
            return 0

        self._AddBuffer(MPSSE_DO_TDO | MPSSE_CLK_ON_WRITE | MPSSE_LSB_FIRST | MPSSE_BITMODE)
        self._AddBuffer(count-1)
        self._AddBuffer(data)

        # self._SendBuffer()

        # self._CheckCommand()

    def _ReadWriteBytes(self, data: typing.Union[bytes, bytearray]) -> bytearray:
        count = len(data)

        if count > self._MaxRW:
            count = self._MaxRW
        elif count <= 0:
            return bytearray()

        self._AddBuffer(MPSSE_DO_TDO | MPSSE_DO_TDI | MPSSE_CLK_ON_WRITE | MPSSE_LSB_FIRST)
        self._AddBufferW(count-1)
        self._AddBufferD(data)
        self._AddBuffer(MPSSE_SEND_IMMEDIATE)

        self._SendBuffer()

        return self._Read(count)

    def _ReadWriteBits(self, data: int, count: int) -> int:
        if count > 8:
            count = 8
        elif count <= 0:
            return 0

        self._AddBuffer(MPSSE_DO_TDO | MPSSE_DO_TDI | MPSSE_CLK_ON_WRITE | MPSSE_LSB_FIRST | MPSSE_BITMODE)
        self._AddBuffer(count-1)
        self._AddBuffer(data)
        self._AddBuffer(MPSSE_SEND_IMMEDIATE)

        self._SendBuffer()

        return self._Read(1)[0] >> (8-count)

    def SetSignal(self, signal: str, high: bool):
        signal_mask = {
            "TCK": 0x1,
            "TDI": 0x2,
            "TMS": 0x8,
        }

        if signal not in signal_mask:
            raise Exception("Signal not controllable")

        current = self._current
        if high:
            current |= signal_mask[signal]
        else:
            current &= ~signal_mask[signal]
        self._current = current

        self._WriteGPIO(self._dir, current)
    
    def IR(self, value: int, count: int, endState: JtagState = JtagState.Idle) -> int:
        self.SetState(JtagState.ShiftIR)

        if endState != JtagState.ShiftIR:
            count -= 1

        outdata = struct.pack("<Q", value)
        indata = bytearray(8)

        idx = 0
        if count > 8:
            nb = count // 8
            indata[0:nb] = self._ReadWriteBytes(outdata[0:nb])
            count = count - nb*8

            idx += nb

        if count > 0:
            indata[idx] = self._ReadWriteBits(outdata[idx], count)

        if endState != JtagState.ShiftIR:
            LastBit = ((outdata[idx] >> count) & 1) == 1
            LastBit = self._IntSetState(endState, LastBit)

            indata[idx] |= int(LastBit) << count
        
        return struct.unpack("<Q", indata)[0]

    def DR(self, value: int, count: int, endState: JtagState = JtagState.Idle) -> int:
        self.SetState(JtagState.ShiftDR)

        if endState != JtagState.ShiftDR:
            count -= 1

        outdata = struct.pack("<Q", value)
        indata = bytearray(8)

        idx = 0
        if count > 8:
            nb = count // 8
            indata[0:nb] = self._ReadWriteBytes(outdata[0:nb])
            count = count - nb*8
            idx += nb

        if count > 0:
            indata[idx] = self._ReadWriteBits(outdata[idx], count)

        if endState != JtagState.ShiftDR:
            LastBit = ((outdata[idx] >> count) & 1) == 1
            LastBit = self._IntSetState(endState, LastBit)

            indata[idx] |= int(LastBit) << count
        
        return struct.unpack("<Q", indata)[0]

    def DROut(self, value: int, count: int, endState: JtagState = JtagState.Idle) -> None:
        self.SetState(JtagState.ShiftDR)

        if endState != JtagState.ShiftDR:
            count -= 1

        outdata = struct.pack("<Q", value)

        idx = 0
        if count > 8:
            nb = count // 8
            self._WriteBytes(outdata[0:nb])
            count = count - nb*8
            idx += nb

        if count > 0:
            self._WriteBits(outdata[idx], count)

        if endState != JtagState.ShiftDR:
            LastBit = ((outdata[idx] >> count) & 1) == 1
            self._IntSetState_NoRead(endState, LastBit)
        
        # self._SendBuffer()

        # self._CheckCommand()

    def ScheduleDRIn(self, bytes: int):
        self._AddBuffer(MPSSE_DO_TDI | MPSSE_LSB_FIRST)
        self._AddBufferW(bytes-1)

    def ReadScheduled(self, bytes: int):
        self._AddBuffer(MPSSE_SEND_IMMEDIATE)
        self._SendBuffer()

        return self._Read(bytes)