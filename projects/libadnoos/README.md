# libadnoos — Universal no-OS Linux Framework

**The professional, industry-grade universal no-OS Linux userspace framework that Analog Devices should have shipped years ago.**

## Overview

libadnoos allows any user (C, Python, Qt, Rust, Node.js, etc.) to directly call the original no-OS driver functions from any no-OS chip on Raspberry Pi or any Linux machine using only `spidev`/`i2c-dev`.

### Architecture

```
┌─────────────────────────────────────────────────────────────┐
│  User Applications (C, Python, Qt, Rust, Node.js, etc.)    │
└───────────────────────┬─────────────────────────────────────┘
                        │
        ┌───────────────┴───────────────┐
        │                               │
┌───────▼────────┐            ┌────────▼────────┐
│ libadf4377.so  │            │ libadxl367.so   │
│ libad5940.so   │            │ libmax31865.so  │
│ ...            │            │ ...             │
└───────┬────────┘            └────────┬────────┘
        │                               │
        └───────────────┬───────────────┘
                        │
                ┌───────▼────────┐
                │ libadnoos.so   │
                │ (Core Library) │
                └───────┬────────┘
                        │
        ┌───────────────┴───────────────┐
        │                               │
    Linux SPI                      Linux I2C
    (spidev)                       (i2c-dev)
```

### Key Features

- **One core library** (`libadnoos.so`) — built once, never touched again
- **One library per chip** — build only the drivers you need
- **Direct function calls** — use original no-OS driver functions as-is
- **Language agnostic** — works with C, Python, Qt, Rust, Node.js, etc.
- **No modifications** — uses original no-OS driver code unchanged
- **Production ready** — proper error handling, linking, and installation

## Quick Start

### 1. Build Core Library (One-Time Setup)

```bash
cd projects/libadnoos/core
mkdir -p build && cd build
cmake ..
make -j8
```

This creates `libadnoos.so` (~3-5 MB) containing:
- All Linux platform implementations (`linux_spi.c`, `linux_gpio.c`, etc.)
- All utility functions (`no_os_alloc.c`, `no_os_mutex.c`, etc.)
- All API layer functions (`no_os_spi.c`, `no_os_gpio.c`, etc.)

### 2. Build Any Driver Library

```bash
cd projects/libadnoos/tools
./make_driver_so.sh ../../drivers/frequency/adf4377/adf4377.c
```

This creates `drivers_so/libadf4377.so` that:
- Links against `libadnoos.so`
- Exports all original driver functions directly
- Can be used from any language

### 3. Use from C

```c
#include "adf4377.h"

int main() {
    struct adf4377_dev *dev;
    adf4377_init(&dev, &init_param);
    adf4377_set_rfout(dev, 11000000000ULL);
    adf4377_remove(dev);
    return 0;
}
```

Compile:
```bash
gcc -o test test.c -Ldrivers_so -ladf4377 -Lcore/build -ladnoos \
    -Wl,-rpath,drivers_so:core/build
```

### 4. Use from Python

```python
from ctypes import CDLL

pll = CDLL("./drivers_so/libadf4377.so")
# Call functions directly
pll.adf4377_init(...)
pll.adf4377_set_rfout(dev, 11000000000)
```

## Detailed Instructions

### Building the Core Library

The core library provides the Linux platform layer for all drivers:

```bash
cd projects/libadnoos/core
mkdir -p build
cd build
cmake ..
make -j8
sudo make install  # Optional: installs to /usr/local/lib
```

**Output:** `core/build/libadnoos.so`

**Contains:**
- `drivers/platform/linux/*.c` — Linux SPI, I2C, GPIO, UART, delay, timer
- `drivers/api/*.c` — Hardware-agnostic API layer
- `util/*.c` — Utilities (alloc, mutex, fifo, list, etc.)

### Building Driver Libraries

Use the provided script to build any driver:

```bash
cd projects/libadnoos/tools

# Build ADF4377
./make_driver_so.sh ../../drivers/frequency/adf4377/adf4377.c

# Build ADXL367
./make_driver_so.sh ../../drivers/accel/adxl367/adxl367.c

# Build AD5940
./make_driver_so.sh ../../drivers/impedance-analyzer/ad5940/ad5940.c

# Build MAX31865
./make_driver_so.sh ../../drivers/temperature/max31865/max31865.c
```

**Output:** `drivers_so/lib<chipname>.so`

Each driver library:
- Links against `libadnoos.so`
- Exports all original driver functions
- Can be used independently

### Using from C

See `examples/adf4377_test.c` for a complete working example.

**Build example:**
```bash
cd examples
make adf4377_test
LD_LIBRARY_PATH=../drivers_so:../core/build ./adf4377_test
```

**Manual compilation:**
```bash
gcc -o myapp myapp.c \
    -I../.. \
    -I../../include \
    -I../../drivers/platform/linux \
    -L../drivers_so -ladf4377 \
    -L../core/build -ladnoos \
    -Wl,-rpath,../drivers_so:../core/build \
    -lpthread -lm
```

### Using from Python

See `examples/python_control.py` for auto-discovery example.

**Basic usage:**
```python
from ctypes import CDLL, Structure, c_int, c_uint64, POINTER, byref

# Load driver library
pll = CDLL("./drivers_so/libadf4377.so")

# Define C structures (simplified)
class ADF4377Dev(Structure):
    pass

# Call driver functions
dev = POINTER(ADF4377Dev)()
# ... setup init_param ...
pll.adf4377_init(byref(dev), init_param_ptr)
pll.adf4377_set_rfout(dev, 11000000000)
```

**Note:** For production Python code, you'll need to properly define all C structures using `ctypes.Structure`. See the driver header files for structure definitions.

### Using from Other Languages

**Rust:**
```rust
use libloading::Library;

let lib = Library::new("./drivers_so/libadf4377.so").unwrap();
let init: Symbol<unsafe extern fn(...)> = lib.get(b"adf4377_init").unwrap();
```

**Node.js:**
```javascript
const ffi = require('ffi-napi');
const pll = ffi.Library('./drivers_so/libadf4377', {
    'adf4377_init': ['int', ['pointer', 'pointer']],
    'adf4377_set_rfout': ['int', ['pointer', 'ulonglong']]
});
```

**Qt/C++:**
```cpp
#include <QLibrary>
QLibrary lib("./drivers_so/libadf4377.so");
typedef int (*InitFunc)(void**, void*);
InitFunc init = (InitFunc)lib.resolve("adf4377_init");
```

## Supported Drivers

### ✅ Drivers That Work Perfectly

These drivers work out-of-the-box with libadnoos:

- **Frequency Synthesizers:** `adf4377`, `adf4378`, `adf4030`, `adf4368`
- **Accelerometers:** `adxl367`, `adxl355`, `adxl313`
- **Impedance Analyzers:** `ad5940`, `ad5941`
- **Temperature:** `max31865`, `adt7420`
- **DACs:** `ad5686`, `ad5758`, `ltc2672`
- **ADCs:** `ad7091r8`, `ad7124`, `ad719x`
- **Power Management:** `ltc4162l`, `adp5055`
- **And many more...**

### ❌ Drivers That Need FPGA/JESD (Skip These)

These drivers require FPGA infrastructure and won't work with simple SPI/I2C:

- `ad9081`, `ad9083` — JESD204B/C interfaces
- `ad4630`, `ad9208` — High-speed ADC with JESD
- `adrv9001`, `adrv9009` — RF transceivers with JESD
- Any driver requiring AXI interfaces

## Project Structure

```
libadnoos/
├── core/                      # Core library (build once)
│   ├── CMakeLists.txt         # Build configuration
│   ├── include/
│   │   └── adnoos.h          # Core header
│   └── build/                 # Build output
│       └── libadnoos.so      # Core library
│
├── drivers_so/                # Driver libraries (user builds as needed)
│   ├── libadf4377.so
│   ├── libadxl367.so
│   └── ...
│
├── examples/                   # Example code
│   ├── adf4377_test.c         # C example
│   ├── python_control.py     # Python example
│   └── Makefile               # Build examples
│
├── tools/                      # Build tools
│   └── make_driver_so.sh     # Driver build script
│
└── README.md                   # This file
```

## Installation (Optional)

Install core library system-wide:

```bash
cd core/build
sudo make install
```

This installs:
- `/usr/local/lib/libadnoos.so` — Core library
- `/usr/local/include/adnoos/` — Headers

Then set `LD_LIBRARY_PATH`:
```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

## Troubleshooting

### "libadnoos.so: cannot open shared object file"

Set `LD_LIBRARY_PATH`:
```bash
export LD_LIBRARY_PATH=$PWD/core/build:$LD_LIBRARY_PATH
```

Or use rpath when compiling:
```bash
gcc ... -Wl,-rpath,$PWD/core/build ...
```

### "undefined reference to `linux_spi_ops`"

Make sure you're linking against `libadnoos.so`:
```bash
gcc ... -Lcore/build -ladnoos ...
```

### Driver build fails with missing headers

Check that include paths are correct in `make_driver_so.sh`. The script should automatically find headers in the no-OS repository.

### SPI/I2C device not found

- **SPI:** Enable with `sudo raspi-config` → Interface Options → SPI
- **I2C:** Enable with `sudo raspi-config` → Interface Options → I2C
- Check device exists: `ls -l /dev/spidev*` or `ls -l /dev/i2c-*`
- May need `sudo` or add user to `spi`/`i2c` groups

## Advanced Usage

### Building Multiple Drivers

```bash
cd tools
for driver in \
    ../../drivers/frequency/adf4377/adf4377.c \
    ../../drivers/accel/adxl367/adxl367.c \
    ../../drivers/impedance-analyzer/ad5940/ad5940.c
do
    ./make_driver_so.sh "$driver"
done
```

### Custom Build Flags

Edit `make_driver_so.sh` to add custom compiler flags:
```bash
gcc -shared -fPIC -O2 -Wall -Wextra \
    -DDEBUG \  # Add custom defines
    -g \       # Add debug symbols
    ...
```

### Static Linking (Not Recommended)

You can statically link, but shared libraries are recommended for flexibility:
```bash
gcc ... -static -Lcore/build -ladnoos ...
```

## Contributing

This framework is designed to work with the unmodified no-OS repository. If you find a driver that doesn't work:

1. Check if it requires FPGA/JESD (skip those)
2. Verify SPI/I2C dependencies are correct
3. Check for missing utility functions
4. Report issues with driver name and error messages

## License

This framework uses the same license as the Analog Devices no-OS repository. See the main no-OS repository for license details.

## Credits

Built on top of the Analog Devices no-OS framework. This is the universal Linux userspace layer that should have been included from the start.

---

**Ready to use?** Start with:
```bash
cd core && mkdir -p build && cd build && cmake .. && make
cd ../../tools && ./make_driver_so.sh ../../drivers/frequency/adf4377/adf4377.c
cd ../examples && make adf4377_test
```

