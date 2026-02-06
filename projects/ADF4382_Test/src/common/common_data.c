/***************************************************************************//**
 *   @file   common_data.c
 *   @brief  Defines the common data used in the ADF4382_Test project
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
struct linux_spi_init_param adf4382_spi_extra = {
	.device_id = SPI_DEVICE_ID,
	.chip_select = SPI_CS,
	.max_speed_hz = SPI_BAUDRATE,
	.mode = SPI_MODE_0  /* CPOL=0, CPHA=0 */
};

/* SPI initialization parameters */
struct no_os_spi_init_param adf4382_spi_ip = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = SPI_BAUDRATE,
	.chip_select = SPI_CS,
	.mode = NO_OS_SPI_MODE_0,
	.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
	.platform_ops = SPI_OPS,
	.extra = SPI_EXTRA
};

/* ADF4382 initialization parameters */
struct adf4382_init_param adf4382_ip = {
	.spi_init = &adf4382_spi_ip,
	.spi_3wire_en = false,      /* Use 4-wire SPI (MOSI, MISO, SCLK, CS) */
	.cmos_3v3 = true,            /* CMOS 3.3V logic levels */
	.ref_freq_hz = 125000000,    /* Input reference clock frequency (125 MHz) */
	.freq = 20000000000,         /* Output frequency (20 GHz) */
	.ref_doubler_en = 1,         /* Enable reference doubler */
	.ref_div = 1,                /* Reference divider factor */
	.cp_i = 15,                  /* Charge pump current (11.1 mA - register value 15) */
	.bleed_word = 4903,          /* Bleed word (default) */
	.ld_count = 10,              /* Lock detect count */
	.en_lut_gen = 0,             /* LUT generation enable */
	.en_lut_cal = 0,              /* LUT calibration enable */
	.max_lpf_cap_value_uf = 10,  /* Maximum LPF capacitor value in uF */
	.id = ID_ADF4382              /* Device ID */
};

