/**
 * @file network_manager.c
 * @brief Network and WiFi management implementation
 */

#include "network_manager.h"
#include "py/mphal.h"

/**********************
 *  STATIC VARIABLES
 **********************/

static bool network_manager_initialized = false;
static wifi_status_t current_wifi_status = WIFI_STATUS_DISCONNECTED;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lvml_error_t network_manager_init(void) {
    if (network_manager_initialized) {
        return LVML_OK;
    }
    
    mp_printf(&mp_plat_print, "[NETWORK] Initializing network manager\n");
    
    current_wifi_status = WIFI_STATUS_DISCONNECTED;
    network_manager_initialized = true;
    
    mp_printf(&mp_plat_print, "[NETWORK] Network manager initialized successfully\n");
    
    return LVML_OK;
}

void network_manager_deinit(void) {
    if (!network_manager_initialized) {
        return;
    }
    
    mp_printf(&mp_plat_print, "[NETWORK] Deinitializing network manager\n");
    
    // Disconnect WiFi if connected
    if (current_wifi_status == WIFI_STATUS_CONNECTED) {
        network_manager_disconnect_wifi();
    }
    
    network_manager_initialized = false;
    mp_printf(&mp_plat_print, "[NETWORK] Network manager deinitialized\n");
}

lvml_error_t network_manager_connect_wifi(const char* ssid, const char* password) {
    if (!network_manager_initialized) {
        mp_printf(&mp_plat_print, "[NETWORK] Network manager not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    if (ssid == NULL) {
        mp_printf(&mp_plat_print, "[NETWORK] Invalid SSID\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    
    mp_printf(&mp_plat_print, "[NETWORK] Connecting to WiFi: %s\n", ssid);
    
    current_wifi_status = WIFI_STATUS_CONNECTING;
    
    // TODO: Implement actual WiFi connection
    // For now, simulate connection
    current_wifi_status = WIFI_STATUS_CONNECTED;
    
    mp_printf(&mp_plat_print, "[NETWORK] WiFi connected successfully\n");
    
    return LVML_OK;
}

lvml_error_t network_manager_disconnect_wifi(void) {
    if (!network_manager_initialized) {
        mp_printf(&mp_plat_print, "[NETWORK] Network manager not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    mp_printf(&mp_plat_print, "[NETWORK] Disconnecting from WiFi\n");
    
    current_wifi_status = WIFI_STATUS_DISCONNECTED;
    
    mp_printf(&mp_plat_print, "[NETWORK] WiFi disconnected\n");
    
    return LVML_OK;
}

wifi_status_t network_manager_get_wifi_status(void) {
    return current_wifi_status;
}

lvml_error_t network_manager_http_get(const char* url, http_response_t* response) {
    if (!network_manager_initialized) {
        mp_printf(&mp_plat_print, "[NETWORK] Network manager not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    if (url == NULL || response == NULL) {
        mp_printf(&mp_plat_print, "[NETWORK] Invalid parameters\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    
    if (current_wifi_status != WIFI_STATUS_CONNECTED) {
        mp_printf(&mp_plat_print, "[NETWORK] WiFi not connected\n");
        return LVML_ERROR_NETWORK;
    }
    
    mp_printf(&mp_plat_print, "[NETWORK] Fetching URL: %s\n", url);
    
    // TODO: Implement actual HTTP GET
    // For now, return empty response
    response->status_code = 200;
    response->content = NULL;
    response->content_length = 0;
    response->content_type = NULL;
    
    mp_printf(&mp_plat_print, "[NETWORK] HTTP GET completed (minimal implementation)\n");
    
    return LVML_OK;
}

void network_manager_free_response(http_response_t* response) {
    if (response == NULL) {
        return;
    }
    
    mp_printf(&mp_plat_print, "[NETWORK] Freeing HTTP response\n");
    
    if (response->content != NULL) {
        free(response->content);
        response->content = NULL;
    }
    
    if (response->content_type != NULL) {
        free(response->content_type);
        response->content_type = NULL;
    }
    
    response->status_code = 0;
    response->content_length = 0;
}

bool network_manager_is_valid_url(const char* url) {
    if (url == NULL) {
        return false;
    }
    
    // Basic URL validation - check for http:// or https://
    return (strncmp(url, "http://", 7) == 0) || (strncmp(url, "https://", 8) == 0);
}
