/***************************************************************************//**
 *   @file   parameters.h
 *   @brief  Definitions specific to Linux platform used by ADF4382_Test project.
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
#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#include "linux_spi.h"
#include "linux_gpio.h"
#include "no_os_spi.h"

/* SPI Configuration for Raspberry Pi spidev */
#define SPI_BAUDRATE    1500000  /* 1.5 MHz SPI speed */
#define SPI_OPS         &linux_spi_ops
#define SPI_EXTRA       &adf4382_spi_extra
#define SPI_DEVICE_ID   0        /* /dev/spidev0.X */
#define SPI_CS          0        /* Chip Select 0 */

/* GPIO Configuration for ADF4382 (optional - can be NULL if not used) */
#define GPIO_OPS        &linux_gpio_ops
#define GPIO_EXTRA      NULL

extern struct linux_spi_init_param adf4382_spi_extra;

#endif /* __PARAMETERS_H__ */

