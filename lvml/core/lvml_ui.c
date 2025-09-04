/**
 * @file lvml_ui.c
 * @brief LVML UI object creation and management implementation
 */

#include "lvml_ui.h"
#include "lvml_core.h"
#include "py/mphal.h"

/**********************
 *  STATIC VARIABLES
 **********************/

static lv_obj_t* static_background = NULL;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lvml_error_t lvml_ui_set_background(uint32_t color_hex) {
    if (!lvml_core_is_initialized()) {
        mp_printf(&mp_plat_print, "[LVML] Core system not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    // Extract RGB components
    uint8_t r = (color_hex >> 16) & 0xFF;
    uint8_t g = (color_hex >> 8) & 0xFF;
    uint8_t b = color_hex & 0xFF;
    lv_color_t lv_color = lv_color_make(r, g, b);
    
    // Create background object if it doesn't exist
    if (static_background == NULL) {
        mp_printf(&mp_plat_print, "[LVML] Creating background object...\n");
        static_background = lv_obj_create(lv_screen_active());
        if (static_background == NULL) {
            mp_printf(&mp_plat_print, "[LVML] Failed to create background object\n");
            return LVML_ERROR_MEMORY;
        }
        
        // Set up background properties (only once)
        lv_obj_set_style_bg_opa(static_background, LV_OPA_COVER, 0);
        lv_obj_set_size(static_background, LV_PCT(100), LV_PCT(100));
        lv_obj_center(static_background);
    }
    
    // Update background color
    lv_obj_set_style_bg_color(static_background, lv_color, 0);
    mp_printf(&mp_plat_print, "[LVML] Background color updated\n");
    
    return LVML_OK;
}

lvml_error_t lvml_ui_parse_color(const char* color_str, int color_int, uint32_t* color_hex) {
    if (color_hex == NULL) {
        return LVML_ERROR_INVALID_PARAM;
    }
    
    if (color_str != NULL) {
        // Handle named colors
        if (strcmp(color_str, "black") == 0) {
            *color_hex = 0x000000;
        } else if (strcmp(color_str, "white") == 0) {
            *color_hex = 0xFFFFFF;
        } else if (strcmp(color_str, "red") == 0) {
            *color_hex = 0xFF0000;
        } else if (strcmp(color_str, "green") == 0) {
            *color_hex = 0x00FF00;
        } else if (strcmp(color_str, "blue") == 0) {
            *color_hex = 0x0000FF;
        } else if (strcmp(color_str, "dark") == 0) {
            *color_hex = 0x000000;
        } else {
            // Try to parse as hex string
            if (strncmp(color_str, "0x", 2) == 0) {
                // C-style hex: 0xFF0000
                *color_hex = strtoul(color_str, NULL, 16);
            } else if (strncmp(color_str, "#", 1) == 0) {
                // CSS-style hex: #FF0000
                *color_hex = strtoul(color_str + 1, NULL, 16);
            } else {
                // Plain hex: FF0000
                *color_hex = strtoul(color_str, NULL, 16);
            }
        }
    } else {
        // Handle integer color value
        *color_hex = (uint32_t)color_int;
    }
    
    return LVML_OK;
}

lvml_error_t lvml_ui_rect(int x, int y, int width, int height, uint32_t color_hex, uint32_t border_color_hex, int border_width) {
    if (!lvml_core_is_initialized()) {
        mp_printf(&mp_plat_print, "[LVML] Core system not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    // Validate parameters
    if (width <= 0 || height <= 0) {
        mp_printf(&mp_plat_print, "[LVML] Invalid rectangle dimensions: width=%d, height=%d\n", width, height);
        return LVML_ERROR_INVALID_PARAM;
    }
    
    if (border_width < 0) {
        mp_printf(&mp_plat_print, "[LVML] Invalid border width: %d\n", border_width);
        return LVML_ERROR_INVALID_PARAM;
    }
    
    // Create rectangle object
    lv_obj_t* rect = lv_obj_create(lv_screen_active());
    if (rect == NULL) {
        mp_printf(&mp_plat_print, "[LVML] Failed to create rectangle object\n");
        return LVML_ERROR_MEMORY;
    }
    
    // Set position and size
    lv_obj_set_pos(rect, x, y);
    lv_obj_set_size(rect, width, height);
    
    // Set fill color
    uint8_t r = (color_hex >> 16) & 0xFF;
    uint8_t g = (color_hex >> 8) & 0xFF;
    uint8_t b = color_hex & 0xFF;
    lv_color_t fill_color = lv_color_make(r, g, b);
    lv_obj_set_style_bg_color(rect, fill_color, 0);
    lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, 0);
    
    // Set border if specified
    if (border_width > 0 && border_color_hex != 0) {
        uint8_t br = (border_color_hex >> 16) & 0xFF;
        uint8_t bg = (border_color_hex >> 8) & 0xFF;
        uint8_t bb = border_color_hex & 0xFF;
        lv_color_t border_color = lv_color_make(br, bg, bb);
        
        lv_obj_set_style_border_color(rect, border_color, 0);
        lv_obj_set_style_border_width(rect, border_width, 0);
        lv_obj_set_style_border_opa(rect, LV_OPA_COVER, 0);
    } else {
        // No border
        lv_obj_set_style_border_width(rect, 0, 0);
    }
    
    // Remove default padding and make it a simple rectangle
    lv_obj_set_style_pad_all(rect, 0, 0);
    lv_obj_set_style_radius(rect, 0, 0); // Square corners
    
    mp_printf(&mp_plat_print, "[LVML] Rectangle created at (%d,%d) size %dx%d color 0x%06X\n", 
              x, y, width, height, color_hex);
    
    return LVML_OK;
}
