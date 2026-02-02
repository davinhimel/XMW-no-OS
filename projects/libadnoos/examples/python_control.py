# adf4377_control.py
from ctypes import CDLL, Structure, POINTER, c_uint64, c_int32, c_uint32, c_uint16, c_uint8, c_void_p, c_bool, byref, pointer, cast
import os
from pathlib import Path

# Set library path
SCRIPT_DIR = Path(__file__).parent.absolute()
PROJECT_ROOT = SCRIPT_DIR.parent
os.environ['LD_LIBRARY_PATH'] = f"{PROJECT_ROOT}/drivers_so:{PROJECT_ROOT}/core/build"

# 1. Load libraries
lib = CDLL(f"{PROJECT_ROOT}/drivers_so/libadf4377.so")
core_lib = CDLL(f"{PROJECT_ROOT}/core/build/libadnoos.so")

# 2. Define the two structs first (needed for argtypes)
class spi_init_param(Structure):
    _fields_ = [
        ("device_id",     c_uint32),
        ("max_speed_hz",  c_uint32),
        ("chip_select",   c_uint8),
        ("mode",          c_uint32),  # enum
        ("bit_order",     c_uint32),  # enum
        ("lanes",         c_uint32),  # enum
        ("platform_ops", c_void_p),  # pointer to struct no_os_spi_ops
        ("platform_delays", c_void_p),  # can be NULL
        ("extra",         c_void_p),  # points to linux_spi_init_param
        ("parent",        c_void_p)   # can be NULL
    ]

class adf4377_init_param(Structure):
    _fields_ = [
        ("spi_init",          POINTER(spi_init_param)),
        ("gpio_ce_param",     c_void_p),  # NULL
        ("gpio_enclk1_param", c_void_p),  # NULL
        ("gpio_enclk2_param", c_void_p),  # NULL
        ("dev_id",            c_uint32),  # enum - ADF4377 = 0x05
        ("spi4wire",          c_bool),
        ("clkin_freq",        c_uint64),
        ("f_clk",             c_uint64),   # desired output frequency
        ("cp_i",              c_uint8),
        ("muxout_select",     c_uint32),
        ("ref_doubler_en",    c_uint8),
        ("clkout_op",         c_uint8),
        ("ref_div_factor",    c_uint32)
    ]

# 3. Teach ctypes the exact C signatures
# int32_t adf4377_init(struct adf4377_dev **device, struct adf4377_init_param *init_param);
# For pointer-to-pointer output: pass POINTER(c_void_p) - ctypes handles the indirection
lib.adf4377_init.argtypes = [POINTER(c_void_p), POINTER(adf4377_init_param)]
lib.adf4377_init.restype  = c_int32

# int32_t adf4377_set_freq(struct adf4377_dev *dev);
lib.adf4377_set_freq.argtypes = [c_void_p]
lib.adf4377_set_freq.restype  = c_int32

# int32_t adf4377_set_rfout(struct adf4377_dev *dev, uint64_t val);
lib.adf4377_set_rfout.argtypes = [c_void_p, c_uint64]
lib.adf4377_set_rfout.restype  = c_int32

# int32_t adf4377_remove(struct adf4377_dev *dev);
lib.adf4377_remove.argtypes = [c_void_p]
lib.adf4377_remove.restype  = c_int32

# int adf4377_spi_read(struct adf4377_dev *dev, uint8_t reg_addr, uint8_t *data);
lib.adf4377_spi_read.argtypes = [c_void_p, c_uint8, POINTER(c_uint8)]
lib.adf4377_spi_read.restype  = c_int32

# int adf4377_spi_update_bit(struct adf4377_dev *dev, uint16_t reg_addr, uint8_t mask, uint8_t data);
lib.adf4377_spi_update_bit.argtypes = [c_void_p, c_uint16, c_uint8, c_uint8]
lib.adf4377_spi_update_bit.restype  = c_int32

# ADF4377 Constants
ADF4377_REG = lambda x: x  # ADF4377_REG(x) is just (x)
ADF4377_MUXOUT_MSK = 0xF0  # Bits [7:4] mask
ADF4377_MUXOUT_HIGH = 0x8  # MUXOUT HIGH value
# ADF4377_MUXOUT(x) shifts x to bits [7:4]: (x << 4) & 0xF0
ADF4377_MUXOUT = lambda x: ((x & 0x0F) << 4) & ADF4377_MUXOUT_MSK

# 4. Get linux_spi_ops pointer from core library
# linux_spi_ops is exported from libadnoos.so
try:
    # Try to access the symbol directly
    linux_spi_ops_addr = getattr(core_lib, 'linux_spi_ops')
    if hasattr(linux_spi_ops_addr, 'value'):
        linux_spi_ops_addr = linux_spi_ops_addr.value
    elif hasattr(linux_spi_ops_addr, '_handle'):
        # It's a symbol, get its address
        linux_spi_ops_addr = cast(linux_spi_ops_addr, c_void_p).value
    else:
        linux_spi_ops_addr = cast(linux_spi_ops_addr, c_void_p).value if linux_spi_ops_addr else None
except (AttributeError, TypeError):
    # Fallback: try using dlsym via ctypes
    try:
        from ctypes import CDLL as _CDLL
        import ctypes
        # Use dlsym to get symbol address
        libdl = _CDLL("libdl.so.2")
        libdl.dlsym.restype = c_void_p
        libdl.dlsym.argtypes = [c_void_p, ctypes.c_char_p]
        handle = core_lib._handle
        linux_spi_ops_addr = libdl.dlsym(handle, b"linux_spi_ops")
        if linux_spi_ops_addr:
            print(f"✓ Got linux_spi_ops address: 0x{linux_spi_ops_addr:x}")
        else:
            print("Warning: Could not get linux_spi_ops via dlsym")
            linux_spi_ops_addr = None
    except:
        print("Warning: Could not get linux_spi_ops, driver may fail")
        linux_spi_ops_addr = None

# 5. Build the parameter structures (matching ADF4377_Test exactly)
# Create linux_spi_extra structure (matching common_data.c)
class LinuxSpiExtra(Structure):
    _fields_ = [
        ("device_id", c_uint32),
        ("chip_select", c_uint8),
        ("max_speed_hz", c_uint32),
        ("mode", c_uint8)
    ]

# Create linux_spi_extra instance (matching adf4377_spi_extra in common_data.c)
linux_spi_extra = LinuxSpiExtra(
    device_id=0,
    chip_select=0,
    max_speed_hz=10000,  # Match working config: 10 kHz (not 2 MHz!)
    mode=0  # SPI_MODE_0
)

spi_param = spi_init_param(
    device_id      = 0,          # /dev/spidev0.X
    max_speed_hz   = 10000,       # Match working: 10 kHz (was 20000)
    chip_select    = 0,
    mode           = 0,          # NO_OS_SPI_MODE_0 (CPOL=0, CPHA=0)
    bit_order      = 0,          # NO_OS_SPI_BIT_ORDER_MSB_FIRST
    lanes          = 0,          # NO_OS_SPI_SINGLE_LANE
    platform_ops   = linux_spi_ops_addr if linux_spi_ops_addr else c_void_p(0),
    platform_delays = None,
    extra          = cast(pointer(linux_spi_extra), c_void_p).value,  # Match working: point to linux_spi_extra
    parent         = None
)

chip_param = adf4377_init_param(
    spi_init         = pointer(spi_param),
    gpio_ce_param    = None,
    gpio_enclk1_param = None,
    gpio_enclk2_param = None,
    dev_id           = 0x05,     # ADF4377
    spi4wire         = True,
    clkin_freq       = 125000000,   # 125 MHz reference
    f_clk            = 10000000000, # 10 GHz output
    cp_i             = 0x0F,     # ADF4377_CP_10MA1
    muxout_select    = 0,        # ADF4377_MUXOUT_HIGH_Z
    ref_doubler_en   = 1,
    clkout_op        = 3,        # ADF4377_CLKOUT_640MV
    ref_div_factor   = 1
)

# 6. Call the real driver exactly like in C
# For pointer-to-pointer output (struct adf4377_dev **device):
# Create a variable to hold the pointer, then a pointer to that variable
dev_ptr_var = c_void_p()  # This will hold the actual device pointer
dev_ptr = pointer(dev_ptr_var)  # Pointer to the variable

print("Initializing ADF4377...")
print(f"  SPI device_id: {spi_param.device_id}")
print(f"  SPI chip_select: {spi_param.chip_select}")
print(f"  SPI max_speed_hz: {spi_param.max_speed_hz}")
print(f"  SPI mode: {spi_param.mode}")
print(f"  platform_ops: 0x{spi_param.platform_ops:x}" if spi_param.platform_ops else "  platform_ops: None/Invalid")

ret = lib.adf4377_init(dev_ptr, byref(chip_param))
print(f"  adf4377_init returned: {ret}")

if ret != 0:
    print(f"✗ Init failed, error {ret}")
    print("\nInitialization checks that may have failed:")
    print("  1. SPI initialization (no_os_spi_init)")
    print("  2. Soft reset (adf4377_soft_reset) - writes reset + default registers")
    print("  3. Chip type read (register 0x03) - should be 0x06")
    print("  4. Scratchpad check")
    print("  5. Device ID read (register 0x04) - should be 0x05")
    print("  6. adf4377_setup() - CRITICAL: Enables outputs, causes current to rise")
    print("\nCheck:")
    print("  - SPI device exists: /dev/spidev0.0")
    print("  - SPI is enabled (raspi-config)")
    print("  - You have permissions (may need sudo)")
    print("  - platform_ops pointer is valid")
    print("  - Hardware connections are correct")
    print("  - Chip is powered and responding")
    exit(1)

print("✓ ADF4377 initialized successfully")

# Get the device pointer from dev_ptr_var (which was written to by the function)
dev = dev_ptr_var
if not dev.value:
    print("  Device pointer: NULL (ERROR!)")
    print("✗ ERROR: Device pointer is NULL after initialization!")
    print("  This means adf4377_init returned success but didn't set the device pointer")
    print("  The function may have failed silently at one of these steps:")
    print("    1. SPI initialization")
    print("    2. Soft reset (writes reset + default registers)")
    print("    3. Chip type check (register 0x03 should be 0x06)")
    print("    4. Scratchpad check")
    print("    5. Device ID check (register 0x04 should be 0x05)")
    print("    6. adf4377_setup() - CRITICAL: This enables outputs and causes current to rise")
    print("\n  If current doesn't go from 50mA to 600mA, setup() likely failed!")
    exit(1)

print(f"  Device pointer: 0x{dev.value:x}")

print("\n# 7. Read current value of REG001D (MUXOUT register)")
print("Step 1: Reading REG001D (MUXOUT register)...")
read_back = c_uint8()
ret = lib.adf4377_spi_read(dev.value, ADF4377_REG(0x1D), byref(read_back))
if ret < 0:
    print(f"✗ Failed to read REG001D (error {ret})")
    if dev.value:
        lib.adf4377_remove(dev.value)
    exit(1)

current_value = read_back.value
muxout_bits = (current_value >> 4) & 0x0F
print(f"  Current REG001D value: 0x{current_value:02X}")
print(f"  MUXOUT bits [7:4]: 0x{muxout_bits:01X}\n")

# 8. Write HIGH to MUXOUT bits [7:4]
print("Step 2: Writing to REG001D to set MUXOUT bits [7:4] to HIGH (0x8)...")
muxout_value = ADF4377_MUXOUT(ADF4377_MUXOUT_HIGH)  # This sets bits [7:4] = 0x8
ret = lib.adf4377_spi_update_bit(dev.value, 
                                  ADF4377_REG(0x1D), 
                                  ADF4377_MUXOUT_MSK, 
                                  muxout_value)
if ret < 0:
    print(f"✗ Failed to write to REG001D (error {ret})")
    lib.adf4377_remove(dev.value)
    exit(1)
print(f"  ✓ Successfully wrote MUXOUT = HIGH (0x8) to bits [7:4]\n")

# 9. Read back REG001D to verify the write
print("Step 3: Verifying write by reading REG001D...")
read_back = c_uint8()
ret = lib.adf4377_spi_read(dev.value, ADF4377_REG(0x1D), byref(read_back))
if ret < 0:
    print(f"✗ Failed to read REG001D (error {ret})")
    if dev.value:
        lib.adf4377_remove(dev.value)
    exit(1)

new_value = read_back.value
new_muxout_bits = (new_value >> 4) & 0x0F
print(f"  REG001D value after write: 0x{new_value:02X}")
print(f"  MUXOUT bits [7:4]: 0x{new_muxout_bits:01X}", end="")

# Verify the MUXOUT bits are set correctly
if new_muxout_bits == ADF4377_MUXOUT_HIGH:
    print(" ✓ (HIGH - correct!)")
else:
    print(f" ✗ (Expected 0x{ADF4377_MUXOUT_HIGH:X}, got 0x{new_muxout_bits:X})")
    ret = -1

# 10. Cleanup
print("\nStep 4: Cleaning up...")
ret = lib.adf4377_remove(dev.value)
if ret < 0:
    print(f"  WARNING: Error during cleanup (error {ret})")
else:
    print("  ✓ Cleanup successful")

print("\n" + "="*50)
if ret == 0:
    print("Test completed successfully!")
    print("MUXOUT pin should now output HIGH signal")
else:
    print("Test completed with errors")
print("="*50)