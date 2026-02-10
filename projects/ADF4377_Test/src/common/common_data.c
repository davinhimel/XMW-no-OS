/***************************************************************************//**
 *   @file   common_data.c
 *   @brief  Defines the common data used in the ADF4377_Test project
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
#include "parameters.h"
#include <linux/spi/spidev.h>

/* Linux SPI extra parameters */
struct linux_spi_init_param adf4377_spi_extra = {
	.device_id = SPI_DEVICE_ID,
	.chip_select = SPI_CS,
	.max_speed_hz = SPI_BAUDRATE,
	.mode = SPI_MODE_0  /* CPOL=0, CPHA=0 */
};

/* SPI initialization parameters */
struct no_os_spi_init_param adf4377_spi_ip = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = SPI_BAUDRATE,
	.chip_select = SPI_CS,
	.mode = NO_OS_SPI_MODE_0,
#ifdef ADF4377_TEST_LSB_FIRST
	.bit_order = NO_OS_SPI_BIT_ORDER_LSB_FIRST,
#else
	.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
#endif
	.platform_ops = SPI_OPS,
	.extra = SPI_EXTRA
};

/* GPIO initialization parameters (optional - set to NULL if not used) */
struct no_os_gpio_init_param gpio_ce_param = {
	.number = GPIO_CE,
	.platform_ops = GPIO_OPS,
	.extra = GPIO_EXTRA
};

struct no_os_gpio_init_param gpio_enclk1_param = {
	.number = GPIO_ENCLK1,
	.platform_ops = GPIO_OPS,
	.extra = GPIO_EXTRA
};

struct no_os_gpio_init_param gpio_enclk2_param = {
	.number = GPIO_ENCLK2,
	.platform_ops = GPIO_OPS,
	.extra = GPIO_EXTRA
};

/* ADF4377 initialization parameters */
struct adf4377_init_param adf4377_ip = {
	.dev_id = ADF4377,
	.spi_init = &adf4377_spi_ip,
	.spi4wire = true,  /* Use 4-wire SPI (MOSI, MISO, SCLK, CS) */
	.gpio_ce_param = NULL,      /* Optional - set to NULL if not used */
	.gpio_enclk1_param = NULL,  /* Optional - set to NULL if not used */
	.gpio_enclk2_param = NULL,  /* Optional - set to NULL if not used */
	.clkin_freq = 125000000,     /* Input reference clock frequency (125 MHz) */
	.ref_doubler_en = 1,         /* Enable reference doubler */
	.f_clk = 10000000000,        /* Output frequency (10 GHz) */
	.ref_div_factor = 1,         /* Reference divider factor */
	.muxout_select = ADF4377_MUXOUT_HIGH_Z,  /* Default MUXOUT (will be changed to HIGH) */
	.cp_i = ADF4377_CP_10MA1,    /* Charge pump current */
	.clkout_op = ADF4377_CLKOUT_640MV  /* Output amplitude */
};

