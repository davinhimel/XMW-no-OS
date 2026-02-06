NO_OS := /home/rpidev1/no-OS/no-OS
PROJECT := /home/rpidev1/no-OS/no-OS/projects/ADF4377_Test

CC ?= gcc
CFLAGS := -fPIC -O2 -Wall -Wextra \
	-I$(NO_OS) \
	-I$(NO_OS)/include \
	-I$(NO_OS)/drivers/api \
	-I$(NO_OS)/drivers/platform/linux \
	-I$(NO_OS)/drivers/frequency/adf4377 \
	-I$(NO_OS)/util \
	-I$(PROJECT)/src/common \
	-I$(PROJECT)/src/platform/linux

SRCS := \
	$(PROJECT)/so/adf4377_test_api.c \
	$(PROJECT)/src/common/common_data.c \
	$(NO_OS)/drivers/api/no_os_spi.c \
	$(NO_OS)/drivers/api/no_os_gpio.c \
	$(NO_OS)/util/no_os_mutex.c \
	$(NO_OS)/util/no_os_util.c \
	$(NO_OS)/util/no_os_alloc.c \
	$(NO_OS)/drivers/platform/linux/linux_spi.c \
	$(NO_OS)/drivers/platform/linux/linux_gpio.c \
	$(NO_OS)/drivers/platform/linux/linux_delay.c \
	$(NO_OS)/drivers/frequency/adf4377/adf4377.c

LDFLAGS := -shared -lpthread -lm
TARGET := libadf4377_test.so

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
