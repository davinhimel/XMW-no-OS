/***************************************************************************//**
 *   @file   minimal_vendor_id_test.c
 *   @brief  Minimal ADF4377 Vendor ID Test using no-OS framework
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

#include "common_data.h"
#include "no_os_print_log.h"
#include "no_os_error.h"
#include "no_os_alloc.h"

/**
 * @brief Minimal example main execution for ADF4377 vendor ID test.
 *
 * @return ret - Result of the example execution.
 */
int example_main()
{
	struct adf4377_dev *dev;
	int ret;
	uint8_t vendor_id_lsb = 0;
	uint8_t vendor_id_msb = 0;
	uint16_t vendor_id = 0;

	pr_info("=== ADF4377 MINIMAL VENDOR ID TEST ===\n");
	
	// Allocate device structure manually
	dev = (struct adf4377_dev *)no_os_calloc(1, sizeof(*dev));
	if (!dev) {
		pr_info("ERROR: Failed to allocate device structure\n");
		return -ENOMEM;
	}
	pr_info("✓ Device structure allocated: %p\n", dev);
	
	// Initialize SPI manually
	pr_info("Initializing SPI...\n");
	ret = no_os_spi_init(&dev->spi_desc, &adf4377_spi_ip);
	if (ret < 0) {
		pr_info("ERROR: SPI initialization failed with error: %d\n", ret);
		no_os_free(dev);
		return ret;
	}
	pr_info("✓ SPI initialized successfully\n");
	
	// Set basic device parameters
	dev->dev_id = ADF4377;
	dev->spi4wire = true;
	dev->clkin_freq = 125000000;
	dev->ref_doubler_en = 1;
	dev->f_clk = 10000000000;
	dev->ref_div_factor = 1;
	dev->muxout_default = ADF4377_MUXOUT_HIGH_Z;
	dev->cp_i = ADF4377_CP_10MA1;
	
	// Set GPIO pointers to NULL (optional)
	dev->gpio_ce = NULL;
	dev->gpio_enclk1 = NULL;
	dev->gpio_enclk2 = NULL;
	
	pr_info("✓ Device parameters set\n");
	
	// Read vendor ID LSB (Register 0x0C)
	pr_info("Reading vendor ID LSB (0x0C)...\n");
	ret = adf4377_spi_read(dev, 0x0C, &vendor_id_lsb);
	if (ret) {
		pr_info("ERROR: Vendor ID LSB read failed with error: %d\n", ret);
		pr_info("This is expected if no ADF4377 hardware is connected\n");
		no_os_spi_remove(dev->spi_desc);
		no_os_free(dev);
		return ret;
	}
	pr_info("Vendor ID LSB: 0x%02X\n", vendor_id_lsb);
	
	// Read vendor ID MSB (Register 0x0D)
	pr_info("Reading vendor ID MSB (0x0D)...\n");
	ret = adf4377_spi_read(dev, 0x0D, &vendor_id_msb);
	if (ret) {
		pr_info("ERROR: Vendor ID MSB read failed with error: %d\n", ret);
		pr_info("This is expected if no ADF4377 hardware is connected\n");
		no_os_spi_remove(dev->spi_desc);
		no_os_free(dev);
		return ret;
	}
	pr_info("Vendor ID MSB: 0x%02X\n", vendor_id_msb);
	
	// Combine LSB and MSB to get full vendor ID
	vendor_id = (vendor_id_msb << 8) | vendor_id_lsb;
	pr_info("Full Vendor ID: 0x%04X\n", vendor_id);
	
	// Check if vendor ID matches expected value (0x0456 for Analog Devices)
	if (vendor_id == 0x0456) {
		pr_info("✓ SUCCESS: Vendor ID matches expected value (0x0456 - Analog Devices)\n");
	} else {
		pr_info("✗ FAIL: Vendor ID does not match expected value (0x0456)\n");
		pr_info("  Expected: 0x0456, Got: 0x%04X\n", vendor_id);
		pr_info("  This is expected if no ADF4377 hardware is connected\n");
		ret = -1;
	}
	
	// Clean up
	no_os_spi_remove(dev->spi_desc);
	no_os_free(dev);
	
	pr_info("=== TEST COMPLETED ===\n");
	return ret;
}
