/***************************************************************************//**
 *   @file   original_example_test.c
 *   @brief  Test the original basic example structure
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

	pr_info("Enter basic example \n");

	/* UART logging disabled for now - using printf instead */
	/* struct no_os_uart_desc *uart_desc; */
	/* ret = no_os_uart_init(&uart_desc, &adf4382_uart_ip); */
	/* if (ret) */
	/* 	return ret; */
	/* no_os_uart_stdio(uart_desc); */

	pr_info("Initializing ADF4382...\n");
	ret = adf4382_init(&dev, &adf4382_ip);
	if (ret) {
		pr_info("ADF4382 init failed with error: %d\n", ret);
		goto error;
	}
	pr_info("ADF4382 initialized successfully!\n");

	pr_info("Setting RF output to 20 GHz...\n");
	ret = adf4382_set_rfout(dev, 20000000000);
	if (ret) {
		pr_info("Failed to set RF output: %d\n", ret);
		goto remove_adf4382;
	}
	pr_info("RF output set successfully!\n");

	pr_info("Setting phase adjustment...\n");
	ret = adf4382_set_phase_adjust(dev, 1);
	if (ret) {
		pr_info("Failed to set phase adjustment: %d\n", ret);
	} else {
		pr_info("Phase adjustment set successfully!\n");
	}

	pr_info("Example completed successfully!\n");

remove_adf4382:
	pr_info("Cleaning up ADF4382...\n");
	adf4382_remove(dev);
error:
	if (ret) {
		pr_info("Example failed with error: %d\n", ret);
	}
	return ret;
}
