#!/usr/bin/env python3
import ctypes
from pathlib import Path

LIB_PATH = Path(__file__).parent / "libadf4377_test.so"

lib = ctypes.CDLL(str(LIB_PATH))

lib.adf4377_test_init.argtypes = [ctypes.c_int, ctypes.c_int]
lib.adf4377_test_init.restype = ctypes.c_int

lib.adf4377_test_read_reg.argtypes = [ctypes.c_uint16, ctypes.POINTER(ctypes.c_uint8)]
lib.adf4377_test_read_reg.restype = ctypes.c_int

lib.adf4377_test_write_reg.argtypes = [ctypes.c_uint16, ctypes.c_uint8]
lib.adf4377_test_write_reg.restype = ctypes.c_int

lib.adf4377_test_update_bits.argtypes = [ctypes.c_uint16, ctypes.c_uint8, ctypes.c_uint8]
lib.adf4377_test_update_bits.restype = ctypes.c_int

lib.adf4377_test_set_rfout.argtypes = [ctypes.c_uint64]
lib.adf4377_test_set_rfout.restype = ctypes.c_int

lib.adf4377_test_remove.argtypes = []
lib.adf4377_test_remove.restype = ctypes.c_int


def main():
    ret = lib.adf4377_test_init(0, 0)
    if ret != 0:
        print(f"init failed: {ret}")
        return

    reg = ctypes.c_uint8()
    ret = lib.adf4377_test_read_reg(0x1D, ctypes.byref(reg))
    print(f"read 0x1D ret={ret} val=0x{reg.value:02X}")

    ret = lib.adf4377_test_update_bits(0x1D, 0xF0, 0x80)  # set MUXOUT[7:4] = 0x8
    print(f"update bits ret={ret}")

    ret = lib.adf4377_test_read_reg(0x1D, ctypes.byref(reg))
    print(f"read 0x1D ret={ret} val=0x{reg.value:02X}")

    ret = lib.adf4377_test_set_rfout(10000000000)
    print(f"set rfout ret={ret}")

    ret = lib.adf4377_test_remove()
    print(f"remove ret={ret}")


if __name__ == "__main__":
    main()
