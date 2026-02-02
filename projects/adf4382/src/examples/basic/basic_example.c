/***************************************************************************//**
 *   @file   basic_example.c
 *   @brief  Basic example eval-adf4382 project
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
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. “AS IS” AND ANY EXPRESS OR
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
#include "no_os_alloc.h"
#include "no_os_error.h"
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
	bool en = true;
	uint8_t i;

	pr_info("Enter basic example \n");

	/* UART logging disabled - using printf for output instead */
	/* struct no_os_uart_desc *uart_desc; */
	/* ret = no_os_uart_init(&uart_desc, &adf4382_uart_ip); */
	/* if (ret) */
	/* 	return ret; */
	/* no_os_uart_stdio(uart_desc); */

	pr_info("=== MINIMAL ADF4382 INITIALIZATION TEST ===\n");
	
	// Step 1: Allocate device structure
	pr_info("Step 1: Allocating device structure...\n");
	dev = (struct adf4382_dev *)no_os_calloc(1, sizeof(*dev));
	if (!dev) {
		pr_info("ERROR: Failed to allocate device structure\n");
		ret = -ENOMEM;
		goto error;
	}
	pr_info("✓ Device structure allocated successfully\n");
	
	// Step 2: Initialize SPI
	pr_info("Step 2: Initializing SPI interface...\n");
	ret = no_os_spi_init(&dev->spi_desc, adf4382_ip.spi_init);
	if (ret) {
		pr_info("ERROR: SPI initialization failed with error: %d\n", ret);
		goto error_dev;
	}
	pr_info("✓ SPI interface initialized successfully\n");
	
	// Step 3: Set device parameters
	pr_info("Step 3: Setting device parameters...\n");
	dev->spi_3wire_en = adf4382_ip.spi_3wire_en;
	dev->cmos_3v3 = adf4382_ip.cmos_3v3;
	dev->ref_freq_hz = adf4382_ip.ref_freq_hz;
	dev->freq = adf4382_ip.freq;
	dev->ref_doubler_en = adf4382_ip.ref_doubler_en;
	dev->ref_div = adf4382_ip.ref_div;
	dev->cp_i = adf4382_ip.cp_i;
	dev->bleed_word = adf4382_ip.bleed_word;
	dev->ld_count = adf4382_ip.ld_count;
	dev->phase_adj = 0;
	dev->max_lpf_cap_value_uf = adf4382_ip.max_lpf_cap_value_uf;
	pr_info("✓ Device parameters set successfully\n");
	
	// Step 4: Set chip-specific limits
	pr_info("Step 4: Setting chip-specific limits for ID_ADF4382...\n");
	dev->freq_max = ADF4382_RFOUT_MAX;
	dev->freq_min = ADF4382_RFOUT_MIN;
	dev->vco_max = ADF4382_VCO_FREQ_MAX;
	dev->vco_min = ADF4382_VCO_FREQ_MIN;
	dev->clkout_div_reg_val_max = ADF4382_CLKOUT_DIV_REG_VAL_MAX;
	pr_info("✓ Chip limits set successfully\n");
	
	// Step 5: Reset chip
	pr_info("Step 5: Sending reset command to chip...\n");
	ret = adf4382_spi_write(dev, 0x00, ADF4382_RESET_CMD);
	if (ret) {
		pr_info("ERROR: Reset command failed with error: %d\n", ret);
		goto error_spi;
	}
	pr_info("✓ Reset command sent successfully\n");
	
	// Step 6: Wait for power-on reset
	pr_info("Step 6: Waiting for power-on reset delay...\n");
	no_os_udelay(ADF4382_POR_DELAY_US);
	pr_info("✓ Power-on reset delay completed\n");
	
	if (dev->spi_3wire_en)
		en = false;

	// Step 7: Configure SPI mode
	pr_info("Step 7: Configuring SPI to 4-wire mode...\n");
	ret = adf4382_spi_write(dev, 0x00, ADF4382_SPI_3W_CFG(en));
	if (ret) {
		pr_info("ERROR: SPI mode configuration failed with error: %d\n", ret);
		goto error_spi;
	}
	pr_info("✓ SPI configured to 4-wire mode\n");
	
	// Step 8: Configure CMOS output voltage
	pr_info("Step 8: Configuring CMOS output voltage...\n");
	ret = adf4382_spi_write(dev, 0x3D, no_os_field_prep(ADF4382_CMOS_OV_MSK, dev->cmos_3v3));
	if (ret) {
		pr_info("ERROR: CMOS voltage configuration failed with error: %d\n", ret);
		goto error_spi;
	}
	pr_info("✓ CMOS output voltage configured\n");
	
	for (i = 0; i < NO_OS_ARRAY_SIZE(adf4382_reg_defaults); i++) {
		ret = adf4382_spi_write(dev,
					adf4382_reg_defaults[i].reg,
					adf4382_reg_defaults[i].val);
		if (ret)
			goto error_spi;
	}
	
	pr_info("testing_mux");
	ret = adf4382_spi_write(dev, 0x02E, 0x8);
	if (ret) {
		pr_info("ERROR: muxout write %d\n", ret);
		goto error_spi;
	}
	pr_info("muxout");

	// Step 9: Test scratchpad (this is where it's likely failing)
	pr_info("Step 9: Testing scratchpad register...\n");
	pr_info("  - Writing test value 0x%02X to register 0x00A\n", ADF4382_SPI_SCRATCHPAD_TEST);
	ret = adf4382_spi_write(dev, 0x00A, ADF4382_SPI_SCRATCHPAD_TEST);
	if (ret) {
		pr_info("ERROR: Scratchpad write failed with error: %d\n", ret);
		goto error_spi;
	}
	pr_info("  - Scratchpad write successful\n");
	
	pr_info("  - Reading back from register 0x00A...\n");
	uint8_t scratchpad_read;
	ret = adf4382_spi_read(dev, 0x00A, &scratchpad_read);
	if (ret) {
		pr_info("ERROR: Scratchpad read failed with error: %d\n", ret);
		goto error_spi;
	}
	pr_info("  - Scratchpad read successful, value: 0x%02X\n", scratchpad_read);
	
	if (scratchpad_read != ADF4382_SPI_SCRATCHPAD_TEST) {
		pr_info("ERROR: Scratchpad test failed!\n");
		pr_info("  Expected: 0x%02X, Got: 0x%02X\n", ADF4382_SPI_SCRATCHPAD_TEST, scratchpad_read);
		pr_info("  This indicates the chip is not responding correctly to SPI reads\n");
		ret = -EINVAL;
		goto error_spi;
	}
	pr_info("✓ Scratchpad test passed!\n");
	
	pr_info("=== INITIALIZATION COMPLETED SUCCESSFULLY ===\n");

	// If we get here, the basic initialization worked!
	pr_info("Basic ADF4382 initialization successful!\n");
	pr_info("Chip is responding to SPI communication.\n");
	
	// Clean up
	no_os_spi_remove(dev->spi_desc);
	no_os_free(dev);
	
	pr_info("=== TEST COMPLETED SUCCESSFULLY ===\n");
	return 0;

error_spi:
	pr_info("Cleaning up SPI interface...\n");
	no_os_spi_remove(dev->spi_desc);
error_dev:
	pr_info("Cleaning up device structure...\n");
	no_os_free(dev);
error:
	pr_info("=== TEST FAILED ===\n");
	return ret;
}
