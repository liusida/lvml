/**
 * @file lvml_ui.h
 * @brief LVML UI object creation and management interfaces
 */

#ifndef LVML_UI_H
#define LVML_UI_H

#include "micropython/py/runtime.h"
#include "lvgl/lvgl.h"
#include "lvml_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Set background color of the active screen
 * @param color_hex RGB color value (0xRRGGBB format)
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_ui_set_background(uint32_t color_hex);

/**
 * Parse color string or integer to hex value
 * @param color_str color string (hex, named color) or NULL for integer parsing
 * @param color_int color integer value (used when color_str is NULL)
 * @param color_hex output hex value
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_ui_parse_color(const char* color_str, int color_int, uint32_t* color_hex);

/**
 * Create a rectangle object
 * @param x x position
 * @param y y position
 * @param width rectangle width
 * @param height rectangle height
 * @param color_hex fill color (0xRRGGBB format)
 * @param border_color_hex border color (0xRRGGBB format), 0 for no border
 * @param border_width border width, 0 for no border
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_ui_rect(int x, int y, int width, int height, uint32_t color_hex, uint32_t border_color_hex, int border_width);

/**
 * Create a button object
 * @param x x position
 * @param y y position
 * @param width button width
 * @param height button height
 * @param text button text
 * @param bg_color_hex background color (0xRRGGBB format)
 * @param text_color_hex text color (0xRRGGBB format)
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_ui_button(int x, int y, int width, int height, const char* text, uint32_t bg_color_hex, uint32_t text_color_hex);

/**
 * Create a text area object
 * @param x x position
 * @param y y position
 * @param width text area width
 * @param height text area height
 * @param placeholder placeholder text
 * @param bg_color_hex background color (0xRRGGBB format)
 * @param text_color_hex text color (0xRRGGBB format)
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_ui_textarea(int x, int y, int width, int height, const char* placeholder, uint32_t bg_color_hex, uint32_t text_color_hex);

/**
 * Display an image from raw PNG data
 * @param png_data raw PNG data bytes
 * @param data_size size of PNG data
 * @param x x position (optional, -1 for center)
 * @param y y position (optional, -1 for center)
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_ui_show_image_data(const uint8_t* png_data, size_t data_size, int x, int y);

/**
 * Clean up an image object and free its PSRAM memory
 * @param img image object to clean up
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t lvml_ui_cleanup_image(lv_obj_t* img);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LVML_UI_H*/
