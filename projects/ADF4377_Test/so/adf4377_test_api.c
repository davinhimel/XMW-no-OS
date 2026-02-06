/***************************************************************************//**
 *   @file   adf4377_test_api.c
 *   @brief  Minimal user-space API wrapper for ADF4377_Test
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

#include "adf4377.h"
#include "common_data.h"
#include "parameters.h"
#include "no_os_error.h"

static struct adf4377_dev *adf4377_dev;

int adf4377_test_init(int spi_device, int spi_cs)
{
	adf4377_spi_extra.device_id = (uint32_t)spi_device;
	adf4377_spi_extra.chip_select = (uint8_t)spi_cs;
	adf4377_spi_ip.device_id = (uint32_t)spi_device;
	adf4377_spi_ip.chip_select = (uint8_t)spi_cs;

	return adf4377_init(&adf4377_dev, &adf4377_ip);
}

int adf4377_test_read_reg(uint16_t reg, uint8_t *val)
{
	if (!adf4377_dev)
		return -EINVAL;

	return adf4377_spi_read(adf4377_dev, ADF4377_REG(reg), val);
}

int adf4377_test_write_reg(uint16_t reg, uint8_t val)
{
	if (!adf4377_dev)
		return -EINVAL;

	return adf4377_spi_write(adf4377_dev, ADF4377_REG(reg), val);
}

int adf4377_test_update_bits(uint16_t reg, uint8_t mask, uint8_t value)
{
	if (!adf4377_dev)
		return -EINVAL;

	return adf4377_spi_update_bit(adf4377_dev, ADF4377_REG(reg), mask, value);
}

int adf4377_test_set_rfout(uint64_t freq_hz)
{
	if (!adf4377_dev)
		return -EINVAL;

	return adf4377_set_rfout(adf4377_dev, freq_hz);
}

int adf4377_test_remove(void)
{
	int ret;

	if (!adf4377_dev)
		return 0;

	ret = adf4377_remove(adf4377_dev);
	adf4377_dev = NULL;
	return ret;
}
