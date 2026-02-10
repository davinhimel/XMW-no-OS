#!/usr/bin/env python3
"""
ADF4377 Test (SWIG) — same behavior as platform/linux/main.c

Uses the SWIG-generated adf4377 module to:
1. Initialize SPI via Linux spidev (/dev/spidev0.0)
2. Initialize the ADF4377
3. Read REG001D (MUXOUT), write MUXOUT bits [7:4] to HIGH (0x8), read back and verify
4. Clean up

Run from so/ with: python3 adf4377_test_swig.py [--spi-device N] [--spi-cs N]
Or with LD_LIBRARY_PATH set to libadnoos drivers_so and core/build if needed.
"""

import argparse
import sys

# Optional: ensure so/ is on path when run from project root
from pathlib import Path
SO_DIR = Path(__file__).resolve().parent
if str(SO_DIR) not in sys.path:
    sys.path.insert(0, str(SO_DIR))

import adf4377

# Register address (same as main.c ADF4377_REG(0x1D))
ADF4377_REG_1D = 0x1D


def main():
    parser = argparse.ArgumentParser(description="ADF4377 Test (SWIG) — same as main.c")
    parser.add_argument("--spi-device", type=int, default=0, help="SPI device ID (default 0)")
    parser.add_argument("--spi-cs", type=int, default=0, help="SPI chip select (default 0)")
    args = parser.parse_args()

    spi_device_id = args.spi_device
    spi_cs = args.spi_cs
    # Match main.c: 2 MHz (parameters.h comment said 2 MHz; value in parameters.h was 10000 — using 2e6 for 2 MHz)
    spi_speed_hz = 10000

    print("========================================")
    print("ADF4377 Test Program for Raspberry Pi")
    print("(SWIG bindings)")
    print("========================================\n")

    print("SPI Configuration:")
    print("  Device: /dev/spidev{}.{}".format(spi_device_id, spi_cs))
    print("  Speed: {} Hz".format(spi_speed_hz))
    print("  Mode: 0 (CPOL=0, CPHA=0)\n")

    # Build init param (same as common_data.c + parameters). Cast to int so SWIG accepts uint32_t/uint8_t.
    spi_extra = adf4377.linux_spi_init_param()
    spi_extra.device_id = int(spi_device_id)
    spi_extra.chip_select = int(spi_cs) & 0xFF
    spi_extra.max_speed_hz = int(spi_speed_hz)
    spi_extra.mode = 0

    spi_init = adf4377.no_os_spi_init_param()
    spi_init.device_id = int(spi_device_id)
    spi_init.max_speed_hz = int(spi_speed_hz)
    spi_init.chip_select = int(spi_cs) & 0xFF
    spi_init.mode = int(adf4377.NO_OS_SPI_MODE_0)
    spi_init.bit_order = int(adf4377.NO_OS_SPI_BIT_ORDER_LSB_FIRST)
    spi_init.lanes = int(adf4377.NO_OS_SPI_SINGLE_LANE)
    spi_init.platform_ops = adf4377.get_linux_spi_ops()
    spi_init.platform_delays = adf4377.no_os_platform_spi_delays()
    spi_init.extra = spi_extra
    spi_init.parent = None

    ip = adf4377.adf4377_init_param()
    ip.spi_init = spi_init
    ip.gpio_ce_param = None
    ip.gpio_enclk1_param = None
    ip.gpio_enclk2_param = None
    ip.dev_id = int(adf4377.ADF4377)
    ip.spi4wire = True
    ip.clkin_freq = 125000000
    ip.f_clk = 10000000000
    ip.ref_doubler_en = 1
    ip.ref_div_factor = 1
    ip.muxout_select = int(adf4377.ADF4377_MUXOUT_HIGH_Z)
    ip.cp_i = int(adf4377.ADF4377_CP_10MA1)
    ip.clkout_op = int(adf4377.ADF4377_CLKOUT_640MV)

    # Step 1: Initialize ADF4377 (same as main.c: adf4377_init(&adf4377_dev, &adf4377_ip))
    print("Step 1: Initializing ADF4377...")
    ret, dev = adf4377.adf4377_init(ip)
    if ret < 0:
        print("ERROR: Failed to initialize ADF4377 (error: {})".format(ret))
        print("Please check:")
        print("  - SPI device exists: /dev/spidev{}.{}".format(spi_device_id, spi_cs))
        print("  - SPI is enabled (raspi-config -> Interface Options -> SPI)")
        print("  - You have permissions (may need sudo or add user to spi group)")
        print("  - Hardware connections are correct")
        return ret if ret < 0 else -1
    print("  ✓ ADF4377 initialized successfully\n")

    # Step 2: Read current value of REG001D (SWIG OUTPUT typemap returns ret, data)
    print("Step 2: Reading REG001D (MUXOUT register)...")
    ret, read_back = adf4377.adf4377_spi_read(dev, ADF4377_REG_1D)
    if ret < 0:
        print("ERROR: Failed to read REG001D (error: {})".format(ret))
        adf4377.adf4377_remove(dev)
        return ret
    print("  Current REG001D value: 0x{:02X}".format(read_back))
    print("  MUXOUT bits [7:4]: 0x{:1X}\n".format((read_back >> 4) & 0x0F))

    # Step 3: Write MUXOUT bits [7:4] to 1000 (HIGH) (same as main.c: ADF4377_MUXOUT_MSK, ADF4377_MUXOUT(ADF4377_MUXOUT_HIGH))
    # C macros: ADF4377_MUXOUT_MSK = NO_OS_GENMASK(7,4) = 0xF0; ADF4377_MUXOUT(x) = (x<<4)&0xF0
    MUXOUT_MSK = 0xF0
    muxout_high_val = (adf4377.ADF4377_MUXOUT_HIGH << 4) & 0xF0
    print("Step 3: Writing to REG001D to set MUXOUT bits [7:4] to 1000 (HIGH)...")
    ret = adf4377.adf4377_spi_update_bit(dev, ADF4377_REG_1D, MUXOUT_MSK, muxout_high_val)
    if ret < 0:
        print("ERROR: Failed to write to REG001D (error: {})".format(ret))
        adf4377.adf4377_remove(dev)
        return ret
    print("  ✓ Successfully wrote MUXOUT = HIGH (0x8) to bits [7:4]\n")

    # Step 4: Read back REG001D to verify
    print("Step 4: Verifying write by reading REG001D...")
    ret, read_back = adf4377.adf4377_spi_read(dev, ADF4377_REG_1D)
    if ret < 0:
        print("ERROR: Failed to read REG001D (error: {})".format(ret))
        adf4377.adf4377_remove(dev)
        return ret
    muxout_bits = (read_back >> 4) & 0x0F
    print("  REG001D value after write: 0x{:02X}".format(read_back))
    print("  MUXOUT bits [7:4]: 0x{:1X}".format(muxout_bits), end="")
    if muxout_bits == adf4377.ADF4377_MUXOUT_HIGH:
        print(" ✓ (HIGH - correct!)")
    else:
        print(" ✗ (Expected 0x8, got 0x{:1X})".format(muxout_bits))
        ret = -1

    # Step 5: Cleanup
    print("\nStep 5: Cleaning up...")
    ret_remove = adf4377.adf4377_remove(dev)
    if ret_remove < 0:
        print("WARNING: Error during cleanup (error: {})".format(ret_remove))
    else:
        print("  ✓ Cleanup successful")

    print("\n========================================")
    if ret == 0:
        print("Test completed successfully!")
        print("MUXOUT pin should now output HIGH signal")
    else:
        print("Test completed with errors")
    print("========================================")

    return ret


if __name__ == "__main__":
    sys.exit(main())
