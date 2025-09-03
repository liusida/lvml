// lvml MicroPython user C module
// Provides: lvml.hello(), lvml.init(), lvml.set_bg(), lvml.tick(), lvml.memory_info()
//           lvml.load_from_url(), lvml.load_from_xml(), lvml.connect_wifi(), lvml.is_ready()
//           lvml.get_version()

#include "py/runtime.h"
#include "py/mphal.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "lvml.h"
#include "driver/esp32_s3_box3_lcd.h"

static bool lvgl_initialized = false;
static esp_timer_handle_t lvgl_timer = NULL;

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
    
    mp_printf(&mp_plat_print, "Initializing LVML with ESP-IDF native LCD driver...\n");
    
    // Initialize ESP-IDF LCD driver for ESP32-S3-Box-3
    esp_err_t ret = esp32_s3_box3_lcd_init();
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "ERROR: ESP32-S3-Box-3 LCD initialization failed\n");
        return mp_const_none;
    }
    
    // Check PSRAM availability
    size_t psram_size = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t internal_size = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    mp_printf(&mp_plat_print, "Memory: PSRAM=%d bytes, Internal=%d bytes\n", psram_size, internal_size);
    
    // Initialize all LVML subsystems
    lvml_error_t lvml_result = lvml_init_all();
    if (lvml_result != LVML_OK) {
        mp_printf(&mp_plat_print, "ERROR: LVML initialization failed: %d\n", lvml_result);
        return mp_const_none;
    }
    
    // Set up display buffer (you may need to adjust this for your hardware)
    // Require PSRAM for display buffers; do not fall back to internal RAM
    #define BUF_ROWS 120
    size_t buffer_size = 320 * BUF_ROWS * sizeof(lv_color_t);

    if (psram_size == 0) {
        mp_printf(&mp_plat_print, "ERROR: PSRAM not available. Cannot allocate display buffers.\n");
        lvml_deinit_all();
        return mp_const_none;
    }

    // Allocate PSRAM with 8-bit capable capability for framebuffer usage
    const int psram_caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;
    lv_color_t *buf1 = (lv_color_t*)heap_caps_malloc(buffer_size, psram_caps);
    lv_color_t *buf2 = (lv_color_t*)heap_caps_malloc(buffer_size, psram_caps);

    if (buf1 == NULL || buf2 == NULL) {
        if (buf1) heap_caps_free(buf1);
        if (buf2) heap_caps_free(buf2);
        mp_printf(&mp_plat_print, "ERROR: Failed to allocate display buffers in PSRAM (size=%d bytes each).\n", (int)buffer_size);
        lvml_deinit_all();
        return mp_const_none;
    }
    mp_printf(&mp_plat_print, "Display buffers allocated in PSRAM (%d bytes each)\n", (int)buffer_size);
    
    // Create ESP-IDF LCD display
    lv_display_t *disp = esp32_s3_box3_lcd_create_display(320, 240);
    if (disp == NULL) {
        mp_printf(&mp_plat_print, "ERROR: ESP32-S3-Box-3 LCD display creation failed!\n");
        heap_caps_free(buf1);
        heap_caps_free(buf2);
        lvml_deinit_all();
        return mp_const_none;
    }
    
    // Set up display buffers
    lv_display_set_buffers(disp, buf1, buf2, buffer_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    mp_printf(&mp_plat_print, "ESP32-S3-Box-3 LCD display created successfully!\n");
    
    lvgl_initialized = true;
    mp_printf(&mp_plat_print, "LVML initialization complete\n");
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_init_obj, lvml_init);

static mp_obj_t lvml_set_bg(mp_obj_t color_obj) {
    if (!lvgl_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, "LVGL not initialized. Call lvml.init() first.");
    }
    
    // Parse color parameter (hex string like "0x000000" or "black")
    uint32_t color_hex = 0x000000; // Default to black
    
    if (mp_obj_is_str(color_obj)) {
        const char *color_str = mp_obj_str_get_str(color_obj);
        
        // Handle named colors
        if (strcmp(color_str, "black") == 0) {
            color_hex = 0x000000;
        } else if (strcmp(color_str, "white") == 0) {
            color_hex = 0xFFFFFF;
        } else if (strcmp(color_str, "red") == 0) {
            color_hex = 0xFF0000;
        } else if (strcmp(color_str, "green") == 0) {
            color_hex = 0x00FF00;
        } else if (strcmp(color_str, "blue") == 0) {
            color_hex = 0x0000FF;
        } else if (strcmp(color_str, "dark") == 0) {
            color_hex = 0x000000;
        } else {
            // Try to parse as hex string
            if (strncmp(color_str, "0x", 2) == 0) {
                color_hex = strtoul(color_str, NULL, 16);
            } else {
                color_hex = strtoul(color_str, NULL, 16);
            }
        }
    } else if (mp_obj_is_int(color_obj)) {
        // Handle integer color value
        color_hex = mp_obj_get_int(color_obj);
    } else {
        mp_raise_msg(&mp_type_ValueError, "Color must be a string (hex or name) or integer");
    }
    
    uint8_t r = (color_hex >> 16) & 0xFF;
    uint8_t g = (color_hex >> 8) & 0xFF;
    uint8_t b = color_hex & 0xFF;
    lv_color_t lv_color = lv_color_make(r, g, b);
    
    // Create a background object
    mp_printf(&mp_plat_print, "Creating background object...\n");
    lv_obj_t * bg = lv_obj_create(lv_screen_active());
    lv_obj_set_style_bg_color(bg, lv_color, 0);
    
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, 0);
    lv_obj_set_size(bg, LV_PCT(100), LV_PCT(100));
    lv_obj_center(bg);
    
    mp_printf(&mp_plat_print, "Background object created, triggering screen update...\n");
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(lvml_set_bg_obj, lvml_set_bg);

static mp_obj_t lvml_is_initialized(void) {
    return mp_obj_new_bool(lvgl_initialized);
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_is_initialized_obj, lvml_is_initialized);

static mp_obj_t lvml_memory_info(void) {
    mp_printf(&mp_plat_print, "=== Memory Information ===\n");
    
    size_t psram_total = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t internal_total = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    size_t psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t psram_largest = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    size_t internal_largest = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
    
    mp_printf(&mp_plat_print, "PSRAM:     Total=%8u bytes, Free=%8u bytes, Largest=%8u bytes\n", 
              (unsigned int)psram_total, (unsigned int)psram_free, (unsigned int)psram_largest);
    mp_printf(&mp_plat_print, "Internal:  Total=%8u bytes, Free=%8u bytes, Largest=%8u bytes\n", 
              (unsigned int)internal_total, (unsigned int)internal_free, (unsigned int)internal_largest);
    
    if (psram_total > 0) {
        int psram_used_pct = ((psram_total - psram_free) * 100) / psram_total;
        mp_printf(&mp_plat_print, "PSRAM usage: %d%% used\n", psram_used_pct);
    }
    
    if (internal_total > 0) {
        int internal_used_pct = ((internal_total - internal_free) * 100) / internal_total;
        mp_printf(&mp_plat_print, "Internal RAM usage: %d%% used\n", internal_used_pct);
    }
    
    size_t total_memory = psram_total + internal_total;
    size_t total_free = psram_free + internal_free;
    if (total_memory > 0) {
        int total_used_pct = ((total_memory - total_free) * 100) / total_memory;
        mp_printf(&mp_plat_print, "Total system memory: %d bytes, %d%% used\n", total_memory, total_used_pct);
    }
    
    mp_printf(&mp_plat_print, "========================\n");
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_memory_info_obj, lvml_memory_info);

static mp_obj_t lvml_tick(void) {
    if (!lvgl_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, "LVGL not initialized. Call lvml.init() first.");
    }
    
    lv_tick_inc(1);
    lv_timer_handler();
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_tick_obj, lvml_tick);

static mp_obj_t lvml_set_rotation(mp_obj_t rotation_obj) {
    if (!lvgl_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, "LVGL not initialized. Call lvml.init() first.");
    }
    
    int rotation = mp_obj_get_int(rotation_obj);
    
    // Validate rotation value (0, 1, 2, 3 for 0°, 90°, 180°, 270°)
    if (rotation < 0 || rotation > 3) {
        mp_raise_msg(&mp_type_ValueError, "Rotation must be 0, 1, 2, or 3 (0°, 90°, 180°, 270°)");
    }
    
    // Set rotation using ESP-IDF LCD driver
    lv_display_rotation_t lv_rotation;
    switch (rotation) {
        case 0: lv_rotation = LV_DISPLAY_ROTATION_0; break;
        case 1: lv_rotation = LV_DISPLAY_ROTATION_90; break;
        case 2: lv_rotation = LV_DISPLAY_ROTATION_180; break;
        case 3: lv_rotation = LV_DISPLAY_ROTATION_270; break;
        default: lv_rotation = LV_DISPLAY_ROTATION_0; break;
    }
    
    esp_err_t ret = esp32_s3_box3_lcd_set_rotation(lv_rotation);
    if (ret != ESP_OK) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to set display rotation");
    }
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(lvml_set_rotation_obj, lvml_set_rotation);

static mp_obj_t lvml_deinit(void) {
    if (!lvgl_initialized) {
        mp_printf(&mp_plat_print, "LVML not initialized\n");
        return mp_const_none;
    }
    
    if (lvgl_timer != NULL) {
        esp_timer_stop(lvgl_timer);
        esp_timer_delete(lvgl_timer);
        lvgl_timer = NULL;
    }
    
    // Deinitialize ESP-IDF LCD driver
    esp32_s3_box3_lcd_deinit();
    
    // Deinitialize all LVML subsystems
    lvml_deinit_all();
    
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
};
static MP_DEFINE_CONST_DICT(lvml_module_globals, lvml_module_globals_table);

const mp_obj_module_t lvml_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&lvml_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_lvml, lvml_user_cmodule);