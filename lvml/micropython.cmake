# Create an INTERFACE library for our C module.
add_library(usermod_lvml INTERFACE)

# Get the project root directory (assuming this file is in PROJECT_ROOT/lvml/)
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(LVGL_DIR "${PROJECT_ROOT}/third-party/lvgl")

# Add our source files to the lib
target_sources(usermod_lvml INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/lvmlmodule.c
)

# Add include directories
target_include_directories(usermod_lvml INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${LVGL_DIR}
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