/***************************************************************************//**
 *   @file   adnoos.h
 *   @brief  libadnoos core header - Universal no-OS Linux Framework
 *   @author libadnoos Framework
********************************************************************************
 * Copyright 2025(c) Analog Devices, Inc.
 *
 * This is the core library header for libadnoos.
 * 
 * libadnoos provides Linux platform implementations for the Analog Devices
 * no-OS framework, allowing any driver to work on Linux using spidev/i2c-dev.
 *
 * Usage:
 *   1. Link against libadnoos.so (provides platform layer)
 *   2. Include individual driver headers directly:
 *      #include "adf4377.h"
 *      #include "adxl367.h"
 *      etc.
 *
 * All driver functions are exported directly from individual driver .so files.
*******************************************************************************/

#pragma once

#ifndef ADNOOS_H_
#define ADNOOS_H_

/**
 * @brief libadnoos core library
 * 
 * This library provides:
 * - Linux SPI implementation (spidev)
 * - Linux I2C implementation (i2c-dev)
 * - Linux GPIO implementation (sysfs)
 * - Linux delay/timing functions
 * - Utility functions (alloc, mutex, fifo, etc.)
 * 
 * Individual chip drivers are built as separate shared libraries that
 * link against libadnoos.so and export their functions directly.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Core library version */
#define ADNOOS_VERSION_MAJOR 1
#define ADNOOS_VERSION_MINOR 0
#define ADNOOS_VERSION_PATCH 0

#ifdef __cplusplus
}
#endif

#endif /* ADNOOS_H_ */

