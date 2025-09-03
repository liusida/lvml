/**
 * @file lvml.h
 * @brief Main LVML header file - includes all interfaces
 */

#ifndef LVML_H
#define LVML_H

// Core LVML functionality
#include "core/lvml_core.h"

// XML parsing and processing
#include "xml/xml_parser.h"

// Network and WiFi management
#include "network/network_manager.h"

// MicroPython script execution
#include "micropython/mp_executor.h"

// Utility functions
#include "utils/memory_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize all LVML subsystems
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_init_all(void);

/**
 * Deinitialize all LVML subsystems
 */
void lvml_deinit_all(void);

/**
 * Load UI from XML URL
 * @param url URL to fetch XML from
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_load_from_url(const char* url);

/**
 * Load UI from XML string
 * @param xml_data XML string
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_load_from_xml(const char* xml_data);

/**
 * Connect to WiFi network
 * @param ssid WiFi SSID
 * @param password WiFi password
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_connect_wifi(const char* ssid, const char* password);

/**
 * Get LVML system status
 * @return true if all systems are ready, false otherwise
 */
bool lvml_is_ready(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LVML_H*/
