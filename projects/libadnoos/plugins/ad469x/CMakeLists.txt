cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drivers/platform/linux
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../util
)

# Plugin sources
set(PLUGIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drivers/drivers/adc/ad469x/ad469x.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drivers/api/no_os_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drivers/api/no_os_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drivers/platform/linux/linux_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drivers/platform/linux/linux_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drivers/platform/linux/linux_delay.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../util/no_os_mutex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../util/no_os_util.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../util/no_os_alloc.c
)

# Create plugin shared library
add_library(adnoos_ad469x SHARED ${PLUGIN_SOURCES})

target_compile_options(adnoos_ad469x PRIVATE -fPIC -O2 -Wall -Wextra -Wno-error)
target_compile_definitions(adnoos_ad469x PRIVATE LINUX_PLATFORM)

# Link libraries
target_link_libraries(adnoos_ad469x PRIVATE m pthread)

# Install plugin
install(TARGETS adnoos_ad469x
    LIBRARY DESTINATION lib/adnoos
    ARCHIVE DESTINATION lib/adnoos
)

# Set output name
set_target_properties(adnoos_ad469x PROPERTIES
    OUTPUT_NAME "adnoos_ad469x"
    PREFIX "lib"
)

# Create plugins directory in build
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

# Copy plugin to build directory for testing
add_custom_command(TARGET adnoos_ad469x POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:adnoos_ad469x>
    ${CMAKE_BINARY_DIR}/plugins
    COMMENT "Copying plugin to build directory"
)
