/**
 * @file lvml_ui.c
 * @brief LVML UI object creation and management implementation
 */

#include "lvml_ui.h"
#include "lvml_core.h"
#include "micropython/py/mphal.h"
#include "lvgl/src/draw/lv_image_dsc.h"
#include "esp_heap_caps.h"
#include <string.h>


/**********************
 *  STATIC VARIABLES
 **********************/

// No static variables needed for background

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lvml_error_t lvml_ui_set_background(uint32_t color_hex) {
    if (!lvml_core_is_initialized()) {
        return LVML_ERROR_INIT;
    }
    
    // Extract RGB components
    uint8_t r = (color_hex >> 16) & 0xFF;
    uint8_t g = (color_hex >> 8) & 0xFF;
    uint8_t b = color_hex & 0xFF;
    lv_color_t lv_color = lv_color_make(r, g, b);
    
    // Set background color directly on the active screen
    lv_obj_t *current_screen = lv_screen_active();
    if (current_screen == NULL) {
        mp_printf(&mp_plat_print, "Error: No active screen\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    lv_obj_set_style_bg_color(current_screen, lv_color, 0);
    lv_obj_set_style_bg_opa(current_screen, LV_OPA_COVER, 0);
    
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
        return LVML_ERROR_INIT;
    }
    
    // Validate parameters
    if (width <= 0 || height <= 0) {
        return LVML_ERROR_INVALID_PARAM;
    }
    
    if (border_width < 0) {
        return LVML_ERROR_INVALID_PARAM;
    }
    
    // Create rectangle object
    lv_obj_t* rect = lv_obj_create(lv_screen_active());
    if (rect == NULL) {
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
    
    return LVML_OK;
}

lvml_error_t lvml_ui_button(int x, int y, int width, int height, const char* text, uint32_t bg_color_hex, uint32_t text_color_hex) {
    if (!lvml_core_is_initialized()) {
        return LVML_ERROR_INIT;
    }
    
    // Validate parameters
    if (width <= 0 || height <= 0) {
        return LVML_ERROR_INVALID_PARAM;
    }
    
    if (text == NULL) {
        return LVML_ERROR_INVALID_PARAM;
    }
    
    // Create button object
    lv_obj_t* btn = lv_button_create(lv_screen_active());
    if (btn == NULL) {
        return LVML_ERROR_MEMORY;
    }
    
    // Set position and size
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, width, height);
    
    // Set background color
    uint8_t r = (bg_color_hex >> 16) & 0xFF;
    uint8_t g = (bg_color_hex >> 8) & 0xFF;
    uint8_t b = bg_color_hex & 0xFF;
    lv_color_t bg_color = lv_color_make(r, g, b);
    lv_obj_set_style_bg_color(btn, bg_color, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    
    // Add text label
    lv_obj_t* label = lv_label_create(btn);
    if (label == NULL) {
        lv_obj_delete(btn);
        return LVML_ERROR_MEMORY;
    }
    
    lv_label_set_text(label, text);
    lv_obj_center(label);
    
    // Set text color
    uint8_t tr = (text_color_hex >> 16) & 0xFF;
    uint8_t tg = (text_color_hex >> 8) & 0xFF;
    uint8_t tb = text_color_hex & 0xFF;
    lv_color_t text_color = lv_color_make(tr, tg, tb);
    lv_obj_set_style_text_color(label, text_color, 0);
    
    return LVML_OK;
}

lvml_error_t lvml_ui_textarea(int x, int y, int width, int height, const char* placeholder, uint32_t bg_color_hex, uint32_t text_color_hex) {
    if (!lvml_core_is_initialized()) {
        return LVML_ERROR_INIT;
    }
    
    // Validate parameters
    if (width <= 0 || height <= 0) {
        return LVML_ERROR_INVALID_PARAM;
    }
    
    // Create text area object
    lv_obj_t* ta = lv_textarea_create(lv_screen_active());
    if (ta == NULL) {
        return LVML_ERROR_MEMORY;
    }
    
    // Set position and size
    lv_obj_set_pos(ta, x, y);
    lv_obj_set_size(ta, width, height);
    
    // Set background color
    uint8_t r = (bg_color_hex >> 16) & 0xFF;
    uint8_t g = (bg_color_hex >> 8) & 0xFF;
    uint8_t b = bg_color_hex & 0xFF;
    lv_color_t bg_color = lv_color_make(r, g, b);
    lv_obj_set_style_bg_color(ta, bg_color, 0);
    lv_obj_set_style_bg_opa(ta, LV_OPA_COVER, 0);
    
    // Set text color
    uint8_t tr = (text_color_hex >> 16) & 0xFF;
    uint8_t tg = (text_color_hex >> 8) & 0xFF;
    uint8_t tb = text_color_hex & 0xFF;
    lv_color_t text_color = lv_color_make(tr, tg, tb);
    lv_obj_set_style_text_color(ta, text_color, 0);
    
    // Set placeholder text if provided
    if (placeholder != NULL) {
        lv_textarea_set_placeholder_text(ta, placeholder);
    }
    
    // Enable text input
    lv_obj_add_state(ta, LV_STATE_FOCUSED);
    
    return LVML_OK;
}

lvml_error_t lvml_ui_show_image_data(const uint8_t* png_data, size_t data_size, int x, int y) {
    if (!lvml_core_is_initialized()) {
        return LVML_ERROR_INIT;
    }
    
    if (png_data == NULL || data_size == 0) {
        return LVML_ERROR_INVALID_PARAM;
    }
    
    // Create image object
    lv_obj_t* img = lv_image_create(lv_screen_active());
    if (img == NULL) {
        return LVML_ERROR_MEMORY;
    }
    
    // Create a proper image descriptor for PNG data
    lv_image_dsc_t *imgDesc = (lv_image_dsc_t*)malloc(sizeof(lv_image_dsc_t));
    if (!imgDesc) {
        mp_printf(&mp_plat_print, "Failed to allocate memory for image descriptor");
        return LVML_ERROR_MEMORY;
    }
    
    // Initialize the descriptor with PNG data
    memset(imgDesc, 0, sizeof(lv_image_dsc_t));
    imgDesc->data = png_data;
    imgDesc->data_size = data_size;
    
    // Set the image source - this will be detected as LV_IMAGE_SRC_VARIABLE
    lv_image_set_src(img, imgDesc);
    
    // Set position
    if (x == -1 || y == -1) {
        // Center the image
        lv_obj_center(img);
    } else {
        lv_obj_set_pos(img, x, y);
    }

    return LVML_OK;
}
