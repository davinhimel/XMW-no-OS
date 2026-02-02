/***************************************************************************//**
 *   @file   simple_chip_id_test.c
 *   @brief  Simple test to read ADF4382 chip ID
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
    
    printf("=== ADF4382 Chip ID Test ===\n");
    printf("This will try to read the chip ID register (0x0003)\n");
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
    
    // Test 1: Reset chip
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
    
    // Test 2: Read chip ID register (0x0003)
    printf("\n2. Reading chip ID register (0x0003)...\n");
    tx_buffer[0] = 0x80; tx_buffer[1] = 0x03; tx_buffer[2] = 0x00; // Read from 0x0003
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
    
    // Test 3: Read vendor ID register (0x000C)
    printf("\n3. Reading vendor ID register (0x000C)...\n");
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
    
    // Test 4: Read product ID register (0x0004)
    printf("\n4. Reading product ID register (0x0004)...\n");
    tx_buffer[0] = 0x80; tx_buffer[1] = 0x04; tx_buffer[2] = 0x00; // Read from 0x0004
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
    
    close(fd);
    
    printf("\n=== Test Complete ===\n");
    printf("If all received values are 0x00, the chip is not responding.\n");
    printf("Check:\n");
    printf("1. Power supply (3.3V at chip)\n");
    printf("2. SDO connection (ADF4382 SDO → Pi MISO Pin 21)\n");
    printf("3. Chip is not damaged\n");
    
    return 0;
}
