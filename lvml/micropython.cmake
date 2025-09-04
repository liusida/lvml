# Create an INTERFACE library for our C module.
add_library(usermod_lvml INTERFACE)

# Get the project root directory (assuming this file is in PROJECT_ROOT/lvml/)
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(THIRD_PARTY_ROOT "${PROJECT_ROOT}/third-party")
set(LVGL_DIR "${THIRD_PARTY_ROOT}/lvgl")

# Add our source files to the lib
target_sources(usermod_lvml INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/lvmlmodule.c
    ${CMAKE_CURRENT_LIST_DIR}/lvml.c
)

# Add core source files
file(GLOB CORE_SOURCES 
    "${CMAKE_CURRENT_LIST_DIR}/core/*.c"
)
target_sources(usermod_lvml INTERFACE
    ${CORE_SOURCES}
)

# Add XML parser source files
file(GLOB XML_SOURCES 
    "${CMAKE_CURRENT_LIST_DIR}/xml/*.c"
)
target_sources(usermod_lvml INTERFACE
    ${XML_SOURCES}
)

# Add network manager source files
file(GLOB NETWORK_SOURCES 
    "${CMAKE_CURRENT_LIST_DIR}/network/*.c"
)
target_sources(usermod_lvml INTERFACE
    ${NETWORK_SOURCES}
)

# Add MicroPython executor source files
file(GLOB MICROPYTHON_SOURCES 
    "${CMAKE_CURRENT_LIST_DIR}/micropython/*.c"
)
target_sources(usermod_lvml INTERFACE
    ${MICROPYTHON_SOURCES}
)

# Add utility source files
file(GLOB UTILS_SOURCES 
    "${CMAKE_CURRENT_LIST_DIR}/utils/*.c"
)
target_sources(usermod_lvml INTERFACE
    ${UTILS_SOURCES}
)

# Add driver source files
file(GLOB DRIVER_SOURCES 
    "${CMAKE_CURRENT_LIST_DIR}/driver/*.c"
)
target_sources(usermod_lvml INTERFACE
    ${DRIVER_SOURCES}
)

# Add include directories
target_include_directories(usermod_lvml INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${THIRD_PARTY_ROOT}
    ${PROJECT_ROOT}
)

# Add LVGL source files
file(GLOB_RECURSE LVGL_SOURCES 
    "${LVGL_DIR}/src/*.c"
)

# Add LVGL sources to our module
target_sources(usermod_lvml INTERFACE
    ${LVGL_SOURCES}
)

# Add LVGL compile definitions
target_compile_definitions(usermod_lvml INTERFACE
    LV_CONF_INCLUDE_SIMPLE
    LV_CONF_PATH="${PROJECT_ROOT}/lv_conf.h"
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_lvml)

# Add ESP-IDF components required for LCD driver
# Note: esp_lcd component should be enabled in sdkconfig.board