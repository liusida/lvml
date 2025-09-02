// Enhanced lvml MicroPython user C module
// Provides: lvml.hello(), lvml.init(), lvml.set_dark_bg()

#include "py/runtime.h"
#include "py/mphal.h"

// LVGL includes
#include "lvgl.h"

// Global initialization state
static bool lvgl_initialized = false;

static mp_obj_t lvml_hello(void) {
    mp_printf(&mp_plat_print, "hello from lvml\n");
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_hello_obj, lvml_hello);

static mp_obj_t lvml_init(void) {
    if (lvgl_initialized) {
        mp_printf(&mp_plat_print, "LVGL already initialized\n");
        return mp_const_none;
    }
    
    mp_printf(&mp_plat_print, "Initializing LVGL...\n");
    
    // Initialize LVGL
    lv_init();
    
    // Set up display buffer (you may need to adjust this for your hardware)
    #define BUF_ROWS 120
    static lv_color_t buf1[320 * BUF_ROWS]; // Primary buffer
    static lv_color_t buf2[320 * BUF_ROWS]; // Secondary buffer
    
    // Create display (you'll need to implement flush callback for your hardware)
    lv_display_t *disp = lv_display_create(320, 240);
    if (disp == NULL) {
        mp_printf(&mp_plat_print, "ERROR: Display creation failed!\n");
        return mp_const_none;
    }
    
    // Set up display buffers (you'll need to implement flush callback)
    // lv_display_set_flush_cb(disp, your_flush_callback);
    lv_display_set_buffers(disp, buf1, buf2, 320 * BUF_ROWS * sizeof(lv_color_t),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    lvgl_initialized = true;
    mp_printf(&mp_plat_print, "LVGL initialization complete\n");
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_init_obj, lvml_init);

static mp_obj_t lvml_set_dark_bg(void) {
    if (!lvgl_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, "LVGL not initialized. Call lvml.init() first.");
    }
    
    mp_printf(&mp_plat_print, "Setting dark background...\n");
    
    // Create a dark background
    lv_obj_t * bg = lv_obj_create(lv_screen_active());
    lv_obj_set_style_bg_color(bg, lv_color_hex(0x000000), 0); // Black background
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, 0);
    lv_obj_set_size(bg, LV_PCT(100), LV_PCT(100));
    lv_obj_center(bg);
    
    mp_printf(&mp_plat_print, "Dark background set\n");
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_set_dark_bg_obj, lvml_set_dark_bg);

static mp_obj_t lvml_is_initialized(void) {
    return mp_obj_new_bool(lvgl_initialized);
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_is_initialized_obj, lvml_is_initialized);

static const mp_rom_map_elem_t lvml_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_lvml) },
    { MP_ROM_QSTR(MP_QSTR_hello), MP_ROM_PTR(&lvml_hello_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&lvml_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_dark_bg), MP_ROM_PTR(&lvml_set_dark_bg_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_initialized), MP_ROM_PTR(&lvml_is_initialized_obj) },
};
static MP_DEFINE_CONST_DICT(lvml_module_globals, lvml_module_globals_table);

const mp_obj_module_t lvml_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&lvml_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_lvml, lvml_user_cmodule);
