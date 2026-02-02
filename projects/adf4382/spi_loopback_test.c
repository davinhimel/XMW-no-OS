/***************************************************************************//**
 *   @file   spi_loopback_test.c
 *   @brief  Simple SPI loopback test - connect MOSI to MISO on Pi
 *   @author User
********************************************************************************
 * Simple test to verify SPI read/write functionality by connecting
 * MOSI directly to MISO on the Raspberry Pi.
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
#define BUFFER_SIZE 32

int main() {
    int fd;
    int ret;
    uint8_t tx_buffer[BUFFER_SIZE];
    uint8_t rx_buffer[BUFFER_SIZE];
    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = 1000000; // 1 MHz
    
    printf("=== SPI Loopback Test ===\n");
    printf("Connect MOSI to MISO on your Pi for this test\n");
    printf("SPI Device: %s\n", SPI_DEVICE);
    printf("Mode: %d, Bits: %d, Speed: %d Hz\n", mode, bits, speed);
    printf("\n");
    
    // Open SPI device
    fd = open(SPI_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
        return -1;
    }
    printf("✓ SPI device opened successfully\n");
    
    // Set SPI mode
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret < 0) {
        perror("Failed to set SPI mode");
        close(fd);
        return -1;
    }
    printf("✓ SPI mode set to %d\n", mode);
    
    // Set bits per word
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret < 0) {
        perror("Failed to set bits per word");
        close(fd);
        return -1;
    }
    printf("✓ Bits per word set to %d\n", bits);
    
    // Set max speed
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret < 0) {
        perror("Failed to set max speed");
        close(fd);
        return -1;
    }
    printf("✓ Max speed set to %d Hz\n", speed);
    
    printf("\n=== Running Loopback Tests ===\n");
    
    // Test 1: Single byte
    printf("\nTest 1: Single byte test\n");
    tx_buffer[0] = 0x5A;
    rx_buffer[0] = 0x00;
    
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = 1,
        .speed_hz = speed,
        .bits_per_word = bits,
    };
    
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if (ret < 0) {
        perror("SPI transfer failed");
        close(fd);
        return -1;
    }
    
    printf("  Sent: 0x%02X, Received: 0x%02X", tx_buffer[0], rx_buffer[0]);
    if (tx_buffer[0] == rx_buffer[0]) {
        printf(" ✓ PASS\n");
    } else {
        printf(" ✗ FAIL\n");
    }
    
    // Test 2: Multiple bytes
    printf("\nTest 2: Multiple bytes test\n");
    for (int i = 0; i < 8; i++) {
        tx_buffer[i] = 0x10 + i; // 0x10, 0x11, 0x12, etc.
    }
    memset(rx_buffer, 0, 8);
    
    transfer.len = 8;
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if (ret < 0) {
        perror("SPI transfer failed");
        close(fd);
        return -1;
    }
    
    printf("  Sent:    ");
    for (int i = 0; i < 8; i++) {
        printf("0x%02X ", tx_buffer[i]);
    }
    printf("\n  Received: ");
    for (int i = 0; i < 8; i++) {
        printf("0x%02X ", rx_buffer[i]);
    }
    
    int all_match = 1;
    for (int i = 0; i < 8; i++) {
        if (tx_buffer[i] != rx_buffer[i]) {
            all_match = 0;
            break;
        }
    }
    
    if (all_match) {
        printf(" ✓ PASS\n");
    } else {
        printf(" ✗ FAIL\n");
    }
    
    // Test 3: ADF4382-like register write/read
    printf("\nTest 3: ADF4382-like register test\n");
    // Simulate ADF4382 register write (16-bit address + 8-bit data)
    uint8_t adf_tx[3] = {0x00, 0x0A, 0x5A}; // Write 0x5A to register 0x000A
    uint8_t adf_rx[3] = {0x00, 0x00, 0x00};
    
    transfer.tx_buf = (unsigned long)adf_tx;
    transfer.rx_buf = (unsigned long)adf_rx;
    transfer.len = 3;
    
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if (ret < 0) {
        perror("SPI transfer failed");
        close(fd);
        return -1;
    }
    
    printf("  ADF4382 Write: 0x%02X%02X%02X", adf_tx[0], adf_tx[1], adf_tx[2]);
    printf(" → Received: 0x%02X%02X%02X", adf_rx[0], adf_rx[1], adf_rx[2]);
    
    if (adf_tx[0] == adf_rx[0] && adf_tx[1] == adf_rx[1] && adf_tx[2] == adf_rx[2]) {
        printf(" ✓ PASS\n");
    } else {
        printf(" ✗ FAIL\n");
    }
    
    close(fd);
    
    printf("\n=== Test Complete ===\n");
    printf("If all tests pass, your Pi's SPI is working correctly.\n");
    printf("If tests fail, check your MOSI to MISO connection.\n");
    
    return 0;
}
