/**
 * @file mp_executor.c
 * @brief MicroPython script execution implementation
 */

#include "mp_executor.h"
#include "py/mphal.h"

/**********************
 *  STATIC VARIABLES
 **********************/

static bool mp_executor_initialized = false;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lvml_error_t mp_executor_init(void) {
    if (mp_executor_initialized) {
        return LVML_OK;
    }
    
    mp_printf(&mp_plat_print, "[MP] Initializing MicroPython executor\n");
    
    mp_executor_initialized = true;
    mp_printf(&mp_plat_print, "[MP] MicroPython executor initialized successfully\n");
    
    return LVML_OK;
}

void mp_executor_deinit(void) {
    if (!mp_executor_initialized) {
        return;
    }
    
    mp_printf(&mp_plat_print, "[MP] Deinitializing MicroPython executor\n");
    
    mp_executor_initialized = false;
    mp_printf(&mp_plat_print, "[MP] MicroPython executor deinitialized\n");
}

lvml_error_t mp_executor_execute_script(const char* script, mp_execution_context_t* context) {
    if (!mp_executor_initialized) {
        mp_printf(&mp_plat_print, "[MP] Executor not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    if (script == NULL) {
        mp_printf(&mp_plat_print, "[MP] Invalid script\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    
    mp_printf(&mp_plat_print, "[MP] Executing MicroPython script (%d chars)\n", strlen(script));
    
    if (context != NULL && context->object_name != NULL) {
        mp_printf(&mp_plat_print, "[MP] Context: object=%s, event=%s\n", 
                  context->object_name, 
                  context->event_type ? context->event_type : "none");
    }
    
    // TODO: Implement actual MicroPython script execution
    // For now, just log the script
    mp_printf(&mp_plat_print, "[MP] Script content: %.100s%s\n", 
              script, strlen(script) > 100 ? "..." : "");
    
    mp_printf(&mp_plat_print, "[MP] Script execution completed (minimal implementation)\n");
    
    return LVML_OK;
}

lvml_error_t mp_executor_execute_scripts(xml_script_t* scripts, size_t script_count) {
    if (!mp_executor_initialized) {
        mp_printf(&mp_plat_print, "[MP] Executor not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    if (scripts == NULL || script_count == 0) {
        mp_printf(&mp_plat_print, "[MP] No scripts to execute\n");
        return LVML_OK;
    }
    
    mp_printf(&mp_plat_print, "[MP] Executing %d scripts from XML\n", script_count);
    
    for (size_t i = 0; i < script_count; i++) {
        mp_printf(&mp_plat_print, "[MP] Executing script %d/%d\n", i + 1, script_count);
        
        mp_execution_context_t context = {0};
        context.object_name = scripts[i].target_object;
        context.event_type = scripts[i].event_type;
        
        lvml_error_t result = mp_executor_execute_script(scripts[i].script_content, &context);
        if (result != LVML_OK) {
            mp_printf(&mp_plat_print, "[MP] Script %d execution failed\n", i + 1);
            return result;
        }
    }
    
    mp_printf(&mp_plat_print, "[MP] All scripts executed successfully\n");
    
    return LVML_OK;
}

lvml_error_t mp_executor_bind_lvgl_object(lv_obj_t* obj, const char* name) {
    if (!mp_executor_initialized) {
        mp_printf(&mp_plat_print, "[MP] Executor not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    if (obj == NULL || name == NULL) {
        mp_printf(&mp_plat_print, "[MP] Invalid parameters for binding\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    
    mp_printf(&mp_plat_print, "[MP] Binding LVGL object to MicroPython namespace: %s\n", name);
    
    // TODO: Implement actual LVGL object binding to MicroPython
    // This would involve creating Python objects that wrap LVGL objects
    
    mp_printf(&mp_plat_print, "[MP] Object binding completed (minimal implementation)\n");
    
    return LVML_OK;
}

mp_execution_context_t* mp_executor_create_context(lv_obj_t* obj, const char* name, const char* event) {
    if (name == NULL) {
        mp_printf(&mp_plat_print, "[MP] Invalid name for context creation\n");
        return NULL;
    }
    
    mp_execution_context_t* context = (mp_execution_context_t*)malloc(sizeof(mp_execution_context_t));
    if (context == NULL) {
        mp_printf(&mp_plat_print, "[MP] Failed to allocate execution context\n");
        return NULL;
    }
    
    context->lvgl_context = obj;
    context->object_name = strdup(name);
    context->event_type = event ? strdup(event) : NULL;
    context->user_data = NULL;
    
    mp_printf(&mp_plat_print, "[MP] Created execution context for %s\n", name);
    
    return context;
}

void mp_executor_free_context(mp_execution_context_t* context) {
    if (context == NULL) {
        return;
    }
    
    mp_printf(&mp_plat_print, "[MP] Freeing execution context\n");
    
    if (context->object_name != NULL) {
        free(context->object_name);
    }
    
    if (context->event_type != NULL) {
        free(context->event_type);
    }
    
    free(context);
}

bool mp_executor_is_available(void) {
    return mp_executor_initialized;
}
