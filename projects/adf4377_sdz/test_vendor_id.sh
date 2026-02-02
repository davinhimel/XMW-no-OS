#!/bin/bash

# ADF4377 Vendor ID Test Script
# This script tests the ADF4377 vendor ID example on Linux SPI

echo "=== ADF4377 Vendor ID Test Script ==="
echo ""

# Check if SPI device exists
if [ ! -e /dev/spidev0.0 ]; then
    echo "ERROR: SPI device /dev/spidev0.0 not found!"
    echo "Please ensure SPI is enabled on your Raspberry Pi:"
    echo "1. Run: sudo raspi-config"
    echo "2. Go to Interface Options -> SPI -> Enable"
    echo "3. Reboot the system"
    exit 1
fi

echo "✓ SPI device /dev/spidev0.0 found"

# Check if executable exists
if [ ! -f "./build/adf4377_sdz.out" ]; then
    echo "ERROR: Executable not found. Please run 'make' first."
    exit 1
fi

echo "✓ Executable found"

# Check permissions
if [ ! -r /dev/spidev0.0 ] || [ ! -w /dev/spidev0.0 ]; then
    echo "WARNING: Permission denied for /dev/spidev0.0"
    echo "You may need to run with sudo or add your user to the spi group:"
    echo "sudo usermod -a -G spi \$USER"
    echo "Then log out and log back in."
    echo ""
    echo "Running with sudo..."
    sudo ./build/adf4377_sdz.out
else
    echo "✓ SPI device permissions OK"
    echo ""
    echo "Running ADF4377 vendor ID test..."
    ./build/adf4377_sdz.out
fi

echo ""
echo "=== Test Complete ==="
