#!/usr/bin/env python3
"""
Pure userspace - no compilation needed!
Just run: python3 test.py (from examples/ directory)
"""

from ctypes import CDLL, Structure, c_int, c_uint8, c_uint32, c_uint64, POINTER, byref, c_void_p, c_bool
import os
from pathlib import Path

# Get the script's directory and find project root
SCRIPT_DIR = Path(__file__).parent.absolute()
PROJECT_ROOT = SCRIPT_DIR.parent
DRIVERS_SO_DIR = PROJECT_ROOT / "drivers_so"
CORE_BUILD_DIR = PROJECT_ROOT / "core" / "build"

# Set library paths
os.environ['LD_LIBRARY_PATH'] = f"{DRIVERS_SO_DIR}:{CORE_BUILD_DIR}"

# Load the driver library
lib_path = DRIVERS_SO_DIR / "libadf4377.so"
if not lib_path.exists():
    print(f"ERROR: Library not found at {lib_path}")
    exit(1)

pll = CDLL(str(lib_path))
print(f"✓ Loaded library: {lib_path}")

# Define C structures using ctypes
class LinuxSpiInitParam(Structure):
    _fields_ = [
        ("device_id", c_uint32),
        ("chip_select", c_uint8),
        ("max_speed_hz", c_uint32),
        ("mode", c_uint8)
    ]

class NoOsSpiInitParam(Structure):
    _fields_ = [
        ("device_id", c_uint32),
        ("max_speed_hz", c_uint32),
        ("chip_select", c_uint8),
        ("mode", c_uint32),  # enum as uint32
        ("bit_order", c_uint32),  # enum as uint32
        ("lanes", c_uint32),  # enum as uint32
        ("platform_ops", c_void_p),  # pointer - will set to &linux_spi_ops
        ("platform_delays", c_void_p),  # struct - can be NULL
        ("extra", POINTER(LinuxSpiInitParam)),  # pointer to linux_spi_extra
        ("parent", c_void_p)  # can be NULL
    ]

class ADF4377InitParam(Structure):
    _fields_ = [
        ("spi_init", POINTER(NoOsSpiInitParam)),
        ("gpio_ce_param", c_void_p),  # NULL
        ("gpio_enclk1_param", c_void_p),  # NULL
        ("gpio_enclk2_param", c_void_p),  # NULL
        ("dev_id", c_uint32),  # enum as uint32 - ADF4377 = 0x05
        ("spi4wire", c_bool),
        ("clkin_freq", c_uint64),
        ("f_clk", c_uint64),
        ("cp_i", c_uint8),
        ("muxout_select", c_uint32),
        ("ref_doubler_en", c_uint8),
        ("clkout_op", c_uint8),
        ("ref_div_factor", c_uint32)
    ]

# Setup Linux SPI parameters
linux_spi_extra = LinuxSpiInitParam(
    device_id=0,        # /dev/spidev0.0
    chip_select=0,
    max_speed_hz=2000000,  # 2 MHz
    mode=0  # SPI_MODE_0
)

# Setup no-OS SPI parameters
# Note: platform_ops needs to point to linux_spi_ops from libadnoos.so
# We'll need to get that symbol from the core library
core_lib = CDLL(str(CORE_BUILD_DIR / "libadnoos.so"))
linux_spi_ops_ptr = core_lib.linux_spi_ops  # This might not work directly

spi_init_param = NoOsSpiInitParam(
    device_id=0,
    max_speed_hz=2000000,
    chip_select=0,
    mode=0,  # NO_OS_SPI_MODE_0
    bit_order=0,  # NO_OS_SPI_BIT_ORDER_MSB_FIRST
    lanes=0,  # NO_OS_SPI_SINGLE_LANE
    platform_ops=linux_spi_ops_ptr,  # This is tricky - see note below
    platform_delays=None,
    extra=byref(linux_spi_extra),
    parent=None
)

# Setup ADF4377 init parameters
init_param = ADF4377InitParam(
    spi_init=byref(spi_init_param),
    gpio_ce_param=None,
    gpio_enclk1_param=None,
    gpio_enclk2_param=None,
    dev_id=0x05,  # ADF4377
    spi4wire=True,
    clkin_freq=125000000,  # 125 MHz
    f_clk=11000000000,  # 11 GHz
    cp_i=0x0F,  # ADF4377_CP_10MA1
    muxout_select=0,  # ADF4377_MUXOUT_HIGH_Z
    ref_doubler_en=1,
    clkout_op=3,  # ADF4377_CLKOUT_640MV
    ref_div_factor=1
)

# Define function signatures
pll.adf4377_init.argtypes = [POINTER(c_void_p), POINTER(ADF4377InitParam)]
pll.adf4377_init.restype = c_int

pll.adf4377_set_rfout.argtypes = [c_void_p, c_uint64]
pll.adf4377_set_rfout.restype = c_int

pll.adf4377_remove.argtypes = [c_void_p]
pll.adf4377_remove.restype = c_int

# Call functions
dev = c_void_p()
print("Initializing ADF4377...")
ret = pll.adf4377_init(byref(dev), byref(init_param))
if ret < 0:
    print(f"ERROR: Init failed with code {ret}")
    exit(1)

print("✓ ADF4377 initialized")
print("Setting frequency to 11 GHz...")
ret = pll.adf4377_set_rfout(dev, 11000000000)
if ret < 0:
    print(f"ERROR: Set frequency failed with code {ret}")
else:
    print("✓ Frequency set")

print("Cleaning up...")
pll.adf4377_remove(dev)
print("✓ Done")