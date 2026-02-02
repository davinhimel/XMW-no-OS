#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <errno.h>

int main() {
    int fd;
    int ret;
    unsigned char tx_buf[3] = {0x00, 0x00, 0x00};
    unsigned char rx_buf[3] = {0x00, 0x00, 0x00};
    struct spi_ioc_transfer tr;
    
    printf("ADF4382 SPI Diagnostic Tool\n");
    printf("==========================\n\n");
    
    // Zero out the transfer structure
    memset(&tr, 0, sizeof(tr));
    
    printf("1. Testing SPI device access...\n");
    
    // Open SPI device
    fd = open("/dev/spidev0.0", O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
        printf("Error: Cannot open /dev/spidev0.0\n");
        printf("Make sure SPI is enabled: sudo raspi-config -> Interface Options -> SPI\n");
        return 1;
    }
    
    printf("✓ SPI device opened successfully\n");
    
    printf("\n2. Testing SPI configuration...\n");
    
    // Set SPI mode
    int mode = SPI_MODE_0;
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret < 0) {
        perror("Failed to set SPI mode");
        close(fd);
        return 1;
    }
    printf("✓ SPI mode set to 0\n");
    
    // Set bits per word
    int bits = 8;
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret < 0) {
        perror("Failed to set bits per word");
        close(fd);
        return 1;
    }
    printf("✓ Bits per word set to 8\n");
    
    // Set max speed
    int speed = 1500000;
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret < 0) {
        perror("Failed to set max speed");
        close(fd);
        return 1;
    }
    printf("✓ Max speed set to 1.5 MHz\n");
    
    printf("\n3. Testing basic SPI communication...\n");
    
    // Configure transfer
    tr.tx_buf = (unsigned long)tx_buf;
    tr.rx_buf = (unsigned long)rx_buf;
    tr.len = 3;
    tr.speed_hz = 1500000;
    tr.bits_per_word = 8;
    tr.delay_usecs = 0;
    
    printf("Sending: %02x %02x %02x\n", tx_buf[0], tx_buf[1], tx_buf[2]);
    
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 0) {
        perror("SPI transfer failed");
        printf("Error code: %d (%s)\n", errno, strerror(errno));
        close(fd);
        return 1;
    }
    
    printf("✓ SPI transfer successful\n");
    printf("Received: %02x %02x %02x\n", rx_buf[0], rx_buf[1], rx_buf[2]);
    
    printf("\n4. Testing ADF4382 specific commands...\n");
    
    // Test ADF4382 reset command
    printf("Sending ADF4382 reset command (0x81)...\n");
    tx_buf[0] = 0x00;  // Register address
    tx_buf[1] = 0x81;  // Reset command
    tx_buf[2] = 0x00;  // Padding
    
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 0) {
        perror("ADF4382 reset command failed");
        close(fd);
        return 1;
    }
    
    printf("✓ ADF4382 reset command sent\n");
    printf("Response: %02x %02x %02x\n", rx_buf[0], rx_buf[1], rx_buf[2]);
    
    // Test reading ADF4382 register
    printf("\n5. Testing ADF4382 register read...\n");
    printf("Attempting to read ADF4382 register 0x00...\n");
    
    tx_buf[0] = 0x00;  // Register address
    tx_buf[1] = 0x00;  // Read command
    tx_buf[2] = 0x00;  // Padding
    
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 0) {
        perror("ADF4382 register read failed");
        close(fd);
        return 1;
    }
    
    printf("✓ ADF4382 register read successful\n");
    printf("Register 0x00 value: %02x %02x %02x\n", rx_buf[0], rx_buf[1], rx_buf[2]);
    
    close(fd);
    
    printf("\n6. Hardware Connection Requirements:\n");
    printf("====================================\n");
    printf("For ADF4382 to work, you need:\n");
    printf("1. Power supply: 3.3V or 5V (check ADF4382 datasheet)\n");
    printf("2. Ground connection\n");
    printf("3. SPI connections:\n");
    printf("   - MOSI (Master Out Slave In): Pi GPIO 10 (Pin 19)\n");
    printf("   - MISO (Master In Slave Out): Pi GPIO 9 (Pin 21)\n");
    printf("   - SCLK (Serial Clock): Pi GPIO 11 (Pin 23)\n");
    printf("   - CS (Chip Select): Pi GPIO 8 (Pin 24)\n");
    printf("4. Reference clock input (if required)\n");
    printf("5. Proper decoupling capacitors\n");
    printf("\nNote: ADF4382 is a complex RF chip that may require:\n");
    printf("- Proper RF layout\n");
    printf("- Matching networks\n");
    printf("- Specific reference clock frequency\n");
    printf("- Power supply sequencing\n");
    
    printf("\n✓ SPI diagnostic completed successfully\n");
    printf("If ADF4382 is properly connected and powered, you should see\n");
    printf("non-zero values in the register read response.\n");
    
    return 0;
}
