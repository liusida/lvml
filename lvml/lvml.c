/**
 * @file lvml.c
 * @brief Main LVML implementation - coordinates all subsystems
 */

#include "lvml.h"
#include "micropython/py/mphal.h"

/**********************
 *  STATIC VARIABLES
 **********************/

static bool lvml_all_initialized = false;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lvml_error_t lvml_init_all(void) {
    mp_printf(&mp_plat_print, "[LVML] All subsystems initializing...\n");
    if (lvml_all_initialized) {
        return LVML_OK;
    }
    
    // Initialize subsystems in order
    lvml_error_t result;
    
    // 1. Initialize memory manager first
    result = memory_manager_init();
    if (result != LVML_OK) {
        return result;
    }
    
    // 2. Initialize core LVML system
    result = lvml_core_init();
    if (result != LVML_OK) {
        memory_manager_deinit();
        return result;
    }
    
    // 3. Initialize XML parser
    result = xml_parser_init();
    if (result != LVML_OK) {
        lvml_core_deinit();
        memory_manager_deinit();
        return result;
    }
    
    
    // 4. Initialize MicroPython executor
    result = mp_executor_init();
    if (result != LVML_OK) {
        xml_parser_deinit();
        lvml_core_deinit();
        memory_manager_deinit();
        return result;
    }
    
    mp_printf(&mp_plat_print, "[LVML] All subsystems initialized\n");
    lvml_all_initialized = true;
    
    return LVML_OK;
}

void lvml_deinit_all(void) {
    if (!lvml_all_initialized) {
        return;
    }
    
    // Deinitialize in reverse order
    mp_executor_deinit();
    xml_parser_deinit();
    lvml_core_deinit();
    memory_manager_deinit();
    
    lvml_all_initialized = false;
}



lvml_error_t lvml_load_from_xml(const char* xml_data) {
    if (!lvml_all_initialized) {
        return LVML_ERROR_INIT;
    }
    
    if (xml_data == NULL) {
        return LVML_ERROR_INVALID_PARAM;
    }
    
    // Parse XML and create UI
    lvml_ui_t ui;
    lvml_error_t result = xml_parser_parse(xml_data, &ui);
    if (result != LVML_OK) {
        return result;
    }
    
    // Extract and execute MicroPython scripts if any
    if (ui.has_scripts) {
        xml_script_t* scripts;
        size_t script_count;
        
        result = xml_parser_extract_scripts(xml_data, &scripts, &script_count);
        if (result == LVML_OK && script_count > 0) {
            result = mp_executor_execute_scripts(scripts, script_count);
            xml_parser_free_scripts(scripts, script_count);
        }
    }
    
    // Free UI structure
    xml_parser_free_ui(&ui);
    
    return result;
}





bool lvml_is_ready(void) {
    return lvml_all_initialized && 
           lvml_core_is_initialized() && 
           memory_manager_is_initialized() && 
           mp_executor_is_available();
}
