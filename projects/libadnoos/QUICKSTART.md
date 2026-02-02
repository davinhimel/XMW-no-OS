# libadnoos Quick Start Guide

## 5-Minute Setup

### Step 1: Build Core Library

```bash
cd projects/libadnoos/core
mkdir -p build && cd build
cmake ..
make -j8
```

**Expected output:** `libadnoos.so` in `core/build/`

### Step 2: Build Your First Driver

```bash
cd ../../tools
./make_driver_so.sh ../../drivers/frequency/adf4377/adf4377.c
```

**Expected output:** `libadf4377.so` in `drivers_so/`

### Step 3: Test It

```bash
cd ../examples
make adf4377_test
LD_LIBRARY_PATH=../drivers_so:../core/build ./adf4377_test
```

## That's It!

You now have:
- ✅ Core library (`libadnoos.so`) — provides Linux platform layer
- ✅ Driver library (`libadf4377.so`) — exports all ADF4377 functions
- ✅ Working example — proves it works

## Next Steps

1. **Build more drivers:**
   ```bash
   cd tools
   ./make_driver_so.sh ../../drivers/accel/adxl367/adxl367.c
   ./make_driver_so.sh ../../drivers/impedance-analyzer/ad5940/ad5940.c
   ```

2. **Use from Python:**
   ```python
   from ctypes import CDLL
   pll = CDLL("./drivers_so/libadf4377.so")
   # Call functions directly
   ```

3. **Use from your own C code:**
   ```c
   #include "adf4377.h"
   // Use driver functions as normal
   ```

## Troubleshooting

**"libadnoos.so not found"**
- Make sure you built the core library first (Step 1)

**"libadf4377.so not found"**
- Make sure you built the driver (Step 2)

**"Permission denied" on SPI/I2C**
- Run with `sudo` or add user to `spi`/`i2c` groups

See `README.md` for full documentation.

