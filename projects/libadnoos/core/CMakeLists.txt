cmake_minimum_required(VERSION 3.10)
project(adnoos C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Get the root of the no-OS repository
# From core/CMakeLists.txt: go up to libadnoos/, then up to projects/, then up to no-OS root
get_filename_component(PROJECT_DIR "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)
get_filename_component(PROJECTS_DIR "${PROJECT_DIR}/.." ABSOLUTE)
get_filename_component(NOOS_ROOT "${PROJECTS_DIR}/.." ABSOLUTE)

# Platform-specific source files (Linux implementations)
set(PLATFORM_SOURCES
    ${NOOS_ROOT}/drivers/platform/linux/linux_spi.c
    ${NOOS_ROOT}/drivers/platform/linux/linux_gpio.c
    ${NOOS_ROOT}/drivers/platform/linux/linux_i2c.c
    ${NOOS_ROOT}/drivers/platform/linux/linux_uart.c
    ${NOOS_ROOT}/drivers/platform/linux/linux_delay.c
    ${NOOS_ROOT}/drivers/platform/linux/linux_timer.c
)

# API layer source files (hardware-agnostic wrappers)
set(API_SOURCES
    ${NOOS_ROOT}/drivers/api/no_os_spi.c
    ${NOOS_ROOT}/drivers/api/no_os_gpio.c
    ${NOOS_ROOT}/drivers/api/no_os_i2c.c
    ${NOOS_ROOT}/drivers/api/no_os_uart.c
    ${NOOS_ROOT}/drivers/api/no_os_timer.c
    ${NOOS_ROOT}/drivers/api/no_os_irq.c
)

# Utility source files
set(UTIL_SOURCES
    ${NOOS_ROOT}/util/no_os_alloc.c
    ${NOOS_ROOT}/util/no_os_mutex.c
    ${NOOS_ROOT}/util/no_os_util.c
    ${NOOS_ROOT}/util/no_os_fifo.c
    ${NOOS_ROOT}/util/no_os_list.c
    ${NOOS_ROOT}/util/no_os_lf256fifo.c
)

# Combine all sources
set(ADNOOS_SOURCES
    ${PLATFORM_SOURCES}
    ${API_SOURCES}
    ${UTIL_SOURCES}
)

# Include directories
include_directories(
    ${NOOS_ROOT}
    ${NOOS_ROOT}/include
    ${NOOS_ROOT}/drivers/platform/linux
    ${NOOS_ROOT}/util
    ${CMAKE_SOURCE_DIR}/include
)

# Create shared library
add_library(adnoos SHARED ${ADNOOS_SOURCES})

# Compiler flags
target_compile_options(adnoos PRIVATE
    -fPIC
    -O2
    -Wall
    -Wextra
    -DLINUX_PLATFORM
)

# Link libraries
target_link_libraries(adnoos
    pthread
    m
)

# Set library properties
set_target_properties(adnoos PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/adnoos.h"
)

# Install target
install(TARGETS adnoos
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

# Install headers (copy all no-OS headers to a convenient location)
install(DIRECTORY ${NOOS_ROOT}/include/
    DESTINATION include/adnoos
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY ${NOOS_ROOT}/drivers/platform/linux/
    DESTINATION include/adnoos/platform
    FILES_MATCHING PATTERN "*.h"
)

