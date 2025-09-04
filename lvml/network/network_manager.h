/**
 * @file network_manager.h
 * @brief Network and WiFi management interface
 */

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "core/lvml_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      DEFINES
 *********************/

#define NETWORK_MAX_SSID_LENGTH 32
#define NETWORK_MAX_PASSWORD_LENGTH 64
#define NETWORK_MAX_RESPONSE_SIZE (1024 * 1024) // 1MB max response

/**********************
 *      TYPEDEFS
 **********************/

/**
 * WiFi connection status
 */
typedef enum {
    WIFI_STATUS_DISCONNECTED = 0,
    WIFI_STATUS_CONNECTING,
    WIFI_STATUS_CONNECTED,
    WIFI_STATUS_ERROR
} wifi_status_t;

/**
 * HTTP response structure
 */
typedef struct {
    int status_code;
    char* content;
    size_t content_length;
    char* content_type;
} http_response_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize network manager
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t network_manager_init(void);

/**
 * Deinitialize network manager
 */
void network_manager_deinit(void);

/**
 * Connect to WiFi network
 * @param ssid WiFi SSID
 * @param password WiFi password
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t network_manager_connect_wifi(const char* ssid, const char* password);

/**
 * Disconnect from WiFi
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t network_manager_disconnect_wifi(void);

/**
 * Get current WiFi status
 * @return WiFi status
 */
wifi_status_t network_manager_get_wifi_status(void);

/**
 * Fetch data from URL via HTTP GET
 * @param url URL to fetch
 * @param response Output response structure
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t network_manager_http_get(const char* url, http_response_t* response);

/**
 * Send HTTP POST request with JSON data
 * @param url URL to POST to
 * @param json_data JSON data to send
 * @param response Output response structure
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t network_manager_http_post(const char* url, const char* json_data, http_response_t* response);

/**
 * Free HTTP response
 * @param response Response to free
 */
void network_manager_free_response(http_response_t* response);

/**
 * Check if URL is valid
 * @param url URL to validate
 * @return true if valid, false otherwise
 */
bool network_manager_is_valid_url(const char* url);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*NETWORK_MANAGER_H*/
