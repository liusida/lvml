/**
 * @file lvml.c
 * @brief Main LVML implementation - coordinates all subsystems
 */

#include "lvml.h"
#include "py/mphal.h"

/**********************
 *  STATIC VARIABLES
 **********************/

static bool lvml_all_initialized = false;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lvml_error_t lvml_init_all(void) {
    if (lvml_all_initialized) {
        mp_printf(&mp_plat_print, "[LVML] All subsystems already initialized\n");
        return LVML_OK;
    }
    
    mp_printf(&mp_plat_print, "[LVML] Initializing all subsystems\n");
    
    // Initialize subsystems in order
    lvml_error_t result;
    
    // 1. Initialize memory manager first
    result = memory_manager_init();
    if (result != LVML_OK) {
        mp_printf(&mp_plat_print, "[LVML] Memory manager initialization failed\n");
        return result;
    }
    
    // 2. Initialize core LVML system
    result = lvml_core_init();
    if (result != LVML_OK) {
        mp_printf(&mp_plat_print, "[LVML] Core system initialization failed\n");
        memory_manager_deinit();
        return result;
    }
    
    // 3. Initialize XML parser
    result = xml_parser_init();
    if (result != LVML_OK) {
        mp_printf(&mp_plat_print, "[LVML] XML parser initialization failed\n");
        lvml_core_deinit();
        memory_manager_deinit();
        return result;
    }
    
    // 4. Initialize network manager
    result = network_manager_init();
    if (result != LVML_OK) {
        mp_printf(&mp_plat_print, "[LVML] Network manager initialization failed\n");
        xml_parser_deinit();
        lvml_core_deinit();
        memory_manager_deinit();
        return result;
    }
    
    // 5. Initialize MicroPython executor
    result = mp_executor_init();
    if (result != LVML_OK) {
        mp_printf(&mp_plat_print, "[LVML] MicroPython executor initialization failed\n");
        network_manager_deinit();
        xml_parser_deinit();
        lvml_core_deinit();
        memory_manager_deinit();
        return result;
    }
    
    lvml_all_initialized = true;
    mp_printf(&mp_plat_print, "[LVML] All subsystems initialized successfully\n");
    
    return LVML_OK;
}

void lvml_deinit_all(void) {
    if (!lvml_all_initialized) {
        mp_printf(&mp_plat_print, "[LVML] Subsystems not initialized\n");
        return;
    }
    
    mp_printf(&mp_plat_print, "[LVML] Deinitializing all subsystems\n");
    
    // Deinitialize in reverse order
    mp_executor_deinit();
    network_manager_deinit();
    xml_parser_deinit();
    lvml_core_deinit();
    memory_manager_deinit();
    
    lvml_all_initialized = false;
    mp_printf(&mp_plat_print, "[LVML] All subsystems deinitialized\n");
}

lvml_error_t lvml_load_from_url(const char* url) {
    if (!lvml_all_initialized) {
        mp_printf(&mp_plat_print, "[LVML] LVML not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    if (url == NULL) {
        mp_printf(&mp_plat_print, "[LVML] Invalid URL\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    
    mp_printf(&mp_plat_print, "[LVML] Loading UI from URL: %s\n", url);
    
    // Check if WiFi is connected
    if (network_manager_get_wifi_status() != WIFI_STATUS_CONNECTED) {
        mp_printf(&mp_plat_print, "[LVML] WiFi not connected\n");
        return LVML_ERROR_NETWORK;
    }
    
    // Fetch XML from URL
    http_response_t response;
    lvml_error_t result = network_manager_http_get(url, &response);
    if (result != LVML_OK) {
        mp_printf(&mp_plat_print, "[LVML] Failed to fetch XML from URL\n");
        return result;
    }
    
    if (response.status_code != 200) {
        mp_printf(&mp_plat_print, "[LVML] HTTP error: %d\n", response.status_code);
        network_manager_free_response(&response);
        return LVML_ERROR_NETWORK;
    }
    
    // Load UI from XML
    result = lvml_load_from_xml(response.content);
    
    // Free response
    network_manager_free_response(&response);
    
    if (result != LVML_OK) {
        mp_printf(&mp_plat_print, "[LVML] Failed to load UI from XML\n");
        return result;
    }
    
    mp_printf(&mp_plat_print, "[LVML] UI loaded successfully from URL\n");
    
    return LVML_OK;
}

lvml_error_t lvml_load_from_xml(const char* xml_data) {
    if (!lvml_all_initialized) {
        mp_printf(&mp_plat_print, "[LVML] LVML not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    if (xml_data == NULL) {
        mp_printf(&mp_plat_print, "[LVML] Invalid XML data\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    
    mp_printf(&mp_plat_print, "[LVML] Loading UI from XML data\n");
    
    // Parse XML and create UI
    lvml_ui_t ui;
    lvml_error_t result = xml_parser_parse(xml_data, &ui);
    if (result != LVML_OK) {
        mp_printf(&mp_plat_print, "[LVML] Failed to parse XML\n");
        return result;
    }
    
    // Extract and execute MicroPython scripts if any
    if (ui.has_scripts) {
        xml_script_t* scripts;
        size_t script_count;
        
        result = xml_parser_extract_scripts(xml_data, &scripts, &script_count);
        if (result == LVML_OK && script_count > 0) {
            mp_printf(&mp_plat_print, "[LVML] Executing %d MicroPython scripts\n", script_count);
            result = mp_executor_execute_scripts(scripts, script_count);
            xml_parser_free_scripts(scripts, script_count);
        }
    }
    
    // Free UI structure
    xml_parser_free_ui(&ui);
    
    if (result != LVML_OK) {
        mp_printf(&mp_plat_print, "[LVML] Failed to execute scripts\n");
        return result;
    }
    
    mp_printf(&mp_plat_print, "[LVML] UI loaded successfully from XML\n");
    
    return LVML_OK;
}

lvml_error_t lvml_connect_wifi(const char* ssid, const char* password) {
    if (!lvml_all_initialized) {
        mp_printf(&mp_plat_print, "[LVML] LVML not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    if (ssid == NULL) {
        mp_printf(&mp_plat_print, "[LVML] Invalid SSID\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    
    mp_printf(&mp_plat_print, "[LVML] Connecting to WiFi: %s\n", ssid);
    
    lvml_error_t result = network_manager_connect_wifi(ssid, password);
    if (result != LVML_OK) {
        mp_printf(&mp_plat_print, "[LVML] WiFi connection failed\n");
        return result;
    }
    
    mp_printf(&mp_plat_print, "[LVML] WiFi connected successfully\n");
    
    return LVML_OK;
}

bool lvml_is_ready(void) {
    return lvml_all_initialized && 
           lvml_core_is_initialized() && 
           memory_manager_is_initialized() && 
           mp_executor_is_available();
}
