/***************************************************************************//**
 *   @file   plugin.c
 *   @brief  AD796X Plugin for libadnoos (AUTO-GENERATED)
 *   @author libadnoos
********************************************************************************
 * Copyright 2025(c) Analog Devices, Inc.
 * AUTO-GENERATED - Do not edit manually
*******************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Include no-OS headers
#include "no_os_spi.h"
#include "no_os_gpio.h"
#include "no_os_alloc.h"
#include "linux_spi.h"
#include "linux_gpio.h"
#include <linux/spi/spidev.h>

// Include device driver
#include "drivers/adc/ad796x/ad796x.h"

// Default SPI configuration
static struct linux_spi_init_param default_spi_extra = {
	.device_id = 0,
	.chip_select = 0,
	.max_speed_hz = 10000000,
	.mode = SPI_MODE_0
};

static struct no_os_spi_init_param default_spi_init = {
	.device_id = 0,
	.max_speed_hz = 10000000,
	.chip_select = 0,
	.mode = NO_OS_SPI_MODE_0,
	.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
	.platform_ops = &linux_spi_ops,
	.extra = &default_spi_extra
};

/**
 * @brief Create default initialization parameters
 */
void* adnoos_create_init_param(void)
{
	struct ad796x_init_param *param = (struct ad796x_init_param *)
		no_os_calloc(1, sizeof(struct ad796x_init_param));
	if (!param)
		return NULL;
	
	// Setup SPI (if device uses SPI)
	// TODO: Auto-detect if device needs SPI/I2C/UART
	struct no_os_spi_init_param *spi_init = (struct no_os_spi_init_param *)
		no_os_calloc(1, sizeof(struct no_os_spi_init_param));
	if (spi_init) {
		*spi_init = default_spi_init;
		struct linux_spi_init_param *extra = (struct linux_spi_init_param *)
			no_os_calloc(1, sizeof(struct linux_spi_init_param));
		if (extra) {
			*extra = default_spi_extra;
			spi_init->extra = extra;
		}
		// Set spi_init pointer if device struct has it
		// This is device-specific and may need manual adjustment
	}
	
	// Set device-specific defaults
	// TODO: Add device-specific parameter initialization here
	
	return param;
}

/**
 * @brief Initialize device
 */
int32_t adnoos_init_device(void **device, void *init_param)
{
	if (!device || !init_param) {
		return -1;
	}
	
	struct ad796x_init_param *param = (struct ad796x_init_param *)init_param;
	struct ad796x_dev *dev = NULL;
	
	int32_t ret = ad796x_gpio_init(&dev, param);
	if (ret != 0) {
		return ret;
	}
	
	*device = dev;
	return 0;
}

/**
 * @brief Remove device
 */
int32_t adnoos_remove_device(void *device)
{
	if (!device) {
		return -1;
	}
	
	struct ad796x_dev *dev = (struct ad796x_dev *)device;
	return ad796x_gpio_remove(dev);
}
