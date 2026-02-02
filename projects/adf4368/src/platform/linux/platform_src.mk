INCS += $(PLATFORM_DRIVERS)/linux_uart.h      \
		$(PLATFORM_DRIVERS)/linux_gpio.h      \
		$(PLATFORM_DRIVERS)/linux_spi.h

SRCS += $(PLATFORM_DRIVERS)/linux_uart.c       \
		$(PLATFORM_DRIVERS)/linux_gpio.c       \
		$(PLATFORM_DRIVERS)/linux_spi.c        \
		$(PLATFORM_DRIVERS)/linux_delay.c
