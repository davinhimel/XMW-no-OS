/***************************************************************************//**
 *   @file   adf4377_test.c
 *   @brief  Example: Using ADF4377 driver with libadnoos
 *   @author libadnoos Framework
 *
 *   This example demonstrates how to use the ADF4377 driver from a C program
 *   that links against libadnoos.so and libadf4377.so.
 *
 *   Build:
 *     gcc -o adf4377_test adf4377_test.c \
 *         -L../drivers_so -ladf4377 \
 *         -L../core/build -ladnoos \
 *         -Wl,-rpath,../drivers_so:../core/build
 *
 *   Run:
 *     LD_LIBRARY_PATH=../drivers_so:../core/build ./adf4377_test
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* Include driver header - this will pull in all necessary no-OS headers */
#include "adf4377.h"

/* Include Linux platform headers */
#include "linux_spi.h"
#include "linux_gpio.h"

/* SPI Configuration for Raspberry Pi */
#define SPI_DEVICE_ID   0        /* /dev/spidev0.0 */
#define SPI_CS          0        /* Chip Select 0 */
#define SPI_BAUDRATE    2000000  /* 2 MHz */
#define SPI_MODE        SPI_MODE_0

/* ADF4377 Configuration */
#define CLKIN_FREQ      125000000ULL  /* 125 MHz reference */
#define OUTPUT_FREQ     11000000000ULL /* 11 GHz output */

int main(int argc, char *argv[])
{
    int32_t ret;
    struct adf4377_dev *dev = NULL;
    struct adf4377_init_param init_param;
    struct no_os_spi_init_param spi_init_param;
    struct linux_spi_init_param linux_spi_extra;
    
    printf("========================================\n");
    printf("libadnoos ADF4377 Test\n");
    printf("========================================\n\n");
    
    /* Setup Linux SPI extra parameters */
    linux_spi_extra.device_id = SPI_DEVICE_ID;
    linux_spi_extra.chip_select = SPI_CS;
    linux_spi_extra.max_speed_hz = SPI_BAUDRATE;
    linux_spi_extra.mode = SPI_MODE;
    
    /* Setup no-OS SPI init parameters */
    spi_init_param.device_id = SPI_DEVICE_ID;
    spi_init_param.max_speed_hz = SPI_BAUDRATE;
    spi_init_param.chip_select = SPI_CS;
    spi_init_param.mode = NO_OS_SPI_MODE_0;
    spi_init_param.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST;
    spi_init_param.platform_ops = &linux_spi_ops;
    spi_init_param.extra = &linux_spi_extra;
    
    /* Setup ADF4377 init parameters */
    init_param.dev_id = ADF4377;
    init_param.spi_init = &spi_init_param;
    init_param.spi4wire = true;
    init_param.gpio_ce_param = NULL;      /* Optional */
    init_param.gpio_enclk1_param = NULL;  /* Optional */
    init_param.gpio_enclk2_param = NULL;  /* Optional */
    init_param.clkin_freq = CLKIN_FREQ;
    init_param.ref_doubler_en = 1;
    init_param.f_clk = OUTPUT_FREQ;
    init_param.ref_div_factor = 1;
    init_param.muxout_select = ADF4377_MUXOUT_HIGH_Z;
    init_param.cp_i = ADF4377_CP_10MA1;
    init_param.clkout_op = ADF4377_CLKOUT_640MV;
    
    /* Initialize ADF4377 */
    printf("Step 1: Initializing ADF4377...\n");
    ret = adf4377_init(&dev, &init_param);
    if (ret < 0) {
        printf("ERROR: Failed to initialize ADF4377 (error: %d)\n", ret);
        printf("Please check:\n");
        printf("  - SPI device exists: /dev/spidev%d.%d\n", SPI_DEVICE_ID, SPI_CS);
        printf("  - SPI is enabled (raspi-config -> Interface Options -> SPI)\n");
        printf("  - You have permissions (may need sudo)\n");
        printf("  - Hardware connections are correct\n");
        return ret;
    }
    printf("  ✓ ADF4377 initialized successfully\n\n");
    
    /* Set output frequency */
    printf("Step 2: Setting output frequency to %.3f GHz...\n", 
           OUTPUT_FREQ / 1e9);
    ret = adf4377_set_rfout(dev, OUTPUT_FREQ);
    if (ret < 0) {
        printf("ERROR: Failed to set frequency (error: %d)\n", ret);
        adf4377_remove(dev);
        return ret;
    }
    printf("  ✓ Frequency set successfully\n\n");
    
    /* Read back frequency */
    printf("Step 3: Reading back frequency...\n");
    uint64_t read_freq = 0;
    ret = adf4377_get_rfout(dev, &read_freq);
    if (ret < 0) {
        printf("ERROR: Failed to read frequency (error: %d)\n", ret);
    } else {
        printf("  Current frequency: %.3f GHz\n", read_freq / 1e9);
    }
    
    /* Write to REG001D to set MUXOUT to HIGH */
    printf("\nStep 4: Setting MUXOUT to HIGH...\n");
    ret = adf4377_spi_update_bit(dev, 
                                  ADF4377_REG(0x1D), 
                                  ADF4377_MUXOUT_MSK, 
                                  ADF4377_MUXOUT(ADF4377_MUXOUT_HIGH));
    if (ret < 0) {
        printf("ERROR: Failed to set MUXOUT (error: %d)\n", ret);
    } else {
        printf("  ✓ MUXOUT set to HIGH\n");
    }
    
    /* Cleanup */
    printf("\nStep 5: Cleaning up...\n");
    ret = adf4377_remove(dev);
    if (ret < 0) {
        printf("WARNING: Error during cleanup (error: %d)\n", ret);
    } else {
        printf("  ✓ Cleanup successful\n");
    }
    
    printf("\n========================================\n");
    printf("Test completed successfully!\n");
    printf("========================================\n");
    
    return 0;
}

