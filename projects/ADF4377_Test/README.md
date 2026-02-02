# ADF4377 Test Project for Raspberry Pi

This project demonstrates how to use the Analog Devices no-OS framework to control an ADF4377 frequency synthesizer chip on a Raspberry Pi using Linux spidev for SPI communication.

## Overview

The program:
1. Initializes SPI communication via Linux spidev (`/dev/spidev0.0`)
2. Initializes the ADF4377 synthesizer chip
3. Writes to REG001D (0x1D) to set MUXOUT bits [7:4] to `1000` (binary), which corresponds to `ADF4377_MUXOUT_HIGH` (0x8), generating a HIGH signal on the MUXOUT pin

## Hardware Requirements

- Raspberry Pi (any model with SPI support)
- ADF4377 evaluation board or chip
- SPI connections:
  - MOSI (Master Out Slave In)
  - MISO (Master In Slave Out)
  - SCLK (Serial Clock)
  - CS (Chip Select)
  - GND (Ground)
  - Power supply for ADF4377

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
cd /home/rpidev1/no-OS/no-OS/projects/ADF4377_Test
make PLATFORM=linux
```

This will create an executable named `adf4377_test` in the `build` directory.

### 4. Run the Program

**Note:** You may need to run with `sudo` or add your user to the `spi` group:
```bash
# Option 1: Run with sudo
sudo ./build/adf4377_test

# Option 2: Add user to spi group (one-time setup)
sudo usermod -a -G spi $USER
# Log out and log back in, then run:
./build/adf4377_test
```

### 5. Command Line Options

```bash
./build/adf4377_test [options]

Options:
  --spi-device <id>  SPI device ID (default: 0 for /dev/spidev0.X)
  --spi-cs <cs>      SPI chip select (default: 0)
  --help, -h         Show help message

Examples:
  ./build/adf4377_test
  ./build/adf4377_test --spi-device 0 --spi-cs 0
```

## Project Structure

```
ADF4377_Test/
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
- Speed: 2 MHz
- Mode: SPI Mode 0 (CPOL=0, CPHA=0)
- Bit order: MSB first

### ADF4377 Initialization

The ADF4377 is initialized with:
- Device ID: ADF4377
- SPI: 4-wire mode
- Reference clock: 125 MHz
- Reference doubler: Enabled
- Output frequency: 10 GHz
- Charge pump current: 10.1 mA

### REG001D Write Operation

The program writes to register 0x1D (REG001D) to configure the MUXOUT pin:
- Bits [7:4]: MUXOUT selection
- Value: `1000` (binary) = `0x8` = `ADF4377_MUXOUT_HIGH`
- This sets the MUXOUT pin to output a HIGH signal

The write is performed using `adf4377_spi_update_bit()` which:
1. Reads the current register value
2. Updates only the MUXOUT field (bits [7:4])
3. Writes the updated value back

## Expected Output

When the program runs successfully, you should see:

```
========================================
ADF4377 Test Program for Raspberry Pi
========================================

SPI Configuration:
  Device: /dev/spidev0.0
  Speed: 2000000 Hz
  Mode: 0 (CPOL=0, CPHA=0)

Step 1: Initializing ADF4377...
  ✓ ADF4377 initialized successfully

Step 2: Reading REG001D (MUXOUT register)...
  Current REG001D value: 0xXX
  MUXOUT bits [7:4]: 0xX

Step 3: Writing to REG001D to set MUXOUT bits [7:4] to 1000 (HIGH)...
  ✓ Successfully wrote MUXOUT = HIGH (0x8) to bits [7:4]

Step 4: Verifying write by reading REG001D...
  REG001D value after write: 0x8X
  MUXOUT bits [7:4]: 0x8 ✓ (HIGH - correct!)

Step 5: Cleaning up...
  ✓ Cleanup successful

========================================
Test completed successfully!
MUXOUT pin should now output HIGH signal
========================================
```

## Troubleshooting

### SPI Device Not Found
- Ensure SPI is enabled: `sudo raspi-config` -> Interface Options -> SPI
- Reboot after enabling SPI
- Check device exists: `ls -l /dev/spidev*`

### Permission Denied
- Run with `sudo`: `sudo ./build/adf4377_test`
- Or add user to spi group: `sudo usermod -a -G spi $USER` (then logout/login)

### Initialization Failed
- Check hardware connections (MOSI, MISO, SCLK, CS, GND)
- Verify ADF4377 power supply
- Check SPI speed (may need to reduce if too fast)
- Verify chip select pin is correct

### No Response from Chip
- Verify SPI connections
- Check power supply to ADF4377
- Try reducing SPI speed in `parameters.h`
- Verify chip select is connected correctly

## Framework Architecture Summary

### no-OS Framework Structure

The Analog Devices no-OS framework provides:

1. **Hardware-Agnostic Drivers**: Device drivers (like `adf4377.c`) that work across platforms
2. **Platform Abstraction Layer**: Platform-specific implementations (like `linux_spi.c`) that map to OS/hardware
3. **Abstraction Interfaces**: Common APIs (like `no_os_spi.h`) that abstract hardware details

### Key Components

- **SPI Abstraction**: `no_os_spi_init()` → `linux_spi_init()` → opens `/dev/spidevX.Y`
- **Driver Integration**: `adf4377_init()` uses `no_os_spi_init()` which calls platform-specific `linux_spi_ops.init()`
- **Register Access**: `adf4377_spi_write()` → `no_os_spi_write_and_read()` → `linux_spi_write_and_read()` → `ioctl(SPI_IOC_MESSAGE)`

### How It Works

1. **Initialization Chain**:
   ```
   main() 
   → adf4377_init() 
   → no_os_spi_init() 
   → linux_spi_init() 
   → open("/dev/spidev0.0")
   ```

2. **Register Write**:
   ```
   adf4377_spi_write(dev, 0x1D, value)
   → no_os_spi_write_and_read(spi_desc, buffer, 3)
   → linux_spi_write_and_read(spi_desc, buffer, 3)
   → ioctl(fd, SPI_IOC_MESSAGE(1), &transfer)
   ```

## License

This project follows the Analog Devices no-OS framework license. See the main no-OS repository for license details.



