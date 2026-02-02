/***************************************************************************//**
 *   @file   update_adf4382.c
 *   @brief  Sample application demonstrating dynamic parameter updates using libadf4382.so
 *   @author CHegbeli (ciprian.hegbeli@analog.com)
********************************************************************************
 * Copyright 2023(c) Analog Devices, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdbool.h>

/* Function pointer types for ADF4382 library functions */
typedef int (*adf4382_init_func)(void **device, void *init_param);
typedef int (*adf4382_remove_func)(void *dev);
typedef int (*adf4382_set_rfout_func)(void *dev, uint64_t freq);
typedef int (*adf4382_set_cp_i_func)(void *dev, int32_t cp_i);
typedef int (*adf4382_set_bleed_word_func)(void *dev, int32_t bleed_word);
typedef int (*adf4382_set_phase_adjust_func)(void *dev, uint32_t phase_ps);

/* Structure definitions matching ADF4382 driver */
struct no_os_spi_init_param {
	uint32_t device_id;
	uint32_t max_speed_hz;
	uint32_t bit_order;
	uint32_t mode;
	void *platform_ops;
	uint32_t chip_select;
	void *extra;
};

struct adf4382_init_param {
	struct no_os_spi_init_param *spi_init;
	bool spi_3wire_en;
	bool cmos_3v3;
	uint64_t ref_freq_hz;
	uint64_t freq;
	bool ref_doubler_en;
	uint8_t ref_div;
	uint8_t cp_i;
	uint16_t bleed_word;
	uint8_t ld_count;
	uint8_t en_lut_gen;
	uint8_t en_lut_cal;
	uint8_t max_lpf_cap_value_uf;
	uint8_t id;
};

/* Global variables */
void *lib_handle = NULL;
void *adf4382_dev = NULL;

/* Function pointers */
adf4382_init_func adf4382_init = NULL;
adf4382_remove_func adf4382_remove = NULL;
adf4382_set_rfout_func adf4382_set_rfout = NULL;
adf4382_set_cp_i_func adf4382_set_cp_i = NULL;
adf4382_set_bleed_word_func adf4382_set_bleed_word = NULL;
adf4382_set_phase_adjust_func adf4382_set_phase_adjust = NULL;

/**
 * @brief Load the ADF4382 shared library and resolve function symbols
 * @return 0 on success, -1 on failure
 */
int load_adf4382_library(void)
{
	lib_handle = dlopen("./libadf4382.so", RTLD_LAZY);
	if (!lib_handle) {
		fprintf(stderr, "Error loading libadf4382.so: %s\n", dlerror());
		return -1;
	}
	
	/* Resolve function symbols */
	adf4382_init = (adf4382_init_func)dlsym(lib_handle, "adf4382_init");
	adf4382_remove = (adf4382_remove_func)dlsym(lib_handle, "adf4382_remove");
	adf4382_set_rfout = (adf4382_set_rfout_func)dlsym(lib_handle, "adf4382_set_rfout");
	adf4382_set_cp_i = (adf4382_set_cp_i_func)dlsym(lib_handle, "adf4382_set_cp_i");
	adf4382_set_bleed_word = (adf4382_set_bleed_word_func)dlsym(lib_handle, "adf4382_set_bleed_word");
	adf4382_set_phase_adjust = (adf4382_set_phase_adjust_func)dlsym(lib_handle, "adf4382_set_phase_adjust");
	
	if (!adf4382_init || !adf4382_remove || !adf4382_set_rfout || 
	    !adf4382_set_cp_i || !adf4382_set_bleed_word || !adf4382_set_phase_adjust) {
		fprintf(stderr, "Error resolving symbols: %s\n", dlerror());
		dlclose(lib_handle);
		return -1;
	}
	
	return 0;
}

/**
 * @brief Initialize ADF4382 device
 * @param spi_device_id SPI device ID
 * @return 0 on success, -1 on failure
 */
int init_adf4382(uint32_t spi_device_id)
{
	struct no_os_spi_init_param spi_param = {
		.device_id = spi_device_id,
		.max_speed_hz = 1500000,
		.bit_order = 0, /* MSB first */
		.mode = 0, /* SPI mode 0 */
		.platform_ops = NULL, /* Will be set by library */
		.chip_select = 0,
		.extra = NULL
	};
	
	struct adf4382_init_param init_param = {
		.spi_init = &spi_param,
		.spi_3wire_en = false,
		.cmos_3v3 = false,
		.ref_freq_hz = 125000000,
		.freq = 20000000000,
		.ref_doubler_en = true,
		.ref_div = 1,
		.cp_i = 15,
		.bleed_word = 4903,
		.ld_count = 10,
		.en_lut_gen = 0,
		.en_lut_cal = 0,
		.max_lpf_cap_value_uf = 10,
		.id = 0 /* ADF4382 */
	};
	
	int ret = adf4382_init(&adf4382_dev, &init_param);
	if (ret != 0) {
		fprintf(stderr, "Failed to initialize ADF4382: %d\n", ret);
		return -1;
	}
	
	printf("ADF4382 initialized successfully\n");
	return 0;
}

/**
 * @brief Set output frequency
 * @param freq_hz Frequency in Hz
 * @return 0 on success, -1 on failure
 */
int set_frequency(uint64_t freq_hz)
{
	if (!adf4382_dev) {
		fprintf(stderr, "ADF4382 not initialized\n");
		return -1;
	}
	
	int ret = adf4382_set_rfout(adf4382_dev, freq_hz);
	if (ret != 0) {
		fprintf(stderr, "Failed to set frequency: %d\n", ret);
		return -1;
	}
	
	printf("Frequency set to %lu Hz\n", freq_hz);
	return 0;
}

/**
 * @brief Set charge pump current
 * @param cp_i Charge pump current (0-15)
 * @return 0 on success, -1 on failure
 */
int set_charge_pump_current(int32_t cp_i)
{
	if (!adf4382_dev) {
		fprintf(stderr, "ADF4382 not initialized\n");
		return -1;
	}
	
	if (cp_i < 0 || cp_i > 15) {
		fprintf(stderr, "Invalid charge pump current: %d (must be 0-15)\n", cp_i);
		return -1;
	}
	
	int ret = adf4382_set_cp_i(adf4382_dev, cp_i);
	if (ret != 0) {
		fprintf(stderr, "Failed to set charge pump current: %d\n", ret);
		return -1;
	}
	
	printf("Charge pump current set to %d\n", cp_i);
	return 0;
}

/**
 * @brief Set bleed word
 * @param bleed_word Bleed word (0-8191)
 * @return 0 on success, -1 on failure
 */
int set_bleed_word(int32_t bleed_word)
{
	if (!adf4382_dev) {
		fprintf(stderr, "ADF4382 not initialized\n");
		return -1;
	}
	
	if (bleed_word < 0 || bleed_word > 8191) {
		fprintf(stderr, "Invalid bleed word: %d (must be 0-8191)\n", bleed_word);
		return -1;
	}
	
	int ret = adf4382_set_bleed_word(adf4382_dev, bleed_word);
	if (ret != 0) {
		fprintf(stderr, "Failed to set bleed word: %d\n", ret);
		return -1;
	}
	
	printf("Bleed word set to %d\n", bleed_word);
	return 0;
}

/**
 * @brief Set phase adjustment
 * @param phase_ps Phase adjustment in picoseconds
 * @return 0 on success, -1 on failure
 */
int set_phase_adjustment(uint32_t phase_ps)
{
	if (!adf4382_dev) {
		fprintf(stderr, "ADF4382 not initialized\n");
		return -1;
	}
	
	int ret = adf4382_set_phase_adjust(adf4382_dev, phase_ps);
	if (ret != 0) {
		fprintf(stderr, "Failed to set phase adjustment: %d\n", ret);
		return -1;
	}
	
	printf("Phase adjustment set to %u ps\n", phase_ps);
	return 0;
}

/**
 * @brief Cleanup resources
 */
void cleanup(void)
{
	if (adf4382_dev) {
		adf4382_remove(adf4382_dev);
		adf4382_dev = NULL;
	}
	
	if (lib_handle) {
		dlclose(lib_handle);
		lib_handle = NULL;
	}
}

/**
 * @brief Print usage information
 */
void print_usage(const char *program_name)
{
	printf("ADF4382 Dynamic Parameter Update Tool\n");
	printf("Usage: %s [options]\n", program_name);
	printf("Options:\n");
	printf("  --spi-device <id>     SPI device ID (default: 0)\n");
	printf("  --freq <hz>          Set output frequency in Hz\n");
	printf("  --cp-i <value>       Set charge pump current (0-15)\n");
	printf("  --bleed <value>      Set bleed word (0-8191)\n");
	printf("  --phase <ps>         Set phase adjustment in picoseconds\n");
	printf("  --help, -h           Show this help message\n");
	printf("\nExamples:\n");
	printf("  %s --freq 15000000000 --cp-i 10\n", program_name);
	printf("  %s --spi-device 1 --freq 18000000000 --bleed 4000\n", program_name);
}

int main(int argc, char *argv[])
{
	uint32_t spi_device_id = 0;
	uint64_t freq_hz = 0;
	int32_t cp_i = -1;
	int32_t bleed_word = -1;
	uint32_t phase_ps = 0;
	bool freq_set = false, cp_set = false, bleed_set = false, phase_set = false;
	
	/* Parse command line arguments */
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--spi-device") == 0 && i + 1 < argc) {
			spi_device_id = atoi(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "--freq") == 0 && i + 1 < argc) {
			freq_hz = strtoull(argv[i + 1], NULL, 10);
			freq_set = true;
			i++;
		} else if (strcmp(argv[i], "--cp-i") == 0 && i + 1 < argc) {
			cp_i = atoi(argv[i + 1]);
			cp_set = true;
			i++;
		} else if (strcmp(argv[i], "--bleed") == 0 && i + 1 < argc) {
			bleed_word = atoi(argv[i + 1]);
			bleed_set = true;
			i++;
		} else if (strcmp(argv[i], "--phase") == 0 && i + 1 < argc) {
			phase_ps = atoi(argv[i + 1]);
			phase_set = true;
			i++;
		} else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			print_usage(argv[0]);
			return 0;
		} else {
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			print_usage(argv[0]);
			return 1;
		}
	}
	
	/* Check if any parameters were specified */
	if (!freq_set && !cp_set && !bleed_set && !phase_set) {
		fprintf(stderr, "No parameters specified. Use --help for usage information.\n");
		return 1;
	}
	
	/* Load the shared library */
	if (load_adf4382_library() != 0) {
		return 1;
	}
	
	/* Initialize ADF4382 */
	if (init_adf4382(spi_device_id) != 0) {
		cleanup();
		return 1;
	}
	
	/* Apply parameter updates */
	if (freq_set) {
		if (set_frequency(freq_hz) != 0) {
			cleanup();
			return 1;
		}
	}
	
	if (cp_set) {
		if (set_charge_pump_current(cp_i) != 0) {
			cleanup();
			return 1;
		}
	}
	
	if (bleed_set) {
		if (set_bleed_word(bleed_word) != 0) {
			cleanup();
			return 1;
		}
	}
	
	if (phase_set) {
		if (set_phase_adjustment(phase_ps) != 0) {
			cleanup();
			return 1;
		}
	}
	
	printf("All parameter updates completed successfully\n");
	
	/* Cleanup */
	cleanup();
	
	return 0;
} 