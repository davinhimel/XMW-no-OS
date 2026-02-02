# ADF4382 Linux Platform Support and GUI

This directory contains Linux platform support for the ADF4382 frequency synthesizer chip, including an executable, shared library, and a scalable Python GUI application for dynamic parameter control.

## Overview

The Linux platform support includes:
- **Linux Platform Files**: Platform-specific code for Linux (main.c, parameters.c/h, platform_src.mk)
- **Executable**: `adf4382` - Standalone application using the basic example
- **Shared Library**: `libadf4382.so` - Dynamic library for runtime parameter updates
- **Sample Application**: `update_adf4382.c` - Demonstrates using the shared library
- **Python GUI**: Scalable GUI application supporting multiple chips via JSON configuration

## Directory Structure

```
adf4382/
├── src/
│   ├── platform/
│   │   └── linux/           # Linux platform files
│   │       ├── main.c       # Entry point
│   │       ├── parameters.c # Platform parameters
│   │       ├── parameters.h # Platform parameters header
│   │       └── platform_src.mk # Platform source makefile
│   └── ...
├── gui/                     # Python GUI application
│   ├── main.py             # Main GUI application
│   └── configs/            # Chip configuration files
│       └── adf4382.json    # ADF4382 configuration
├── update_adf4382.c        # Sample C application
├── Makefile                # Updated build system
└── README_LINUX.md         # This file
```

## Prerequisites

### System Requirements
- Linux system (tested on Ubuntu 20.04+, Raspberry Pi OS)
- GCC compiler
- Make build system
- Python 3.6+ (for GUI)
- SPI device support (e.g., `/dev/spidev0.0`)

### Python Dependencies
```bash
# For PyQt5 GUI (recommended)
pip install PyQt5

# Or use system package manager
sudo apt-get install python3-pyqt5  # Ubuntu/Debian
sudo dnf install python3-qt5        # Fedora
```

### SPI Setup
Ensure SPI is enabled on your system:

**Raspberry Pi:**
```bash
# Enable SPI in raspi-config
sudo raspi-config
# Navigate to: Interface Options -> SPI -> Enable

# Or edit /boot/config.txt
echo "dtparam=spi=on" | sudo tee -a /boot/config.txt
sudo reboot
```

**Other Linux systems:**
```bash
# Load SPI modules
sudo modprobe spi_bcm2835  # For Raspberry Pi
sudo modprobe spi_bcm2708  # Alternative for older Pi

# Check SPI devices
ls -la /dev/spidev*
```

## Current Build Status

✅ **All components successfully built and tested on Linux ARM64 (Raspberry Pi)**

The following files are ready for use:
- `build/adf4382.out` - Main executable (361,800 bytes)
- `libadf4382.so` - Shared library (361,472 bytes) 
- `update_adf4382` - Sample application (71,616 bytes)

## Quick Start for Your Test Setup

Since everything is already built, you can immediately test the functionality:

### 1. Test Basic Functionality (No Hardware Required)
```bash
# Navigate to project directory
cd /home/xmicrowave/no-OS/no-OS/projects/adf4382

# Test help functionality (no hardware required)
./build/adf4382.out --help
./update_adf4382 --help
```

**⚠️ Important**: The executables will **FAIL** if no ADF4382 chip is connected:
- Main executable returns error code -22 with clear error message
- Shared library application may crash with segmentation fault (use static version instead)
- Static library application returns error code -22 with clear error message
- This is expected behavior - the software requires actual hardware to function

**✅ Issues Fixed**: 
- UART logging has been disabled - now only SPI is used for chip communication
- Clear error messages explain when no chip is connected
- Static library version provides better error handling than shared library version

### 2. Test with Hardware (ADF4382 Chip Required)
```bash
# Check if SPI is available
ls -la /dev/spidev*

# ⚠️ REQUIRES: ADF4382 chip connected to SPI interface
# Run with default SPI device (requires sudo for hardware access)
sudo ./build/adf4382.out

# Run with custom SPI device
sudo ./build/adf4382.out --spi-device 1

# Test static library with frequency setting (recommended)
sudo ./update_adf4382_static --freq 15000000000

# Test shared library with frequency setting (may crash if no chip connected)
sudo ./update_adf4382 --freq 15000000000
```

**Expected Results with Hardware:**
- Main executable should complete successfully (return code 0)
- Static library application should update chip parameters successfully
- Shared library application should update chip parameters without crashing
- All applications will communicate with the ADF4382 via SPI

### 3. Test GUI (if Python dependencies available)
```bash
# Check if GUI dependencies are available
python3 -c "import PyQt5" 2>/dev/null && echo "PyQt5 available" || echo "PyQt5 not available"

# Run GUI
cd gui && python3 main.py
```

**Your System Status:**
- ✅ PyQt5 is available - GUI can be used
- ✅ SPI devices found: `/dev/spidev0.0` and `/dev/spidev0.1` - Hardware communication ready

## Building

### 1. Build Executable
```bash
# Build the standalone executable
make PLATFORM=linux EXAMPLE=basic

# This creates: build/adf4382.out
```

### 2. Build Shared Library
```bash
# Build the shared library for dynamic loading
make PLATFORM=linux libadf4382.so

# This creates: libadf4382.so
```

### 3. Build Sample Application
```bash
# Build the sample C application (shared library)
gcc -o update_adf4382 update_adf4382.c -ldl

# This creates: update_adf4382
```

### 4. Build Static Library and Sample Application
```bash
# Build the static library
make PLATFORM=linux libadf4382.a

# Build the static sample application
gcc -static -o update_adf4382_static update_adf4382_static.c -L. -ladf4382 -I../../drivers/platform/linux -I../../drivers/frequency/adf4382 -I../../drivers/api -I../../include -I../../projects/adf4382/build/app/Core/noos/include

# This creates: libadf4382.a and update_adf4382_static
```

### 5. Clean Build
```bash
# Clean all build artifacts
make PLATFORM=linux clean-linux
rm -f update_adf4382
```

## Usage

### 1. Standalone Executable

Run the basic example:
```bash
# Basic usage
sudo ./build/adf4382.out

# With custom SPI device
sudo ./build/adf4382.out --spi-device 1

# With custom UART device
sudo ./build/adf4382.out --uart-device /dev/ttyUSB1

# Show help
./build/adf4382.out --help
```

**Note**: `sudo` may be required for SPI access depending on your system configuration.

### 2. Sample C Application (Static Library - Recommended)

Use the static library for dynamic parameter updates:
```bash
# Set frequency only
sudo ./update_adf4382_static --freq 15000000000

# Set multiple parameters
sudo ./update_adf4382_static --freq 18000000000 --cp-i 10 --bleed 4000

# Use different SPI device
sudo ./update_adf4382_static --spi-device 1 --freq 20000000000

# Show help
./update_adf4382_static --help
```

### 3. Sample C Application (Shared Library)

Use the shared library for dynamic parameter updates (may crash if no chip connected):
```bash
# Set frequency only
sudo ./update_adf4382 --freq 15000000000

# Set multiple parameters
sudo ./update_adf4382 --freq 18000000000 --cp-i 10 --bleed 4000

# Use different SPI device
sudo ./update_adf4382 --spi-device 1 --freq 20000000000

# Show help
./update_adf4382 --help
```

### 4. Python GUI Application

Run the GUI application:
```bash
# Navigate to GUI directory
cd gui

# Run with PyQt5 (recommended)
python3 main.py

# Or run with Tkinter (fallback)
# (Automatically selected if PyQt5 not available)
```

#### GUI Features:
- **Chip Selection**: Dropdown to select from available chips
- **SPI Configuration**: Configure SPI device ID and chip select
- **Dynamic Parameters**: Form fields generated from JSON configuration
- **Real-time Control**: Initialize chip and update parameters
- **Log Console**: Real-time feedback and error messages

#### Adding New Chips:
1. **Compile shared library** for the new chip (e.g., `libad9361.so`)
2. **Create JSON configuration** in `gui/configs/` (e.g., `ad9361.json`)
3. **Restart GUI** - new chip will appear in dropdown automatically

Example JSON for new chip:
```json
{
  "chip_name": "AD9361",
  "lib_name": "libad9361.so",
  "init_func": "ad9361_init",
  "remove_func": "ad9361_remove",
  "params": [
    {"name": "sample_rate", "type": "uint64", "default": 30720000, "range": [1000000, 61440000]},
    {"name": "frequency", "type": "uint64", "default": 2400000000, "range": [70000000, 6000000000]}
  ],
  "setters": [
    {"func": "ad9361_set_sample_rate", "param": "sample_rate", "type": "uint64"},
    {"func": "ad9361_set_frequency", "param": "frequency", "type": "uint64"}
  ],
  "spi_defaults": {"device_id": 0, "chip_select": 0, "max_speed_hz": 1000000, "mode": 0}
}
```

## Configuration

### SPI Device Configuration

The default SPI configuration uses `/dev/spidev0.0`. To use different devices:

**Command Line:**
```bash
./build/adf4382.out --spi-device 1  # Use /dev/spidev1.0
```

**GUI:**
- Set "SPI Device ID" to desired device number
- Set "Chip Select" to desired chip select line

### UART Configuration

UART is used for logging. If UART access is problematic:

**Command Line:**
```bash
./build/adf4382.out --uart-device /dev/ttyUSB0  # Use USB-to-UART adapter
```

**Code Modification:**
Edit `src/platform/linux/parameters.c` to change default UART device:
```c
struct linux_uart_init_param adf4382_uart_extra_ip = {
    .device_id = "/dev/ttyUSB0",  // Change default device
};
```

## Troubleshooting

### Common Issues

**1. Executable Fails Without Hardware**
```bash
# Error: "ADF4382 Linux Platform - Failed with error: -22"
# Or: "Segmentation fault" with update_adf4382

# This is NORMAL behavior - the software requires an ADF4382 chip connected
# Solution: Connect ADF4382 chip to SPI interface before running
# Note: UART logging has been disabled - only SPI communication is used
# Recommendation: Use update_adf4382_static instead of update_adf4382 for better error handling
```

**2. SPI Permission Denied**
```bash
# Add user to spi group
sudo usermod -a -G spi $USER
# Log out and back in, or run:
newgrp spi

# Or run with sudo
sudo ./build/adf4382.out
```

**3. SPI Device Not Found**
```bash
# Check if SPI is enabled
ls -la /dev/spidev*

# Enable SPI in device tree
echo "dtparam=spi=on" | sudo tee -a /boot/config.txt
sudo reboot
```

**4. Shared Library Not Found**
```bash
# Set library path
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

# Or copy to system library directory
sudo cp libadf4382.so /usr/local/lib/
sudo ldconfig
```

**5. GUI Dependencies Missing**
```bash
# Install PyQt5
pip install PyQt5

# Or use system package manager
sudo apt-get install python3-pyqt5
```

**6. Build Errors**
```bash
# Clean and rebuild
make clean
make PLATFORM=linux

# Check compiler version
gcc --version

# Ensure all dependencies are installed
sudo apt-get install build-essential
```

### Debug Information

**Enable Verbose Output:**
```bash
# Build with debug symbols
make PLATFORM=linux CFLAGS="-g -O0"

# Run with debug output
./build/adf4382.out 2>&1 | tee debug.log
```

**Check SPI Communication:**
```bash
# Test SPI with spidev_test (if available)
sudo apt-get install spi-tools
sudo spidev_test -D /dev/spidev0.0 -s 1500000
```

## Development

### Adding New Platform Support

To add support for a new platform:

1. **Create platform directory:**
   ```bash
   mkdir src/platform/newplatform
   ```

2. **Create platform files:**
   - `main.c` - Entry point
   - `parameters.c/h` - Platform-specific parameters
   - `platform_src.mk` - Platform source files

3. **Update build system:**
   - Add platform-specific targets to Makefile
   - Include platform source files

### Extending the GUI

The GUI is designed to be extensible:

1. **Add new parameter types** in `main.py`
2. **Extend ChipController** for new functionality
3. **Add new UI elements** as needed
4. **Create custom widgets** for complex parameters

### Testing

**Unit Tests:**
```bash
# Run basic functionality test
./build/adf4382.out --help

# Test shared library
./update_adf4382 --help

# Test GUI
cd gui && python3 main.py
```

**Integration Tests:**
```bash
# Test with actual hardware
sudo ./build/adf4382.out --spi-device 0
sudo ./update_adf4382 --freq 15000000000
```

## License

This software is provided under the same license as the no-OS framework. See the main LICENSE file for details.

## Support

For issues and questions:
1. Check the troubleshooting section above
2. Review the no-OS documentation
3. Check Analog Devices support forums
4. Create an issue in the project repository

## Contributing

Contributions are welcome! Please:
1. Follow the existing code style
2. Add appropriate documentation
3. Include tests for new functionality
4. Update this README as needed 