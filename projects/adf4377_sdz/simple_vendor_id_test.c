/***************************************************************************//**
 *   @file   simple_vendor_id_test.c
 *   @brief  Simple ADF4377 Vendor ID Test - Direct SPI access
 *   @author Generated for Linux platform
********************************************************************************
 * Copyright 2025(c) Analog Devices, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL ANALOG DEVICES, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/spi/spidev.h>

#define SPI_DEVICE "/dev/spidev0.0"

int main() {
    int fd;
    int ret;
    uint8_t tx_buffer[3];
    uint8_t rx_buffer[3];
    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = 1000000; // 1 MHz
    uint8_t vendor_id_lsb = 0;
    uint8_t vendor_id_msb = 0;
    uint16_t vendor_id = 0;
    
    printf("=== ADF4377 Simple Vendor ID Test ===\n");
    printf("This test uses direct SPI access to read vendor ID registers\n");
    printf("SPI Device: %s\n", SPI_DEVICE);
    printf("Mode: %d, Bits: %d, Speed: %d Hz\n\n", mode, bits, speed);
    
    // Check if SPI device exists
    if (access(SPI_DEVICE, F_OK) != 0) {
        printf("❌ SPI device %s not found!\n", SPI_DEVICE);
        printf("Please enable SPI on your Raspberry Pi:\n");
        printf("1. Run: sudo raspi-config\n");
        printf("2. Go to Interface Options -> SPI -> Enable\n");
        printf("3. Reboot the system\n");
        return -1;
    }
    printf("✓ SPI device found\n");
    
    // Open SPI device
    fd = open(SPI_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
        printf("You may need to run with sudo or add your user to the spi group\n");
        return -1;
    }
    printf("✓ SPI device opened\n");
    
    // Set SPI mode
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret < 0) {
        perror("Failed to set SPI mode");
        close(fd);
        return -1;
    }
    
    // Set bits per word
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret < 0) {
        perror("Failed to set bits per word");
        close(fd);
        return -1;
    }
    
    // Set max speed
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret < 0) {
        perror("Failed to set max speed");
        close(fd);
        return -1;
    }
    
    printf("✓ SPI configured successfully\n");
    
    // Test 1: Reset chip first
    printf("\n1. Sending reset command...\n");
    tx_buffer[0] = 0x00; tx_buffer[1] = 0x00; tx_buffer[2] = 0x81; // Reset
    memset(rx_buffer, 0, 3);
    
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = 3,
        .speed_hz = speed,
        .bits_per_word = bits,
    };
    
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if (ret < 0) {
        perror("SPI transfer failed");
        close(fd);
        return -1;
    }
    
    printf("   Sent: 0x%02X%02X%02X, Received: 0x%02X%02X%02X\n", 
           tx_buffer[0], tx_buffer[1], tx_buffer[2],
           rx_buffer[0], rx_buffer[1], rx_buffer[2]);
    
    // Wait for reset
    usleep(10000); // 10ms
    
    // Test 2: Read vendor ID LSB register (0x000C)
    printf("\n2. Reading vendor ID LSB register (0x000C)...\n");
    printf("   Expected: 0x56 (according to datasheet)\n");
    tx_buffer[0] = 0x80; tx_buffer[1] = 0x0C; tx_buffer[2] = 0x00; // Read from 0x000C
    memset(rx_buffer, 0, 3);
    
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if (ret < 0) {
        perror("SPI transfer failed");
        close(fd);
        return -1;
    }
    
    printf("   Sent: 0x%02X%02X%02X, Received: 0x%02X%02X%02X\n", 
           tx_buffer[0], tx_buffer[1], tx_buffer[2],
           rx_buffer[0], rx_buffer[1], rx_buffer[2]);
    
    vendor_id_lsb = rx_buffer[2];
    
    if (vendor_id_lsb == 0x56) {
        printf("   ✓ SUCCESS! Got expected vendor ID LSB: 0x%02X\n", vendor_id_lsb);
    } else if (vendor_id_lsb != 0) {
        printf("   ⚠ PARTIAL SUCCESS! Got some response: 0x%02X\n", vendor_id_lsb);
        printf("   This means the chip is responding but might be in wrong mode\n");
    } else {
        printf("   ✗ FAILURE! No response from chip (all zeros)\n");
    }
    
    // Test 3: Read vendor ID MSB register (0x000D)
    printf("\n3. Reading vendor ID MSB register (0x000D)...\n");
    printf("   Expected: 0x04 (according to datasheet)\n");
    tx_buffer[0] = 0x80; tx_buffer[1] = 0x0D; tx_buffer[2] = 0x00; // Read from 0x000D
    memset(rx_buffer, 0, 3);
    
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if (ret < 0) {
        perror("SPI transfer failed");
        close(fd);
        return -1;
    }
    
    printf("   Sent: 0x%02X%02X%02X, Received: 0x%02X%02X%02X\n", 
           tx_buffer[0], tx_buffer[1], tx_buffer[2],
           rx_buffer[0], rx_buffer[1], rx_buffer[2]);
    
    vendor_id_msb = rx_buffer[2];
    
    if (vendor_id_msb == 0x04) {
        printf("   ✓ SUCCESS! Got expected vendor ID MSB: 0x%02X\n", vendor_id_msb);
    } else if (vendor_id_msb != 0) {
        printf("   ⚠ PARTIAL SUCCESS! Got some response: 0x%02X\n", vendor_id_msb);
    } else {
        printf("   ✗ FAILURE! No response from chip (all zeros)\n");
    }
    
    // Combine LSB and MSB to get full vendor ID
    vendor_id = (vendor_id_msb << 8) | vendor_id_lsb;
    printf("\n4. Full Vendor ID: 0x%04X\n", vendor_id);
    
    if (vendor_id == 0x0456) {
        printf("   ✓ SUCCESS! Full vendor ID matches expected value (0x0456 - Analog Devices)\n");
    } else if (vendor_id != 0) {
        printf("   ⚠ PARTIAL SUCCESS! Got vendor ID: 0x%04X (expected 0x0456)\n", vendor_id);
    } else {
        printf("   ✗ FAILURE! No vendor ID response\n");
    }
    
    close(fd);
    
    printf("\n=== Test Complete ===\n");
    if (vendor_id == 0x0456) {
        printf("✓ ADF4377 is working correctly!\n");
    } else if (vendor_id != 0) {
        printf("⚠ ADF4377 is responding but may need configuration\n");
    } else {
        printf("✗ ADF4377 not responding - check connections and power\n");
    }
    
    return 0;
}
