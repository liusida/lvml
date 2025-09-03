/**
 * @file mp_executor.h
 * @brief MicroPython script execution interface
 */

#ifndef MP_EXECUTOR_H
#define MP_EXECUTOR_H

#include "core/lvml_core.h"
#include "xml/xml_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      DEFINES
 *********************/

#define MP_MAX_SCRIPT_LENGTH 8192
#define MP_MAX_OBJECT_NAME_LENGTH 64

/**********************
 *      TYPEDEFS
 **********************/

/**
 * MicroPython execution context
 */
typedef struct {
    lv_obj_t* lvgl_context;       // LVGL object context
    char* object_name;            // Name of the LVGL object
    char* event_type;             // Event type (click, change, etc.)
    void* user_data;              // User-defined data
} mp_execution_context_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize MicroPython executor
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t mp_executor_init(void);

/**
 * Deinitialize MicroPython executor
 */
void mp_executor_deinit(void);

/**
 * Execute MicroPython script
 * @param script Script content to execute
 * @param context Execution context
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t mp_executor_execute_script(const char* script, mp_execution_context_t* context);

/**
 * Execute scripts from XML
 * @param scripts Array of scripts from XML
 * @param script_count Number of scripts
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t mp_executor_execute_scripts(xml_script_t* scripts, size_t script_count);

/**
 * Bind LVGL object to MicroPython namespace
 * @param obj LVGL object to bind
 * @param name Name in MicroPython namespace
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t mp_executor_bind_lvgl_object(lv_obj_t* obj, const char* name);

/**
 * Create execution context
 * @param obj LVGL object
 * @param name Object name
 * @param event Event type
 * @return Execution context or NULL on failure
 */
mp_execution_context_t* mp_executor_create_context(lv_obj_t* obj, const char* name, const char* event);

/**
 * Free execution context
 * @param context Context to free
 */
void mp_executor_free_context(mp_execution_context_t* context);

/**
 * Check if MicroPython is available
 * @return true if available, false otherwise
 */
bool mp_executor_is_available(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*MP_EXECUTOR_H*/
