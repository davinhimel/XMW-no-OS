# Add project source directories to SRC_DIRS so all .c files are included
SRC_DIRS += $(PROJECT)/src/platform/$(PLATFORM) \
	    $(PROJECT)/src/common

SRCS += $(DRIVERS)/api/no_os_gpio.c     \
	$(DRIVERS)/api/no_os_spi.c  	\
	$(NO-OS)/util/no_os_mutex.c     \
	$(NO-OS)/util/no_os_util.c      \
	$(NO-OS)/util/no_os_alloc.c

INCS += $(INCLUDE)/no_os_delay.h     \
	$(INCLUDE)/no_os_error.h     \
	$(INCLUDE)/no_os_gpio.h      \
	$(INCLUDE)/no_os_mutex.h     \
	$(INCLUDE)/no_os_spi.h       \
	$(INCLUDE)/no_os_print_log.h \
	$(INCLUDE)/no_os_util.h      \
	$(INCLUDE)/no_os_units.h     \
	$(INCLUDE)/no_os_alloc.h

# Linux platform drivers
SRCS += $(PLATFORM_DRIVERS)/linux_spi.c \
	$(PLATFORM_DRIVERS)/linux_gpio.c \
	$(PLATFORM_DRIVERS)/linux_delay.c

INCS += $(PLATFORM_DRIVERS)/linux_spi.h \
	$(PLATFORM_DRIVERS)/linux_gpio.h

# ADF4382 driver
INCS += $(DRIVERS)/frequency/adf4382/adf4382.h
SRCS += $(DRIVERS)/frequency/adf4382/adf4382.c

