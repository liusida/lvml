/**
 * @file xml_parser.h
 * @brief XML parsing and processing interface
 */

#ifndef XML_PARSER_H
#define XML_PARSER_H

#include "core/lvml_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      DEFINES
 *********************/

#define XML_MAX_TAG_LENGTH 64
#define XML_MAX_ATTRIBUTE_LENGTH 128
#define XML_MAX_SCRIPT_LENGTH 4096

/**********************
 *      TYPEDEFS
 **********************/

/**
 * XML element structure
 */
typedef struct {
    char tag[XML_MAX_TAG_LENGTH];
    char attributes[XML_MAX_ATTRIBUTE_LENGTH];
    char* content;
    struct xml_element_t* children;
    struct xml_element_t* next;
} xml_element_t;

/**
 * MicroPython script found in XML
 */
typedef struct {
    char* script_content;
    char* target_object;
    char* event_type;
} xml_script_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize XML parser
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t xml_parser_init(void);

/**
 * Deinitialize XML parser
 */
void xml_parser_deinit(void);

/**
 * Parse XML string and create LVGL UI
 * @param xml_data XML string to parse
 * @param ui Output UI structure
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t xml_parser_parse(const char* xml_data, lvml_ui_t* ui);

/**
 * Free XML UI structure
 * @param ui UI structure to free
 */
void xml_parser_free_ui(lvml_ui_t* ui);

/**
 * Extract MicroPython scripts from XML
 * @param xml_data XML string to parse
 * @param scripts Output array of scripts
 * @param script_count Number of scripts found
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t xml_parser_extract_scripts(const char* xml_data, xml_script_t** scripts, size_t* script_count);

/**
 * Free script array
 * @param scripts Script array to free
 * @param script_count Number of scripts
 */
void xml_parser_free_scripts(xml_script_t* scripts, size_t script_count);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*XML_PARSER_H*/
