/***************************************************************************//**
 *   @file   main.c
 *   @brief  Main file for Linux platform of ADF4368 project.
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
	
	/* Parse command line arguments for help */
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			printf("ADF4368 Linux Platform\n");
			printf("Usage: %s [options]\n", argv[0]);
			printf("Options:\n");
			printf("  --help, -h           Show this help message\n");
			return 0;
		}
	}
	
	printf("ADF4368 Linux Platform - Starting...\n");
	printf("SPI Device: /dev/spidev0.0\n");
	printf("UART Device: /dev/ttyUSB0\n");
	
	ret = example_main();
	
	if (ret == 0) {
		printf("ADF4368 Linux Platform - Completed successfully\n");
	} else {
		printf("ADF4368 Linux Platform - Failed with error: %d\n", ret);
	}
	
	return ret;
}