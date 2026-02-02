#!/bin/bash
###############################################################################
#   @file   make_driver_so.sh
#   @brief  Build a shared library for any no-OS driver
#   @author libadnoos Framework
#
#   Usage: ./make_driver_so.sh drivers/frequency/adf4377/adf4377.c
#          ./make_driver_so.sh drivers/accel/adxl367/adxl367.c
#
#   This script compiles a single driver .c file into a shared library that
#   links against libadnoos.so and exports all driver functions.
###############################################################################

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check arguments
if [ $# -lt 1 ]; then
    echo -e "${RED}Error: No driver file specified${NC}"
    echo "Usage: $0 <driver.c> [additional_sources.c ...]"
    echo "Example: $0 drivers/frequency/adf4377/adf4377.c"
    exit 1
fi

# Get paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
NOOS_ROOT="$(cd "$PROJECT_ROOT/../.." && pwd)"
CORE_BUILD_DIR="$PROJECT_ROOT/core/build"
DRIVERS_SO_DIR="$PROJECT_ROOT/drivers_so"

# Check if core library exists
if [ ! -f "$CORE_BUILD_DIR/libadnoos.so" ]; then
    echo -e "${RED}Error: libadnoos.so not found!${NC}"
    echo "Please build the core library first:"
    echo "  cd core && mkdir -p build && cd build && cmake .. && make"
    exit 1
fi

# Get driver file (first argument)
DRIVER_C="$1"
if [ ! -f "$DRIVER_C" ]; then
    # Try relative to NOOS_ROOT
    if [ -f "$NOOS_ROOT/$DRIVER_C" ]; then
        DRIVER_C="$NOOS_ROOT/$DRIVER_C"
    else
        echo -e "${RED}Error: Driver file not found: $1${NC}"
        exit 1
    fi
fi

# Get absolute path
DRIVER_C="$(cd "$(dirname "$DRIVER_C")" && pwd)/$(basename "$DRIVER_C")"

# Extract chip name from path
# e.g., drivers/frequency/adf4377/adf4377.c -> adf4377
DRIVER_DIR=$(dirname "$DRIVER_C")
CHIP_NAME=$(basename "$DRIVER_DIR")

# Check if it's a valid chip name (not "frequency", "accel", etc.)
if [[ "$CHIP_NAME" == "frequency" ]] || [[ "$CHIP_NAME" == "accel" ]] || \
   [[ "$CHIP_NAME" == "adc" ]] || [[ "$CHIP_NAME" == "dac" ]] || \
   [[ "$CHIP_NAME" == "impedance-analyzer" ]]; then
    # Try parent directory
    CHIP_NAME=$(basename "$(dirname "$DRIVER_DIR")")
fi

# Get driver header (usually same name as .c but .h)
DRIVER_H="${DRIVER_C%.c}.h"
if [ ! -f "$DRIVER_H" ]; then
    echo -e "${YELLOW}Warning: Header file not found: $DRIVER_H${NC}"
fi

# Output library name
OUTPUT_LIB="$DRIVERS_SO_DIR/lib${CHIP_NAME}.so"

# Create output directory
mkdir -p "$DRIVERS_SO_DIR"

echo -e "${GREEN}Building lib${CHIP_NAME}.so...${NC}"
echo "  Driver: $DRIVER_C"
echo "  Output: $OUTPUT_LIB"

# Collect all source files (driver + any additional sources)
SOURCES="$DRIVER_C"
shift  # Remove first argument
for arg in "$@"; do
    if [ -f "$arg" ]; then
        SOURCES="$SOURCES $arg"
    elif [ -f "$NOOS_ROOT/$arg" ]; then
        SOURCES="$SOURCES $NOOS_ROOT/$arg"
    fi
done

# Build command
gcc -shared -fPIC -O2 -Wall -Wextra \
    -o "$OUTPUT_LIB" \
    $SOURCES \
    -I"$NOOS_ROOT" \
    -I"$NOOS_ROOT/include" \
    -I"$NOOS_ROOT/drivers/platform/linux" \
    -I"$NOOS_ROOT/util" \
    -L"$CORE_BUILD_DIR" -ladnoos \
    -Wl,-rpath,"$CORE_BUILD_DIR" \
    -lpthread -lm \
    -DLINUX_PLATFORM

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Successfully built lib${CHIP_NAME}.so${NC}"
    echo "  Library: $OUTPUT_LIB"
    echo "  Size: $(du -h "$OUTPUT_LIB" | cut -f1)"
else
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
fi

