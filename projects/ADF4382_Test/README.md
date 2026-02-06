# ADF4382 Test Project for Raspberry Pi

This project demonstrates how to use the Analog Devices no-OS framework to control an ADF4382 frequency synthesizer chip on a Raspberry Pi using Linux spidev for SPI communication.

## Overview

The program:
1. Initializes SPI communication via Linux spidev (`/dev/spidev0.0`)
2. Initializes the ADF4382 synthesizer chip
3. Reads and writes to REG000A (scratchpad register) to verify SPI communication
4. Reads the current output frequency and reference clock configuration

## Hardware Requirements

- Raspberry Pi (any model with SPI support)
- ADF4382 evaluation board or chip
- SPI connections:
  - MOSI (Master Out Slave In)
  - MISO (Master In Slave Out)
  - SCLK (Serial Clock)
  - CS (Chip Select)
  - GND (Ground)
  - Power supply for ADF4382

## Software Requirements

- Raspberry Pi OS (or any Linux distribution with SPI support)
- SPI enabled in kernel (via `raspi-config` or device tree)
- GCC compiler
- Make

## Setup Instructions

### 1. Enable SPI on Raspberry Pi

```bash
sudo raspi-config
# Navigate to: Interface Options -> SPI -> Enable
# Reboot after enabling
```

### 2. Verify SPI Device

After reboot, verify that the SPI device exists:
```bash
ls -l /dev/spidev*
# Should show /dev/spidev0.0 and /dev/spidev0.1
```

### 3. Build the Project

From the project directory:
```bash
cd /home/rpidev1/no-OS/no-OS/projects/ADF4382_Test
make PLATFORM=linux
```

This will create an executable named `adf4382_test` in the `build` directory.

### 4. Run the Program

**Note:** You may need to run with `sudo` or add your user to the `spi` group:
```bash
# Option 1: Run with sudo
sudo ./build/adf4382_test

# Option 2: Add user to spi group (one-time setup)
sudo usermod -a -G spi $USER
# Log out and log back in, then run:
./build/adf4382_test
```

### 5. Command Line Options

```bash
./build/adf4382_test [options]

Options:
  --spi-device <id>  SPI device ID (default: 0 for /dev/spidev0.X)
  --spi-cs <cs>      SPI chip select (default: 0)
  --help, -h         Show help message

Examples:
  ./build/adf4382_test
  ./build/adf4382_test --spi-device 0 --spi-cs 0
```

## Project Structure

```
ADF4382_Test/
├── Makefile              # Main build file
├── src.mk                 # Source file definitions
├── README.md             # This file
└── src/
    ├── common/
    │   ├── common_data.h  # Common data structures
    │   └── common_data.c  # Common data initialization
    └── platform/
        └── linux/
            ├── parameters.h  # Platform-specific parameters
            └── main.c       # Main program entry point
```

## Code Explanation

### SPI Initialization

The SPI is initialized using the Linux spidev interface:
- Device: `/dev/spidev0.0` (SPI bus 0, chip select 0)
- Speed: 1.5 MHz
- Mode: SPI Mode 0 (CPOL=0, CPHA=0)
- Bit order: MSB first

### ADF4382 Initialization

The ADF4382 is initialized with:
- Device ID: ADF4382
- SPI: 4-wire mode (MOSI, MISO, SCLK, CS)
- CMOS 3.3V logic levels
- Reference clock: 125 MHz
- Reference doubler: Enabled
- Reference divider: 1
- Output frequency: 20 GHz
- Charge pump current: 11.1 mA (register value 15)
- Bleed word: 4903 (default)

### REG000A Scratchpad Test

The program performs a read-write-read test on register 0x0A (scratchpad register):
1. Reads the current register value
2. Writes a test value (0xAA)
3. Reads back the value to verify the write

This verifies that SPI communication is working correctly.

## Expected Output

When the program runs successfully, you should see:

```
========================================
ADF4382 Test Program for Raspberry Pi
========================================

SPI Configuration:
  Device: /dev/spidev0.0
  Speed: 1500000 Hz
  Mode: 0 (CPOL=0, CPHA=0)

Step 1: Initializing ADF4382...
  ✓ ADF4382 initialized successfully

Step 2: Reading REG000A (scratchpad register)...
  Current REG000A value: 0xXX

Step 3: Writing test value 0xAA to REG000A (scratchpad register)...
  ✓ Successfully wrote 0xAA to REG000A

Step 4: Verifying write by reading REG000A...
  REG000A value after write: 0xAA ✓ (correct!)

Step 5: Reading current output frequency configuration...
  Current output frequency: 20000000000 Hz (20.000 GHz)

Step 6: Reading reference clock configuration...
  Reference clock: 125000000 Hz (125.000 MHz)

Step 7: Cleaning up...
  ✓ Cleanup successful

========================================
Test completed successfully!
ADF4382 communication verified
========================================
```

## Troubleshooting

### SPI Device Not Found
- Ensure SPI is enabled: `sudo raspi-config` -> Interface Options -> SPI
- Reboot after enabling SPI
- Check device exists: `ls -l /dev/spidev*`

### Permission Denied
- Run with `sudo`: `sudo ./build/adf4382_test`
- Or add user to spi group: `sudo usermod -a -G spi $USER` (then logout/login)

### Initialization Failed
- Check hardware connections (MOSI, MISO, SCLK, CS, GND)
- Verify ADF4382 power supply
- Check SPI speed (may need to reduce if too fast)
- Verify chip select pin is correct

### No Response from Chip
- Verify SPI connections
- Check power supply to ADF4382
- Try reducing SPI speed in `parameters.h`
- Verify chip select is connected correctly

## Differences from ADF4377

The ADF4382 has some differences from the ADF4377:

1. **Initialization Parameters:**
   - ADF4382 uses `spi_3wire_en` (boolean) instead of `spi4wire`
   - ADF4382 has `cmos_3v3` parameter for logic level selection
   - ADF4382 has additional parameters: `ld_count`, `en_lut_gen`, `en_lut_cal`, `max_lpf_cap_value_uf`

2. **Register Structure:**
   - ADF4382 has a different register map
   - ADF4382 uses 16-bit register addresses (vs 8-bit for ADF4377 in some cases)

3. **Frequency Range:**
   - ADF4382: 687.5 MHz to 22 GHz
   - ADF4377: Different frequency range

4. **No GPIO Pins:**
   - ADF4382 initialization doesn't require GPIO pins (unlike ADF4377 which has optional CE, ENCLK1, ENCLK2)

## Framework Architecture Summary

### no-OS Framework Structure

The Analog Devices no-OS framework provides:

1. **Hardware-Agnostic Drivers**: Device drivers (like `adf4382.c`) that work across platforms
2. **Platform Abstraction Layer**: Platform-specific implementations (like `linux_spi.c`) that map to OS/hardware
3. **Abstraction Interfaces**: Common APIs (like `no_os_spi.h`) that abstract hardware details

### Key Components

- **SPI Abstraction**: `no_os_spi_init()` → `linux_spi_init()` → opens `/dev/spidevX.Y`
- **Driver Integration**: `adf4382_init()` uses `no_os_spi_init()` which calls platform-specific `linux_spi_ops.init()`
- **Register Access**: `adf4382_spi_write()` → `no_os_spi_write_and_read()` → `linux_spi_write_and_read()` → `ioctl(SPI_IOC_MESSAGE)`

### How It Works

1. **Initialization Chain**:
   ```
   main() 
   → adf4382_init() 
   → no_os_spi_init() 
   → linux_spi_init() 
   → open("/dev/spidev0.0")
   ```

2. **Register Write**:
   ```
   adf4382_spi_write(dev, 0x0A, 0xAA)
   → no_os_spi_write_and_read(spi_desc, buffer, 3)
   → linux_spi_write_and_read(spi_desc, buffer, 3)
   → ioctl(fd, SPI_IOC_MESSAGE(1), &transfer)
   ```

## License

This project follows the Analog Devices no-OS framework license. See the main no-OS repository for license details.

