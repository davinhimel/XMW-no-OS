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

# Paths for SWIG build against libadnoos (core + driver .so)
LIBADNOOS     := $(NO_OS)/projects/libadnoos
CORE_BUILD    := $(LIBADNOOS)/core/build
DRIVERS_SO    := $(LIBADNOOS)/drivers_so
SWIG_WRAP     := ADF4377_wrap.c
SWIG_SO       := _adf4377.so
PYTHON_INC    := $(shell python3-config --includes 2>/dev/null || echo "-I/usr/include/python3.11")
PYTHON_LDFLAGS:= $(shell python3-config --ldflags 2>/dev/null || echo "")

# Build SWIG Python module linked to libadnoos (libadf4377.so + libadnoos.so)
# Requires: 1) libadnoos core built: cd libadnoos/core/build && cmake .. && make
#           2) libadf4377.so built:   libadnoos/tools/make_driver_so.sh ../../drivers/frequency/adf4377/adf4377.c
swig-adf4377: $(SWIG_WRAP)
	@test -f "$(CORE_BUILD)/libadnoos.so" || (echo "Missing $(CORE_BUILD)/libadnoos.so - build libadnoos core first."; exit 1)
	@test -f "$(DRIVERS_SO)/libadf4377.so" || (echo "Missing $(DRIVERS_SO)/libadf4377.so - run make_driver_so.sh for adf4377."; exit 1)
	$(CC) -shared -fPIC -O2 $(CFLAGS) $(PYTHON_INC) \
		-o $(SWIG_SO) $(SWIG_WRAP) \
		-L$(DRIVERS_SO) -L$(CORE_BUILD) -ladf4377 -ladnoos \
		-Wl,-rpath,$(DRIVERS_SO):$(CORE_BUILD) \
		$(PYTHON_LDFLAGS) -lpthread -lm
	@echo "Built $(SWIG_SO) (links libadf4377.so + libadnoos.so). Run with: LD_LIBRARY_PATH=$(DRIVERS_SO):$(CORE_BUILD) python3 -c 'import adf4377'"

clean:
	rm -f $(TARGET) $(SWIG_SO)

.PHONY: all clean swig-adf4377
