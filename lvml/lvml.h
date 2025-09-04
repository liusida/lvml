/**
 * @file lvml.h
 * @brief Main LVML header file - includes all interfaces
 */

#ifndef LVML_H
#define LVML_H

// Core LVML functionality
#include "core/lvml_core.h"
#include "core/lvml_ui.h"

// XML parsing and processing
#include "xml/xml_parser.h"


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
 * Load UI from XML string
 * @param xml_data XML string
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_load_from_xml(const char* xml_data);


/**
 * Get LVML system status
 * @return true if all systems are ready, false otherwise
 */
bool lvml_is_ready(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LVML_H*/
