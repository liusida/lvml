// lvml MicroPython user C module
// Provides: lvml.hello(), lvml.init(), lvml.set_bg(), lvml.tick(), lvml.memory_info()
//           lvml.load_from_url(), lvml.load_from_xml(), lvml.connect_wifi(), lvml.is_ready()
//           lvml.get_version(), lvml.rect(), lvml.test_rotation()

#include "py/runtime.h"
#include "py/mphal.h"
#include "lvml.h"
#include "driver/esp32_s3_box3_lcd.h"

static bool lvgl_initialized = false;

static mp_obj_t lvml_hello(void) {
    mp_printf(&mp_plat_print, "hello from lvml\n");
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_hello_obj, lvml_hello);

static mp_obj_t lvml_init(void) {
    if (lvgl_initialized) {
        mp_printf(&mp_plat_print, "LVML already initialized\n");
        return mp_const_none;
    }
    
    // Use unified core init (includes display setup)
    lvml_error_t result = lvml_core_init();
    if (result != LVML_OK) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to initialize LVML");
    }
    
    lvgl_initialized = true;
    mp_printf(&mp_plat_print, "LVML initialization complete\n");
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_init_obj, lvml_init);

static mp_obj_t lvml_set_bg(mp_obj_t color_obj) {
    if (!lvgl_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, "LVGL not initialized. Call lvml.init() first.");
    }
    
    uint32_t color_hex = 0x000000; // Default to black
    lvml_error_t result;
    
    if (mp_obj_is_str(color_obj)) {
        const char *color_str = mp_obj_str_get_str(color_obj);
        result = lvml_ui_parse_color(color_str, 0, &color_hex);
    } else if (mp_obj_is_int(color_obj)) {
        int color_int = mp_obj_get_int(color_obj);
        result = lvml_ui_parse_color(NULL, color_int, &color_hex);
    } else {
        mp_raise_msg(&mp_type_ValueError, "Color must be a string (hex or name) or integer");
        return mp_const_none; // Never reached, but for completeness
    }
    
    if (result != LVML_OK) {
        mp_raise_msg(&mp_type_ValueError, "Invalid color format");
    }
    
    // Use UI function to set background
    result = lvml_ui_set_background(color_hex);
    if (result != LVML_OK) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to set background");
    }
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(lvml_set_bg_obj, lvml_set_bg);

static mp_obj_t lvml_is_initialized(void) {
    return mp_obj_new_bool(lvgl_initialized);
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_is_initialized_obj, lvml_is_initialized);

static mp_obj_t lvml_memory_info(void) {
    lvml_error_t result = lvml_core_print_memory_info();
    if (result != LVML_OK) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to get memory information");
    }
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_memory_info_obj, lvml_memory_info);

static mp_obj_t lvml_tick(void) {
    if (!lvgl_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, "LVGL not initialized. Call lvml.init() first.");
    }
    
    // Use core function to process tick
    lvml_error_t result = lvml_core_tick();
    if (result != LVML_OK) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to process LVGL tick");
    }
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_tick_obj, lvml_tick);

static mp_obj_t lvml_set_rotation(mp_obj_t rotation_obj) {
    if (!lvgl_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, "LVGL not initialized. Call lvml.init() first.");
    }
    
    int rotation = mp_obj_get_int(rotation_obj);
    
    // Use core function to set rotation
    lvml_error_t result = lvml_core_set_rotation(rotation);
    if (result != LVML_OK) {
        if (result == LVML_ERROR_INVALID_PARAM) {
            mp_raise_msg(&mp_type_ValueError, "Rotation must be 0, 1, 2, or 3 (0°, 90°, 180°, 270°)");
        } else {
            mp_raise_msg(&mp_type_RuntimeError, "Failed to set display rotation");
        }
    }
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(lvml_set_rotation_obj, lvml_set_rotation);

static mp_obj_t lvml_deinit(void) {
    if (!lvgl_initialized) {
        mp_printf(&mp_plat_print, "LVML not initialized\n");
        return mp_const_none;
    }
    
    // Use core function to deinitialize
    lvml_error_t result = lvml_core_deinit();
    if (result != LVML_OK) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to deinitialize LVML");
    }
    
    lvgl_initialized = false;
    mp_printf(&mp_plat_print, "LVML deinitialized\n");
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_deinit_obj, lvml_deinit);

// New function to load UI from URL
static mp_obj_t lvml_load_from_url_mp(mp_obj_t url_obj) {
    if (!lvgl_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, "LVML not initialized. Call lvml.init() first.");
    }
    
    const char* url = mp_obj_str_get_str(url_obj);
    mp_printf(&mp_plat_print, "Loading UI from URL: %s\n", url);
    
    lvml_error_t result = lvml_load_from_url(url);
    if (result != LVML_OK) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to load UI from URL");
    }
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(lvml_load_from_url_obj, lvml_load_from_url_mp);

// New function to load UI from XML string
static mp_obj_t lvml_load_from_xml_mp(mp_obj_t xml_obj) {
    if (!lvgl_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, "LVML not initialized. Call lvml.init() first.");
    }
    
    const char* xml_data = mp_obj_str_get_str(xml_obj);
    mp_printf(&mp_plat_print, "Loading UI from XML data\n");
    
    lvml_error_t result = lvml_load_from_xml(xml_data);
    if (result != LVML_OK) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to load UI from XML");
    }
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(lvml_load_from_xml_obj, lvml_load_from_xml_mp);

// New function to connect to WiFi
static mp_obj_t lvml_connect_wifi_mp(mp_obj_t ssid_obj, mp_obj_t password_obj) {
    if (!lvgl_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, "LVML not initialized. Call lvml.init() first.");
    }
    
    const char* ssid = mp_obj_str_get_str(ssid_obj);
    const char* password = mp_obj_str_get_str(password_obj);
    
    mp_printf(&mp_plat_print, "Connecting to WiFi: %s\n", ssid);
    
    lvml_error_t result = lvml_connect_wifi(ssid, password);
    if (result != LVML_OK) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to connect to WiFi");
    }
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(lvml_connect_wifi_obj, lvml_connect_wifi_mp);

// New function to get LVML status
static mp_obj_t lvml_is_ready_mp(void) {
    return mp_obj_new_bool(lvml_is_ready());
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_is_ready_obj, lvml_is_ready_mp);

// New function to get LVML version
static mp_obj_t lvml_get_version(void) {
    const char* version = lvml_core_get_version();
    return mp_obj_new_str(version, strlen(version));
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_get_version_obj, lvml_get_version);

// New function to create a rectangle
static mp_obj_t lvml_rect_mp(size_t n_args, const mp_obj_t *args) {
    if (!lvgl_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, "LVML not initialized. Call lvml.init() first.");
    }
    
    // Validate argument count
    if (n_args != 7) {
        mp_raise_msg(&mp_type_TypeError, "rect() takes exactly 7 arguments");
    }
    
    // Get parameters
    int x = mp_obj_get_int(args[0]);
    int y = mp_obj_get_int(args[1]);
    int width = mp_obj_get_int(args[2]);
    int height = mp_obj_get_int(args[3]);
    int border_width = mp_obj_get_int(args[6]);
    
    // Parse fill color
    uint32_t color_hex = 0x000000; // Default to black
    lvml_error_t result;
    
    if (mp_obj_is_str(args[4])) {
        const char *color_str = mp_obj_str_get_str(args[4]);
        result = lvml_ui_parse_color(color_str, 0, &color_hex);
    } else if (mp_obj_is_int(args[4])) {
        int color_int = mp_obj_get_int(args[4]);
        result = lvml_ui_parse_color(NULL, color_int, &color_hex);
    } else {
        mp_raise_msg(&mp_type_ValueError, "Color must be a string (hex or name) or integer");
        return mp_const_none;
    }
    
    if (result != LVML_OK) {
        mp_raise_msg(&mp_type_ValueError, "Invalid fill color format");
    }
    
    // Parse border color
    uint32_t border_color_hex = 0x000000; // Default to black
    if (mp_obj_is_str(args[5])) {
        const char *border_color_str = mp_obj_str_get_str(args[5]);
        result = lvml_ui_parse_color(border_color_str, 0, &border_color_hex);
    } else if (mp_obj_is_int(args[5])) {
        int border_color_int = mp_obj_get_int(args[5]);
        result = lvml_ui_parse_color(NULL, border_color_int, &border_color_hex);
    } else {
        mp_raise_msg(&mp_type_ValueError, "Border color must be a string (hex or name) or integer");
        return mp_const_none;
    }
    
    if (result != LVML_OK) {
        mp_raise_msg(&mp_type_ValueError, "Invalid border color format");
    }
    
    // Create rectangle
    result = lvml_ui_rect(x, y, width, height, color_hex, border_color_hex, border_width);
    if (result != LVML_OK) {
        if (result == LVML_ERROR_INVALID_PARAM) {
            mp_raise_msg(&mp_type_ValueError, "Invalid rectangle parameters");
        } else {
            mp_raise_msg(&mp_type_RuntimeError, "Failed to create rectangle");
        }
    }
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lvml_rect_obj, 7, 7, lvml_rect_mp);

// Test function to try different MADCTL values for debugging
static mp_obj_t lvml_test_rotation_mp(mp_obj_t madctl_value_obj) {
    if (!lvgl_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, "LVML not initialized. Call lvml.init() first.");
    }
    
    int madctl_value = mp_obj_get_int(madctl_value_obj);
    if (madctl_value < 0 || madctl_value > 255) {
        mp_raise_msg(&mp_type_ValueError, "MADCTL value must be between 0 and 255");
    }
    
    // Call the test function
    esp_err_t result = esp32_s3_box3_lcd_test_rotation((uint8_t)madctl_value);
    if (result != ESP_OK) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to test rotation");
    }
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(lvml_test_rotation_obj, lvml_test_rotation_mp);

// Simple test function to check if display is working
static mp_obj_t lvml_test_display_mp(void) {
    if (!lvgl_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, "LVML not initialized. Call lvml.init() first.");
    }
    
    mp_printf(&mp_plat_print, "Testing display with white background...\n");
    
    // Set white background
    lvml_error_t result = lvml_ui_set_background(0xFFFFFF);
    if (result != LVML_OK) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to set background");
    }
    
    // Create a red rectangle
    result = lvml_ui_rect(50, 50, 100, 100, 0xFF0000, 0x000000, 0);
    if (result != LVML_OK) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to create test rectangle");
    }
    
    mp_printf(&mp_plat_print, "Display test complete - you should see white background with red rectangle\n");
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_test_display_obj, lvml_test_display_mp);

static const mp_rom_map_elem_t lvml_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_lvml) },
    { MP_ROM_QSTR(MP_QSTR_hello), MP_ROM_PTR(&lvml_hello_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&lvml_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&lvml_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_bg), MP_ROM_PTR(&lvml_set_bg_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_rotation), MP_ROM_PTR(&lvml_set_rotation_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_initialized), MP_ROM_PTR(&lvml_is_initialized_obj) },
    { MP_ROM_QSTR(MP_QSTR_memory_info), MP_ROM_PTR(&lvml_memory_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_tick), MP_ROM_PTR(&lvml_tick_obj) },
    { MP_ROM_QSTR(MP_QSTR_load_from_url), MP_ROM_PTR(&lvml_load_from_url_obj) },
    { MP_ROM_QSTR(MP_QSTR_load_from_xml), MP_ROM_PTR(&lvml_load_from_xml_obj) },
    { MP_ROM_QSTR(MP_QSTR_connect_wifi), MP_ROM_PTR(&lvml_connect_wifi_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_ready), MP_ROM_PTR(&lvml_is_ready_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_version), MP_ROM_PTR(&lvml_get_version_obj) },
    { MP_ROM_QSTR(MP_QSTR_rect), MP_ROM_PTR(&lvml_rect_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_rotation), MP_ROM_PTR(&lvml_test_rotation_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_display), MP_ROM_PTR(&lvml_test_display_obj) },
};
static MP_DEFINE_CONST_DICT(lvml_module_globals, lvml_module_globals_table);

const mp_obj_module_t lvml_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&lvml_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_lvml, lvml_user_cmodule);