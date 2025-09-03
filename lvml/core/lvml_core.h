/**
 * @file lvml_core.h
 * @brief Core LVML functionality and interfaces
 */

#ifndef LVML_CORE_H
#define LVML_CORE_H

#include "py/runtime.h"
#include "lvgl.h"

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

/**
 * Deinitialize LVML core system
 */
void lvml_core_deinit(void);

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
 * Set custom delay function for LVGL
 * @param delay_cb delay callback function
 */
void lvml_core_set_delay_callback(void (*delay_cb)(uint32_t ms));

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LVML_CORE_H*/
