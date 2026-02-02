/***************************************************************************//**
 *   @file   basic_example2.c
 *   @brief  Basic example eval-adf4382 project - without scratchpad test
 *   @author CHegbeli (ciprian.hegbeli@analog.com)
********************************************************************************
 * Copyright 2022(c) Analog Devices, Inc.
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

#include "common_data.h"
#include "no_os_delay.h"
#include "no_os_print_log.h"

/**
 * @brief Basic example main execution.
 *
 * @return ret - Result of the example execution. If working correctly, will
 *               execute continuously the while(1) loop and will not return.
 */
int example_main()
{
	struct adf4382_dev *dev;
	int ret;

	pr_info("Enter basic example 2 (no scratchpad test) \n");

	/* UART logging disabled - using printf for output instead */
	/* struct no_os_uart_desc *uart_desc; */
	/* ret = no_os_uart_init(&uart_desc, &adf4382_uart_ip); */
	/* if (ret) */
	/* 	return ret; */
	/* no_os_uart_stdio(uart_desc); */

	pr_info("Initializing ADF4382 device...\n");
	ret = adf4382_init(&dev, &adf4382_ip);
	if (ret) {
		pr_info("ADF4382 initialization failed with error: %d\n", ret);
		goto error;
	}
	pr_info("ADF4382 initialized successfully!\n");

	pr_info("Setting MUXOUT to high signal...\n");
	ret = adf4382_spi_write(dev, 0x2E, 0x8);  // Set MUXOUT high
	if (ret) {
		pr_info("Failed to set MUXOUT high: %d\n", ret);
		goto remove_adf4382;
	}
	pr_info("MUXOUT set to high successfully!\n");

	pr_info("ADF4382 configuration completed successfully!\n");
	pr_info("Example completed successfully!\n");

remove_adf4382:
	pr_info("Cleaning up ADF4382 device...\n");
	adf4382_remove(dev);
error:
	if (ret) {
		pr_info("Example completed with error: %d\n", ret);
	} else {
		pr_info("Example completed successfully\n");
	}
	return ret;
}
