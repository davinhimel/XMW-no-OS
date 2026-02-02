/***************************************************************************//**
 *   @file   vendor_id_test.c
 *   @brief  Simple test to read ADF4382 vendor ID register
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
    
    printf("=== ADF4382 Vendor ID Test ===\n");
    printf("Reading vendor ID register (0x000D) - should return 0x04\n");
    printf("SPI Device: %s\n", SPI_DEVICE);
    printf("Mode: %d, Bits: %d, Speed: %d Hz\n\n", mode, bits, speed);
    
    // Open SPI device
    fd = open(SPI_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
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
    
    printf("✓ SPI configured\n");
    
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
    
    // Test 2: Configure CMOS output to 3.3V
    printf("\n2. Configuring CMOS output to 3.3V...\n");
    tx_buffer[0] = 0x00; tx_buffer[1] = 0x3D; tx_buffer[2] = 0x20; // Set CMOS_OV bit
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
    
    // Test 3: Configure SPI to 4-wire mode
    printf("\n3. Configuring SPI to 4-wire mode...\n");
    tx_buffer[0] = 0x00; tx_buffer[1] = 0x00; tx_buffer[2] = 0x08; // Set SDO_ACTIVE bit
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
    
    // Test 4: Read vendor ID register (0x000D)
    printf("\n4. Reading vendor ID register (0x000D)...\n");
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
    
    // Check if we got the expected vendor ID
    if (rx_buffer[2] == 0x04) {
        printf("   ✓ SUCCESS! Got expected vendor ID: 0x%02X\n", rx_buffer[2]);
    } else if (rx_buffer[0] != 0 || rx_buffer[1] != 0 || rx_buffer[2] != 0) {
        printf("   ⚠ PARTIAL SUCCESS! Got some response: 0x%02X%02X%02X\n", 
               rx_buffer[0], rx_buffer[1], rx_buffer[2]);
        printf("   This means the chip is responding but might be in wrong mode\n");
    } else {
        printf("   ✗ FAILURE! No response from chip (all zeros)\n");
    }
    
    // Test 5: Try reading vendor ID LSB register (0x000C)
    printf("\n5. Reading vendor ID LSB register (0x000C)...\n");
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
    
    if (rx_buffer[2] == 0x56) {
        printf("   ✓ SUCCESS! Got expected vendor ID LSB: 0x%02X\n", rx_buffer[2]);
    } else if (rx_buffer[0] != 0 || rx_buffer[1] != 0 || rx_buffer[2] != 0) {
        printf("   ⚠ PARTIAL SUCCESS! Got some response: 0x%02X%02X%02X\n", 
               rx_buffer[0], rx_buffer[1], rx_buffer[2]);
    } else {
        printf("   ✗ FAILURE! No response from chip (all zeros)\n");
    }
    
    close(fd);
    
    printf("\n=== Test Complete ===\n");
    printf("If you got 0x04 and 0x56, the chip is working!\n");
    printf("If you got all zeros, check power supply and SDO connection.\n");
    
    return 0;
}
