# ADF4377 CFFI Wrapper

A clean Python wrapper for the ADF4377 driver using CFFI (C Foreign Function Interface).

## Installation

```bash
pip install cffi
# or
pip install -r requirements.txt
```

## Usage

### Simple Example

```python
from adf4377_cffi import ADF4377

# Create and initialize device
chip = ADF4377(
    device_id=0,        # /dev/spidev0.0
    chip_select=0,      # CS0
    max_speed_hz=10000, # 10 kHz
    f_clk=10000000000   # 10 GHz output
)

chip.init()

# Read a register
value = chip.read_register(0x1D)
print(f"Register 0x1D: 0x{value:02X}")

# Write to MUXOUT
chip.set_muxout_high()

# Set frequency
chip.set_rfout(11500000000)  # 11.5 GHz

# Cleanup
chip.remove()
```

### Context Manager (Recommended)

```python
with ADF4377(device_id=0, chip_select=0) as chip:
    # Device is automatically initialized
    chip.set_muxout_high()
    chip.set_rfout(11000000000)  # 11 GHz
    # Device is automatically cleaned up on exit
```

### Full Example (Matching main.c)

```python
from adf4377_cffi import ADF4377

chip = ADF4377(
    device_id=0,
    chip_select=0,
    max_speed_hz=10000,
    clkin_freq=125000000,
    f_clk=10000000000,
    ref_doubler_en=1,
    ref_div_factor=1,
    cp_i=0x0F,
    clkout_op=3,
    muxout_select=0
)

try:
    chip.init()
    
    # Read REG001D
    reg_value = chip.read_register(0x1D)
    print(f"Current REG001D: 0x{reg_value:02X}")
    
    # Write HIGH to MUXOUT
    chip.set_muxout_high()
    
    # Verify
    reg_value = chip.read_register(0x1D)
    muxout_bits = (reg_value >> 4) & 0x0F
    print(f"MUXOUT bits [7:4]: 0x{muxout_bits:01X}")
    
finally:
    chip.remove()
```

## Advantages Over ctypes

1. **No manual structure definitions** - CFFI parses C declarations
2. **Automatic pointer handling** - No `byref()`/`pointer()` complexity
3. **Better error messages** - Clearer debugging
4. **Type safety** - CFFI validates types
5. **Cleaner code** - Pythonic interface

## API Reference

### ADF4377 Class

#### `__init__(device_id, chip_select, max_speed_hz, ...)`
Initialize ADF4377 configuration (doesn't initialize hardware yet).

#### `init()`
Initialize the hardware device. Must be called before other operations.

#### `remove()`
Clean up and release resources.

#### `read_register(reg_addr)`
Read a register value. Returns `uint8_t`.

#### `write_register(reg_addr, data)`
Write a register value.

#### `update_register_bits(reg_addr, mask, data)`
Update specific bits in a register using mask.

#### `set_muxout_high()`
Set MUXOUT bits [7:4] to HIGH (0x8).

#### `set_rfout(frequency_hz)`
Set RF output frequency in Hz.

#### `get_rfout()`
Get current RF output frequency in Hz.

## Requirements

- Python 3.6+
- cffi package
- libadf4377.so (built with `make_driver_so.sh`)
- libadnoos.so (built with CMake)

## Building

Make sure you've built the core library and driver:

```bash
# Build core library
cd projects/libadnoos/core
mkdir build && cd build
cmake .. && make

# Build driver library
cd ../../tools
./make_driver_so.sh ../../drivers/frequency/adf4377/adf4377.c
```

Then use the CFFI wrapper:

```bash
cd examples
python3 adf4377_cffi.py
```

