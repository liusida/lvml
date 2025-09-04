/**
 * @file lvml_core.h
 * @brief Core LVML functionality and interfaces
 */

#ifndef LVML_CORE_H
#define LVML_CORE_H

#include "micropython/py/runtime.h"
#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      DEFINES
 *********************/

#define LVML_VERSION "1.0.0"
#define LVML_MAX_URL_LENGTH 512
#define LVML_MAX_XML_SIZE (1024 * 1024) // 1MB max XML size

/**********************
 *      TYPEDEFS
 **********************/

/**
 * LVML UI structure to hold parsed XML data
 */
typedef struct {
    lv_obj_t* root_obj;           // Root LVGL object
    char* title;                  // UI title
    bool has_scripts;             // Whether UI contains MicroPython scripts
    void* script_data;            // MicroPython script data
    size_t script_count;          // Number of scripts
} lvml_ui_t;

/**
 * LVML error codes
 */
typedef enum {
    LVML_OK = 0,
    LVML_ERROR_INIT = -1,
    LVML_ERROR_MEMORY = -2,
    LVML_ERROR_NETWORK = -3,
    LVML_ERROR_XML_PARSE = -4,
    LVML_ERROR_MP_EXEC = -5,
    LVML_ERROR_INVALID_PARAM = -6
} lvml_error_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize LVML core system
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_core_init(void);

// Unified initializer performs full display and memory setup
// (previously lvml_core_init_with_display)

/**
 * Check if LVML core is initialized
 * @return true if initialized, false otherwise
 */
bool lvml_core_is_initialized(void);

/**
 * Get LVML version string
 * @return version string
 */
const char* lvml_core_get_version(void);

/**
 * Print detailed memory information
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_core_print_memory_info(void);

/**
 * Set display rotation
 * @param rotation rotation value (0, 1, 2, 3 for 0°, 90°, 180°, 270°)
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_core_set_rotation(int rotation);

/**
 * Deinitialize LVML core system
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_core_deinit(void);

/**
 * Process LVGL tick and timer handler
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_core_tick(void);

/**
 * Print debug information about display refresh state
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_core_print_refresh_info(void);

/**
 * Turn screen on (enable backlight)
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_core_screen_on(void);

/**
 * Turn screen off (disable backlight)
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_core_screen_off(void);

// UI management functions
#include "lvml_ui.h"

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LVML_CORE_H*/
