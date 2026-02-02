#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <string.h>

int main() {
    int fd;
    int ret;
    unsigned char tx_buf[3];
    unsigned char rx_buf[3];
    struct spi_ioc_transfer tr;
    
    printf("ADF4382 Connection Test\n");
    printf("======================\n\n");
    
    // Open SPI device
    fd = open("/dev/spidev0.0", O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
        return 1;
    }
    
    // Configure SPI
    int mode = SPI_MODE_0;
    int bits = 8;
    int speed = 1000000; // Slower speed for testing
    
    ioctl(fd, SPI_IOC_WR_MODE, &mode);
    ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    
    // Zero out transfer structure
    memset(&tr, 0, sizeof(tr));
    tr.speed_hz = speed;
    tr.bits_per_word = bits;
    tr.len = 3;
    
    printf("Testing different SPI speeds and patterns...\n\n");
    
    // Test 1: Very slow speed
    printf("Test 1: Very slow speed (100 kHz)\n");
    tr.speed_hz = 100000;
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &tr.speed_hz);
    
    tx_buf[0] = 0x00; tx_buf[1] = 0x00; tx_buf[2] = 0x00;
    tr.tx_buf = (unsigned long)tx_buf;
    tr.rx_buf = (unsigned long)rx_buf;
    
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    printf("Result: %s (Response: %02x %02x %02x)\n", 
           ret < 0 ? "FAILED" : "OK", rx_buf[0], rx_buf[1], rx_buf[2]);
    
    // Test 2: Different data patterns
    printf("\nTest 2: Different data patterns\n");
    tr.speed_hz = 1000000;
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &tr.speed_hz);
    
    for (int i = 0; i < 5; i++) {
        tx_buf[0] = 0x00;
        tx_buf[1] = 0x80 | i; // Different patterns
        tx_buf[2] = 0x00;
        
        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        printf("Pattern %d: %s (Response: %02x %02x %02x)\n", 
               i, ret < 0 ? "FAILED" : "OK", rx_buf[0], rx_buf[1], rx_buf[2]);
    }
    
    // Test 3: ADF4382 specific commands
    printf("\nTest 3: ADF4382 specific commands\n");
    
    // Reset command
    tx_buf[0] = 0x00; tx_buf[1] = 0x81; tx_buf[2] = 0x00;
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    printf("Reset command: %s (Response: %02x %02x %02x)\n", 
           ret < 0 ? "FAILED" : "OK", rx_buf[0], rx_buf[1], rx_buf[2]);
    
    // Read register 0
    tx_buf[0] = 0x00; tx_buf[1] = 0x00; tx_buf[2] = 0x00;
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    printf("Read reg 0: %s (Response: %02x %02x %02x)\n", 
           ret < 0 ? "FAILED" : "OK", rx_buf[0], rx_buf[1], rx_buf[2]);
    
    close(fd);
    
    printf("\nDiagnosis:\n");
    printf("==========\n");
    printf("If all responses are 00 00 00, the ADF4382 is not responding.\n");
    printf("This usually means:\n");
    printf("1. No power to the chip\n");
    printf("2. Wrong connections\n");
    printf("3. Chip not enabled\n");
    printf("4. Missing reference clock\n");
    printf("5. Chip is damaged\n");
    printf("\nCheck your connections and power supply!\n");
    
    return 0;
}
