/***************************************************************************//**
 *   @file   main.c
 *   @brief  Main file for ADF4382_Test project on Linux platform (Raspberry Pi)
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
#include "adf4382.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Main function for ADF4382_Test
 * 
 * This program:
 * 1. Initializes SPI communication via Linux spidev (/dev/spidev0.0)
 * 2. Initializes the ADF4382 synthesizer chip
 * 3. Reads and writes to REG000A (scratchpad register) to verify communication
 * 4. Reads the current output frequency configuration
 * 
 * @return 0 on success, negative error code on failure
 */
int main(int argc, char *argv[])
{
	int32_t ret;
	struct adf4382_dev *adf4382_dev;
	uint8_t read_back;
	uint64_t freq_out;
	
	printf("========================================\n");
	printf("ADF4382 Test Program for Raspberry Pi\n");
	printf("========================================\n\n");
	
	/* Parse command line arguments for SPI device configuration */
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--spi-device") == 0 && i + 1 < argc) {
			adf4382_spi_extra.device_id = atoi(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "--spi-cs") == 0 && i + 1 < argc) {
			adf4382_spi_extra.chip_select = atoi(argv[i + 1]);
			adf4382_spi_ip.chip_select = adf4382_spi_extra.chip_select;
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
	       adf4382_spi_extra.device_id, adf4382_spi_extra.chip_select);
	printf("  Speed: %d Hz\n", adf4382_spi_ip.max_speed_hz);
	printf("  Mode: %d (CPOL=0, CPHA=0)\n\n", adf4382_spi_ip.mode);
	
	/* Step 1: Initialize ADF4382 device */
	printf("Step 1: Initializing ADF4382...\n");
	ret = adf4382_init(&adf4382_dev, &adf4382_ip);
	if (ret < 0) {
		printf("ERROR: Failed to initialize ADF4382 (error: %d)\n", ret);
		printf("Please check:\n");
		printf("  - SPI device exists: /dev/spidev%d.%d\n", 
		       adf4382_spi_extra.device_id, adf4382_spi_extra.chip_select);
		printf("  - SPI is enabled (raspi-config -> Interface Options -> SPI)\n");
		printf("  - You have permissions (may need sudo or add user to spi group)\n");
		printf("  - Hardware connections are correct\n");
		return ret;
	}
	printf("  ✓ ADF4382 initialized successfully\n\n");
	
	/* Step 2: Read current value of REG000A (scratchpad register) */
	printf("Step 2: Reading REG000A (scratchpad register)...\n");
	ret = adf4382_spi_read(adf4382_dev, 0x0A, &read_back);
	if (ret < 0) {
		printf("ERROR: Failed to read REG000A (error: %d)\n", ret);
		adf4382_remove(adf4382_dev);
		return ret;
	}
	printf("  Current REG000A value: 0x%02X\n\n", read_back);
	
	/* Step 3: Write test value to REG000A (scratchpad register) */
	printf("Step 3: Writing test value 0xAA to REG000A (scratchpad register)...\n");
	ret = adf4382_spi_write(adf4382_dev, 0x0A, 0xAA);
	if (ret < 0) {
		printf("ERROR: Failed to write to REG000A (error: %d)\n", ret);
		adf4382_remove(adf4382_dev);
		return ret;
	}
	printf("  ✓ Successfully wrote 0xAA to REG000A\n\n");
	
	/* Step 4: Read back REG000A to verify the write */
	printf("Step 4: Verifying write by reading REG000A...\n");
	ret = adf4382_spi_read(adf4382_dev, 0x0A, &read_back);
	if (ret < 0) {
		printf("ERROR: Failed to read REG000A (error: %d)\n", ret);
		adf4382_remove(adf4382_dev);
		return ret;
	}
	printf("  REG000A value after write: 0x%02X", read_back);
	
	/* Verify the scratchpad register value */
	if (read_back == 0xAA) {
		printf(" ✓ (correct!)\n");
	} else {
		printf(" ✗ (Expected 0xAA, got 0x%02X)\n", read_back);
		ret = -1;
	}
	
	/* Step 5: Read current output frequency */
	printf("\nStep 5: Reading current output frequency configuration...\n");
	ret = adf4382_get_rfout(adf4382_dev, &freq_out);
	if (ret < 0) {
		printf("ERROR: Failed to read output frequency (error: %d)\n", ret);
		adf4382_remove(adf4382_dev);
		return ret;
	}
	printf("  Current output frequency: %llu Hz (%.3f GHz)\n", 
	       (unsigned long long)freq_out, freq_out / 1e9);
	
	/* Step 6: Read reference clock configuration */
	printf("\nStep 6: Reading reference clock configuration...\n");
	uint64_t ref_clk;
	ret = adf4382_get_ref_clk(adf4382_dev, &ref_clk);
	if (ret < 0) {
		printf("ERROR: Failed to read reference clock (error: %d)\n", ret);
		adf4382_remove(adf4382_dev);
		return ret;
	}
	printf("  Reference clock: %llu Hz (%.3f MHz)\n", 
	       (unsigned long long)ref_clk, ref_clk / 1e6);
	
	/* Cleanup */
	printf("\nStep 7: Cleaning up...\n");
	ret = adf4382_remove(adf4382_dev);
	if (ret < 0) {
		printf("WARNING: Error during cleanup (error: %d)\n", ret);
	} else {
		printf("  ✓ Cleanup successful\n");
	}
	
	printf("\n========================================\n");
	if (ret == 0) {
		printf("Test completed successfully!\n");
		printf("ADF4382 communication verified\n");
	} else {
		printf("Test completed with errors\n");
	}
	printf("========================================\n");
	
	return ret;
}

