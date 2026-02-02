#!/usr/bin/env python3
"""
Simple test to verify SPI CS is working
"""
import struct
import fcntl
import os
import time
from ctypes import *

# SPI ioctl definitions
SPI_IOC_RD_MODE = 0x80016b01
SPI_IOC_WR_MODE = 0x40016b01
SPI_IOC_RD_MAX_SPEED_HZ = 0x80046b04
SPI_IOC_WR_MAX_SPEED_HZ = 0x40046b04
SPI_IOC_MESSAGE = lambda n: 0x40006b00 + (n << 16)

class spi_ioc_transfer(Structure):
    _pack_ = 1
    _fields_ = [
        ('tx_buf', c_ulonglong),  # Use 64-bit for pointer
        ('rx_buf', c_ulonglong),  # Use 64-bit for pointer
        ('len', c_uint32),
        ('speed_hz', c_uint32),
        ('delay_usecs', c_uint16),
        ('bits_per_word', c_uint8),
        ('cs_change', c_uint8),
        ('tx_nbits', c_uint8),
        ('rx_nbits', c_uint8),
        ('word_delay_usecs', c_uint8),
        ('pad1', c_uint8),  # Padding
        ('pad2', c_uint8),  # Padding
        ('pad3', c_uint8),  # Padding
    ]

def test_spi_cs():
    print("Opening /dev/spidev0.0...")
    fd = os.open('/dev/spidev0.0', os.O_RDWR)
    
    # Set mode
    mode = struct.pack('B', 0)  # SPI_MODE_0
    fcntl.ioctl(fd, SPI_IOC_WR_MODE, mode)
    print("✓ SPI Mode set to 0")
    
    # Set speed
    speed = struct.pack('I', 100000)  # 100 kHz
    fcntl.ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, speed)
    print("✓ SPI Speed set to 100 kHz")
    
    # Prepare test data
    tx_data = (c_uint8 * 3)(0xAA, 0xBB, 0xCC)
    rx_data = (c_uint8 * 3)(0, 0, 0)
    
    tr = spi_ioc_transfer(
        tx_buf=cast(tx_data, c_void_p).value,
        rx_buf=cast(rx_data, c_void_p).value,
        len=3,
        speed_hz=100000,
        delay_usecs=0,
        bits_per_word=8,
        cs_change=0,  # Keep CS low between transfers
        tx_nbits=0,
        rx_nbits=0,
        word_delay_usecs=0,
        pad=0
    )
    
    print("\n" + "="*50)
    print("Sending 3 bytes: 0xAA 0xBB 0xCC")
    print("="*50)
    print("⚠️  WATCH YOUR SCOPE ON PIN 24 (GPIO 8) NOW!")
    print("⚠️  CS should go LOW during this transfer")
    print("="*50)
    
    for i in range(5):
        print(f"\nTransfer {i+1}/5...")
        ret = fcntl.ioctl(fd, SPI_IOC_MESSAGE(1), tr)
        if ret < 0:
            print(f"✗ Transfer failed: {os.strerror(-ret)}")
        else:
            print(f"✓ Transfer successful")
            print(f"  Sent: {[hex(b) for b in tx_data]}")
            print(f"  Received: {[hex(b) for b in rx_data]}")
        time.sleep(0.5)
    
    print("\n" + "="*50)
    print("Test complete. CS should return HIGH now.")
    print("="*50)
    
    os.close(fd)

if __name__ == "__main__":
    test_spi_cs()

