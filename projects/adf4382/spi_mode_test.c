/***************************************************************************//**
 *   @file   spi_mode_test.c
 *   @brief  Test different SPI modes with ADF4382
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

int test_spi_mode(uint8_t mode, const char* mode_name) {
    int fd;
    int ret;
    uint8_t tx_buffer[3] = {0x00, 0x0A, 0x5A}; // Write 0x5A to register 0x000A
    uint8_t rx_buffer[3] = {0x00, 0x00, 0x00};
    uint8_t bits = 8;
    uint32_t speed = 1000000; // 1 MHz
    
    printf("\n=== Testing SPI Mode %d (%s) ===\n", mode, mode_name);
    
    // Open SPI device
    fd = open(SPI_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
        return -1;
    }
    
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
    
    // Test 1: Reset command
    printf("1. Sending reset command...\n");
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
    
    // Wait a bit
    usleep(10000); // 10ms
    
    // Test 2: Configure SPI mode
    printf("2. Configuring SPI to 4-wire mode...\n");
    tx_buffer[0] = 0x00; tx_buffer[1] = 0x00; tx_buffer[2] = 0x00; // 4-wire mode
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
    
    // Test 3: Scratchpad test
    printf("3. Testing scratchpad register...\n");
    tx_buffer[0] = 0x00; tx_buffer[1] = 0x0A; tx_buffer[2] = 0x5A; // Write 0x5A to 0x000A
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
    
    // Test 4: Read scratchpad
    printf("4. Reading scratchpad register...\n");
    tx_buffer[0] = 0x80; tx_buffer[1] = 0x0A; tx_buffer[2] = 0x00; // Read from 0x000A
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
    
    // Check if we got any non-zero response
    if (rx_buffer[0] != 0 || rx_buffer[1] != 0 || rx_buffer[2] != 0) {
        printf("   *** GOT RESPONSE! ***\n");
        return 1;
    } else {
        printf("   No response from chip\n");
        return 0;
    }
}

int main() {
    printf("=== ADF4382 SPI Mode Test ===\n");
    printf("This will test different SPI modes to see if the chip responds\n");
    printf("Watch your oscilloscope for activity on the SDO line\n\n");
    
    int found_response = 0;
    
    // Test all SPI modes
    if (test_spi_mode(SPI_MODE_0, "Mode 0: CPOL=0, CPHA=0")) {
        found_response = 1;
    }
    
    if (test_spi_mode(SPI_MODE_1, "Mode 1: CPOL=0, CPHA=1")) {
        found_response = 1;
    }
    
    if (test_spi_mode(SPI_MODE_2, "Mode 2: CPOL=1, CPHA=0")) {
        found_response = 1;
    }
    
    if (test_spi_mode(SPI_MODE_3, "Mode 3: CPOL=1, CPHA=1")) {
        found_response = 1;
    }
    
    printf("\n=== Test Complete ===\n");
    if (found_response) {
        printf("SUCCESS: Found a mode where the chip responds!\n");
    } else {
        printf("FAILURE: No response from chip in any SPI mode\n");
        printf("Check:\n");
        printf("1. Power supply (3.3V at chip)\n");
        printf("2. SDO connection\n");
        printf("3. Chip is not damaged\n");
    }
    
    return 0;
}
