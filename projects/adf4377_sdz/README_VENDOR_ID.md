# ADF4377 Vendor ID Test

This is a simple Linux platform example for the ADF4377 frequency synthesizer that tests basic SPI communication by reading the vendor ID register.

## What it does

The example:
1. Initializes the ADF4377 device using the no-OS framework
2. Reads the vendor ID LSB register (0x0C)
3. Reads the vendor ID MSB register (0x0D)
4. Combines them to get the full vendor ID
5. Verifies it matches the expected value (0x0456 for Analog Devices)

## Building

```bash
cd /home/xmicrowave/no-OS/no-OS/projects/adf4377_sdz
make clean
make EXAMPLE=vendor_id
```

## Running

```bash
# Run with default SPI device (/dev/spidev0.0)
./build/adf4377_sdz.out

# Run with specific SPI device
./build/adf4377_sdz.out --spi-device 1

# Show help
./build/adf4377_sdz.out --help
```

## Hardware Requirements

- Raspberry Pi 4 (or other Linux system with SPI)
- ADF4377 chip connected to SPI interface
- SPI connections:
  - MOSI (Master Out Slave In)
  - MISO (Master In Slave Out) 
  - SCLK (Serial Clock)
  - CS (Chip Select)
- Power supply connections

## Expected Output

If the ADF4377 is connected correctly, you should see:
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

If you get permission errors:
```bash
sudo chmod 666 /dev/spidev0.0
```

If the test fails, check:
1. SPI connections are correct
2. Power supply is connected
3. SPI device permissions
4. ADF4377 chip is properly connected
