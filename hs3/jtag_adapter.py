# Copyright (C) 2012-2022 Jeppe Johansen <jeppe@j-software.dk>
import abc
from enum import IntEnum
import math
import struct
import typing


class JtagState(IntEnum):
    Unknown = -1,

    Reset = 0,
    Idle = 1,
    SelectDR = 2,
    CaptureDR = 3,
    ShiftDR = 4,
    Exit1DR = 5,
    PauseDR = 6,
    Exit2DR = 7,
    UpdateDR = 8,
    
    SelectIR = 9,
    CaptureIR = 10,
    ShiftIR = 11,
    Exit1IR = 12,
    PauseIR = 13,
    Exit2IR = 14,
    UpdateIR = 15,

NextState = {
    JtagState.Reset: {False: JtagState.Idle, True: JtagState.Reset},
    JtagState.Idle: {False: JtagState.Idle, True: JtagState.SelectDR},
    JtagState.SelectDR: {False: JtagState.CaptureDR, True:  JtagState.SelectIR},
    JtagState.CaptureDR: {False: JtagState.ShiftDR, True: JtagState.Exit1DR},
    JtagState.ShiftDR: {False: JtagState.ShiftDR, True: JtagState.Exit1DR},
    JtagState.Exit1DR: {False: JtagState.PauseDR, True: JtagState.UpdateDR},
    JtagState.PauseDR: {False: JtagState.PauseDR, True: JtagState.Exit2DR},
    JtagState.Exit2DR: {False: JtagState.ShiftDR, True: JtagState.UpdateDR},
    JtagState.UpdateDR: {False: JtagState.Idle, True: JtagState.SelectDR},
    JtagState.SelectIR: {False: JtagState.CaptureIR, True:  JtagState.Reset},
    JtagState.CaptureIR: {False: JtagState.ShiftIR, True: JtagState.Exit1IR},
    JtagState.ShiftIR: {False: JtagState.ShiftIR, True: JtagState.Exit1IR},
    JtagState.Exit1IR: {False: JtagState.PauseIR, True: JtagState.UpdateIR},
    JtagState.PauseIR: {False: JtagState.PauseIR, True: JtagState.Exit2IR},
    JtagState.Exit2IR: {False: JtagState.ShiftIR, True: JtagState.UpdateIR},
    JtagState.UpdateIR: {False: JtagState.Idle, True: JtagState.SelectDR}
}

NavigateState = {
    JtagState.Reset:      [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    JtagState.Idle:       [1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
    JtagState.SelectDR:   [1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1],
    JtagState.CaptureDR:  [1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
    JtagState.ShiftDR:    [1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
    JtagState.Exit1DR:    [1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1],
    JtagState.PauseDR:    [1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1],
    JtagState.Exit2DR:    [1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1],
    JtagState.UpdateDR:   [1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
    JtagState.SelectIR:   [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    JtagState.CaptureIR:  [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1],
    JtagState.ShiftIR:    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1],
    JtagState.Exit1IR:    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1],
    JtagState.PauseIR:    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1],
    JtagState.Exit2IR:    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1],
    JtagState.UpdateIR:   [1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
}


INVALIDMANUFACTURER = 0xFE
MANUFACTURERMASK = 0xFFE
PARTMASK = 0xFFFF000
VERSIONMASK = 0xF0000000


def _Bits(len: int) -> int:
   return (1 << len)-1


class JtagDevice:
    def __init__(self, idcode, irLength):
        self._idcode = idcode
        self._irLength = irLength

    @property
    def IDCode(self) -> int:
        return self._idcode

    @property
    def IRLength(self) -> int:
        return self._irLength


class JtagAdapter(abc.ABC):
    def __init__(self):
        self._IRLength = 0
        self._devices = []  # type: typing.List[JtagDevice]

        self._headerDR = 0
        self._trailerDR = 0
        self._headerIR = 0
        self._trailerIR = 0

    def Close(self):
        pass

    def DeviceIndex(self, idCode):
        return map(lambda dev: dev.IDCode == idCode, self._devices).index(True)

    def SoftReset(self):
        self.SetState(JtagState.Reset)
        self.SetState(JtagState.Idle)

    @abc.abstractmethod
    def SetSignal(self, signal: str, high: bool):
        raise NotImplementedError()

    @abc.abstractmethod
    def SRST(self, state: str):
        """
        state can be '0', '1', or 'z'.
        """
        raise NotImplementedError()

    @abc.abstractmethod
    def SetState(self, state: JtagState) -> None:
        raise NotImplementedError()

    @abc.abstractmethod
    def Flush(self) -> None:
        raise NotImplementedError()

    @abc.abstractmethod
    def Clock(self, count: int):
        raise NotImplementedError()

    def DetectDevices(self):
        self.SoftReset()
        self._IRLength = 0
        _devices = []
        if self.IR(1,1,JtagState.ShiftIR) == 0:
            self.SoftReset()
            return

        self.SoftReset()

        tmp = self.DR(0xFFFFFFFF, 32)
        self.SoftReset()

        while (True):
            tmp = self.DR(1, 1, JtagState.ShiftDR)
            if tmp == 1:
                tmp = tmp | (self.DR(INVALIDMANUFACTURER >> 1, 31, JtagState.ShiftDR) << 1)

                if tmp == 0xFFFFFFFF:
                    break # Highly unlikely

                if (tmp & MANUFACTURERMASK) == INVALIDMANUFACTURER:
                    break

                _devices.append({"IRLength": 0, "IDCode": tmp})
            else:
                _devices.append({"IRLength": 0, "IDCode": 0})
            if len(_devices) > 15:
                break

        self.SetState(JtagState.Reset)

        # Get IR length
        self.IR(1,1,JtagState.ShiftIR)

        for i in range(len(_devices)):
            _devices[i]["IRLength"] = 0

            while True:
                _devices[i]["IRLength"] += 1
                self._IRLength += 1
                if self.IR(1, 1, JtagState.ShiftIR) == 1:
                    break

        self.SetState(JtagState.Reset)
        self.SetState(JtagState.Idle)

        self._devices = [JtagDevice(o["IDCode"], o["IRLength"]) for o in _devices]

    def SelectDevice(self, index: int):
        for i in range(self.IRLength):
            self.IR(1, 1, JtagState.ShiftIR)

            self.DR(~(1 << index), len(self._devices))

            self._headerDR = index
            self._trailerDR = len(self._devices)-index-1

            self._headerIR = 0
            self._trailerIR = 0
            for i in range(len(self._devices)):
                if index > i:
                    self._headerIR += self._devices[i]["IRLength"]
                elif index < i:
                    self._trailerIR += self._devices[i]["IRLength"]

    @abc.abstractmethod
    def IR(self, value: int, count: int, endState: JtagState = JtagState.Idle) -> int:
        raise NotImplementedError()

    @abc.abstractmethod
    def DR(self, value: int, count: int, endState: JtagState = JtagState.Idle) -> int:
        raise NotImplementedError()

    def DROut(self, value: int, count: int, endState: JtagState = JtagState.Idle) -> None:
        self.DR(value, count, endState)

    @abc.abstractmethod
    def ScheduleDRIn(self, bytes: int):
        raise NotImplementedError()

    @abc.abstractmethod
    def ReadScheduled(self, bytes: int):
        raise NotImplementedError()

    def ScanIR(self, inValue: bytes, count: int, endState: JtagState) -> bytes:
        output = bytes((count+7) // 8)
        idx = 0

        while count > 64:
            inp = struct.unpack_from("<Q", inValue, idx)[0]
            val = self.IR(inp, 64, JtagState.ShiftIR)
            struct.pack_into("<Q", output, idx, val)

            count -= 64
            idx += 8

        while count > 0:
            inp = struct.unpack_from("<B", inValue, idx)[0]
            val = self.IR(inp, math.min(8, count), JtagState.ShiftIR)
            struct.pack_into("<B", output, idx, val)

            count -= 8
            idx += 1

        if endState != JtagState.ShiftIR:
            self.SetState(endState)
        
        return output

    def ScanDR(self, inValue: bytes, count: int, endState: JtagState = JtagState.Idle) -> bytes:
        output = bytes((count+7) // 8)
        idx = 0

        while count > 64:
            inp = struct.unpack_from("<Q", inValue, idx)[0]
            val = self.DR(inp, 64, JtagState.ShiftDR)
            struct.pack_into("<Q", output, idx, val)

            count -= 64
            idx += 8

        while count > 0:
            inp = struct.unpack_from("<B", inValue, idx)[0]
            val = self.DR(inp, math.min(8, count), JtagState.ShiftDR)
            struct.pack_into("<B", output, idx, val)

            count -= 8
            idx += 1

        if endState != JtagState.ShiftIR:
            self.SetState(endState)
        
        return output

    def ChainIR(self, AValue: int, ACount: int, endState: JtagState = JtagState.Idle) -> int:
        self.IR(_Bits(self._headerIR), self._headerIR, JtagState.ShiftIR)
        result = self.IR(AValue, ACount, JtagState.ShiftIR)
        self.IR(_Bits(self._trailerIR), self._trailerIR, endState)
        return result

    def ChainDR(self, AValue: int, ACount: int, endState: JtagState = JtagState.Idle) -> int:
        self.DR(_Bits(self._headerDR), self._headerDR, JtagState.ShiftDR)
        result = self.DR(AValue, ACount, JtagState.ShiftDR)
        self.DR(_Bits(self._trailerDR), self._trailerDR, endState)
        return result

    @property
    def IRLength(self) -> int:
        return self._IRLength
    
    @property
    def Devices(self) -> typing.Sequence[JtagDevice]:
        return list(self._devices)