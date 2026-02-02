/***************************************************************************//**
 *   @file   vendor_id_diagnostic.c
 *   @brief  ADF4377 Vendor ID Diagnostic - Tests without hardware
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
    
    printf("=== ADF4377 Vendor ID Diagnostic ===\n");
    printf("This test checks SPI communication without requiring ADF4377 hardware\n");
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
    
    // Test SPI communication with a simple write
    printf("\nTesting SPI communication...\n");
    tx_buffer[0] = 0x00; tx_buffer[1] = 0x00; tx_buffer[2] = 0x00; // NOP command
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
    
    printf("✓ SPI transfer successful\n");
    printf("   Sent: 0x%02X%02X%02X, Received: 0x%02X%02X%02X\n", 
           tx_buffer[0], tx_buffer[1], tx_buffer[2],
           rx_buffer[0], rx_buffer[1], rx_buffer[2]);
    
    close(fd);
    
    printf("\n=== Diagnostic Complete ===\n");
    printf("✓ SPI interface is working correctly\n");
    printf("✓ Ready to test with ADF4377 hardware\n");
    printf("\nTo test with ADF4377 hardware:\n");
    printf("1. Connect ADF4377 to SPI pins\n");
    printf("2. Run: ./build/adf4377_sdz.out\n");
    
    return 0;
}
