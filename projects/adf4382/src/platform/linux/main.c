/***************************************************************************//**
 *   @file   main.c
 *   @brief  Main file for Linux platform of ADF4382 project.
 *   @author CHegbeli (ciprian.hegbeli@analog.com)
********************************************************************************
 * Copyright 2023(c) Analog Devices, Inc.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int example_main();

/**
 * @brief Main function execution for Linux platform.
 * @return ret - Result of the enabled examples execution.
 */
int main(int argc, char *argv[])
{
	int ret;
	
	/* Parse command line arguments for SPI/UART device configuration */
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--spi-device") == 0 && i + 1 < argc) {
			adf4382_spi_extra.device_id = atoi(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "--uart-device") == 0 && i + 1 < argc) {
			adf4382_uart_extra_ip.device_id = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			printf("ADF4382 Linux Platform\n");
			printf("Usage: %s [options]\n", argv[0]);
			printf("Options:\n");
			printf("  --spi-device <id>     SPI device ID (default: 0 for /dev/spidev0.0)\n");
			printf("  --uart-device <path>  UART device path (default: /dev/ttyUSB0)\n");
			printf("  --help, -h           Show this help message\n");
			return 0;
		}
	}
	
	printf("ADF4382 Linux Platform - Starting...\n");
	printf("SPI Device: /dev/spidev%d.%d\n", 
	       adf4382_spi_extra.device_id, adf4382_spi_extra.chip_select);
	printf("UART Device: %s\n", adf4382_uart_extra_ip.device_id);
	
	ret = example_main();
	
	if (ret == 0) {
		printf("ADF4382 Linux Platform - Completed successfully\n");
	} else {
		printf("ADF4382 Linux Platform - Failed with error: %d\n", ret);
	}
	
	return ret;
} 