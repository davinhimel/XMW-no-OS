# ADF4377 Vendor ID Example for Linux

This example demonstrates how to read the vendor ID from an ADF4377 frequency synthesizer using the no-OS framework on Linux with SPI communication.

## Overview

The ADF4377 is a wideband frequency synthesizer with integrated VCO. This example reads the vendor ID registers (0x0C and 0x0D) to verify proper communication with the device.

## Hardware Requirements

- Raspberry Pi (or any Linux system with SPI support)
- ADF4377 evaluation board or breakout board
- SPI connections:
  - MOSI (Master Out Slave In) - Pi GPIO 10 (SPI0_MOSI)
  - MISO (Master In Slave Out) - Pi GPIO 9 (SPI0_MISO)  
  - SCLK (Serial Clock) - Pi GPIO 11 (SPI0_SCLK)
  - CS (Chip Select) - Pi GPIO 8 (SPI0_CE0_N)
  - CE (Chip Enable) - Pi GPIO 18
  - ENCLK1 (Enable CLK1) - Pi GPIO 19
  - ENCLK2 (Enable CLK2) - Pi GPIO 20

## Software Setup

### 1. Enable SPI on Raspberry Pi

```bash
sudo raspi-config
# Navigate to: Interface Options -> SPI -> Enable
# Reboot the system
```

### 2. Verify SPI is enabled

```bash
ls -la /dev/spi*
# Should show /dev/spidev0.0 and /dev/spidev0.1
```

### 3. Add user to spi group (optional, for non-root access)

```bash
sudo usermod -a -G spi $USER
# Log out and log back in
```

## Building the Example

```bash
cd /home/rpidev1/no-OS/no-OS/projects/adf4377_sdz
make clean
make
```

This will create the executable at `build/adf4377_sdz.out`.

## Running the Example

### Option 1: Using the test script (recommended)

```bash
./test_vendor_id.sh
```

### Option 2: Running directly

```bash
# If you have SPI permissions:
./build/adf4377_sdz.out

# If you need root permissions:
sudo ./build/adf4377_sdz.out
```

### Option 3: With custom SPI device

```bash
./build/adf4377_sdz.out --spi-device 1  # Use /dev/spidev1.0
```

## Expected Output

If the ADF4377 is properly connected and communicating:

```
=== ADF4377 VENDOR ID TEST ===
Initializing ADF4377 device...
✓ ADF4377 initialized successfully
Reading vendor ID LSB (0x0C)...
Vendor ID LSB: 0x56
Reading vendor ID MSB (0x0D)...
Vendor ID MSB: 0x04
Full Vendor ID: 0x0456
✓ SUCCESS: Vendor ID matches expected value (0x0456 - Analog Devices)
=== TEST COMPLETED ===
```

## Troubleshooting

### No response from chip (all zeros)
- Check power supply to ADF4377
- Verify SPI connections (MOSI, MISO, SCLK, CS)
- Ensure chip enable (CE) is properly connected
- Check that SPI is enabled on the Pi

### Permission denied errors
- Run with `sudo` or add user to spi group
- Check that `/dev/spidev0.0` exists and is accessible

### SPI device not found
- Enable SPI in `raspi-config`
- Reboot the system
- Check if SPI kernel modules are loaded: `lsmod | grep spi`

### Wrong vendor ID
- Verify you're using an ADF4377 (not ADF4382 or other chip)
- Check for loose connections
- Ensure proper power supply voltage

## Code Structure

The example consists of:

- `vendor_id_example.c` - Main example code
- `common_data.c/h` - Platform-specific configuration
- `parameters.c/h` - Linux-specific parameters
- `main.c` - Linux platform main function

## Key Features

- Uses no-OS framework for portable SPI communication
- Proper device initialization before register access
- Error handling and cleanup
- Configurable SPI device selection
- Clear success/failure reporting

## Technical Details

- **SPI Mode**: Mode 0 (CPOL=0, CPHA=0)
- **SPI Speed**: 2 MHz maximum
- **Register Access**: 3-byte SPI transactions
- **Vendor ID**: 0x0456 (Analog Devices)
- **Registers**: 0x0C (LSB), 0x0D (MSB)

## Next Steps

Once the vendor ID test passes, you can:

1. Explore other ADF4377 examples (basic_example.c)
2. Modify frequency settings
3. Use the ADF4377 driver for your own applications
4. Integrate with IIO framework for advanced control

## References

- [ADF4377 Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/adf4377.pdf)
- [no-OS Framework Documentation](https://github.com/analogdevicesinc/no-OS)
- [Raspberry Pi SPI Documentation](https://www.raspberrypi.org/documentation/hardware/raspberrypi/spi/)
