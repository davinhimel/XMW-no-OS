# Makefile for libadnoos examples

PROJECT_ROOT := $(shell dirname $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
DRIVERS_SO_DIR := $(PROJECT_ROOT)/drivers_so
CORE_BUILD_DIR := $(PROJECT_ROOT)/core/build

# Compiler flags
CFLAGS := -Wall -Wextra -O2 -g
LDFLAGS := -L$(DRIVERS_SO_DIR) -L$(CORE_BUILD_DIR) \
           -Wl,-rpath,$(DRIVERS_SO_DIR):$(CORE_BUILD_DIR)

# Include directories
INCLUDES := -I$(PROJECT_ROOT)/../.. \
            -I$(PROJECT_ROOT)/../../include \
            -I$(PROJECT_ROOT)/../../drivers/platform/linux

# Example targets
.PHONY: all clean adf4377_test

all: adf4377_test

adf4377_test: adf4377_test.c
	@echo "Building adf4377_test..."
	@if [ ! -f "$(CORE_BUILD_DIR)/libadnoos.so" ]; then \
		echo "Error: libadnoos.so not found. Build core library first:"; \
		echo "  cd ../core && mkdir -p build && cd build && cmake .. && make"; \
		exit 1; \
	fi
	@if [ ! -f "$(DRIVERS_SO_DIR)/libadf4377.so" ]; then \
		echo "Error: libadf4377.so not found. Build driver first:"; \
		echo "  cd ../tools && ./make_driver_so.sh ../../drivers/frequency/adf4377/adf4377.c"; \
		exit 1; \
	fi
	gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) -ladf4377 -ladnoos -lpthread -lm
	@echo "✓ Built $@"
	@echo "Run with: LD_LIBRARY_PATH=$(DRIVERS_SO_DIR):$(CORE_BUILD_DIR) ./$@"

clean:
	rm -f adf4377_test *.o

