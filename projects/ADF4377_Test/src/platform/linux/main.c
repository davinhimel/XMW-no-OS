/***************************************************************************//**
 *   @file   main.c
 *   @brief  Main file for ADF4377_Test project on Linux platform (Raspberry Pi)
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

#include "parameters.h"
#include "common_data.h"
#include "no_os_error.h"
#include "no_os_print_log.h"
#include "adf4377.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Main function for ADF4377_Test
 * 
 * This program:
 * 1. Initializes SPI communication via Linux spidev (/dev/spidev0.0)
 * 2. Initializes the ADF4377 synthesizer chip
 * 3. Writes to REG001D (0x1D) to set MUXOUT bits [7:4] to 1000 (HIGH output)
 * 
 * @return 0 on success, negative error code on failure
 */
int main(int argc, char *argv[])
{
	int32_t ret;
	struct adf4377_dev *adf4377_dev;
	uint8_t read_back;
	
	printf("========================================\n");
	printf("ADF4377 Test Program for Raspberry Pi\n");
	printf("========================================\n\n");
	
	/* Parse command line arguments for SPI device configuration */
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--spi-device") == 0 && i + 1 < argc) {
			adf4377_spi_extra.device_id = atoi(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "--spi-cs") == 0 && i + 1 < argc) {
			adf4377_spi_extra.chip_select = atoi(argv[i + 1]);
			adf4377_spi_ip.chip_select = adf4377_spi_extra.chip_select;
			i++;
		} else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			printf("Usage: %s [options]\n", argv[0]);
			printf("Options:\n");
			printf("  --spi-device <id>  SPI device ID (default: 0 for /dev/spidev0.X)\n");
			printf("  --spi-cs <cs>      SPI chip select (default: 0)\n");
			printf("  --help, -h         Show this help message\n");
			return 0;
		}
	}
	
	printf("SPI Configuration:\n");
	printf("  Device: /dev/spidev%d.%d\n", 
	       adf4377_spi_extra.device_id, adf4377_spi_extra.chip_select);
	printf("  Speed: %d Hz\n", adf4377_spi_ip.max_speed_hz);
	printf("  Mode: %d (CPOL=0, CPHA=0)\n\n", adf4377_spi_ip.mode);
	
	/* Step 1: Initialize ADF4377 device */
	printf("Step 1: Initializing ADF4377...\n");
	ret = adf4377_init(&adf4377_dev, &adf4377_ip);
	if (ret < 0) {
		printf("ERROR: Failed to initialize ADF4377 (error: %d)\n", ret);
		printf("Please check:\n");
		printf("  - SPI device exists: /dev/spidev%d.%d\n", 
		       adf4377_spi_extra.device_id, adf4377_spi_extra.chip_select);
		printf("  - SPI is enabled (raspi-config -> Interface Options -> SPI)\n");
		printf("  - You have permissions (may need sudo or add user to spi group)\n");
		printf("  - Hardware connections are correct\n");
		return ret;
	}
	printf("  ✓ ADF4377 initialized successfully\n\n");
	
	/* Step 2: Read current value of REG001D */
	printf("Step 2: Reading REG001D (MUXOUT register)...\n");
	ret = adf4377_spi_read(adf4377_dev, ADF4377_REG(0x1D), &read_back);
	if (ret < 0) {
		printf("ERROR: Failed to read REG001D (error: %d)\n", ret);
		adf4377_remove(adf4377_dev);
		return ret;
	}
	printf("  Current REG001D value: 0x%02X\n", read_back);
	printf("  MUXOUT bits [7:4]: 0x%01X\n\n", (read_back >> 4) & 0x0F);
	
	/* Step 3: Write to REG001D to set MUXOUT bits [7:4] to 1000 (HIGH) */
	printf("Step 3: Writing to REG001D to set MUXOUT bits [7:4] to 1000 (HIGH)...\n");
	
	/* Use adf4377_spi_update_bit to update only the MUXOUT field */
	/* ADF4377_MUXOUT_HIGH = 0x8, which sets bits [7:4] = 1000 binary */
	ret = adf4377_spi_update_bit(adf4377_dev, 
	                              ADF4377_REG(0x1D), 
	                              ADF4377_MUXOUT_MSK, 
	                              ADF4377_MUXOUT(ADF4377_MUXOUT_HIGH));
	if (ret < 0) {
		printf("ERROR: Failed to write to REG001D (error: %d)\n", ret);
		adf4377_remove(adf4377_dev);
		return ret;
	}
	printf("  ✓ Successfully wrote MUXOUT = HIGH (0x8) to bits [7:4]\n\n");
	
	/* Step 4: Read back REG001D to verify the write */
	printf("Step 4: Verifying write by reading REG001D...\n");
	ret = adf4377_spi_read(adf4377_dev, ADF4377_REG(0x1D), &read_back);
	if (ret < 0) {
		printf("ERROR: Failed to read REG001D (error: %d)\n", ret);
		adf4377_remove(adf4377_dev);
		return ret;
	}
	printf("  REG001D value after write: 0x%02X\n", read_back);
	printf("  MUXOUT bits [7:4]: 0x%01X", (read_back >> 4) & 0x0F);
	
	/* Verify the MUXOUT bits are set correctly */
	if (((read_back >> 4) & 0x0F) == ADF4377_MUXOUT_HIGH) {
		printf(" ✓ (HIGH - correct!)\n");
	} else {
		printf(" ✗ (Expected 0x8, got 0x%01X)\n", (read_back >> 4) & 0x0F);
		ret = -1;
	}
	
	/* Cleanup */
	printf("\nStep 5: Cleaning up...\n");
	ret = adf4377_remove(adf4377_dev);
	if (ret < 0) {
		printf("WARNING: Error during cleanup (error: %d)\n", ret);
	} else {
		printf("  ✓ Cleanup successful\n");
	}
	
	printf("\n========================================\n");
	if (ret == 0) {
		printf("Test completed successfully!\n");
		printf("MUXOUT pin should now output HIGH signal\n");
	} else {
		printf("Test completed with errors\n");
	}
	printf("========================================\n");
	
	return ret;
}

