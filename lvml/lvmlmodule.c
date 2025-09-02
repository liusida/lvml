// Enhanced lvml MicroPython user C module
// Provides: lvml.hello(), lvml.init(), lvml.set_dark_bg()

#include "py/runtime.h"
#include "py/mphal.h"

// ESP-IDF includes for PSRAM
#include "esp_heap_caps.h"

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
    
    // Check PSRAM availability
    size_t psram_size = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t internal_size = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    mp_printf(&mp_plat_print, "Memory: PSRAM=%d bytes, Internal=%d bytes\n", psram_size, internal_size);
    
    // Initialize LVGL
    lv_init();
    
    // Set up display buffer (you may need to adjust this for your hardware)
    // Require PSRAM for display buffers; do not fall back to internal RAM
    #define BUF_ROWS 120
    size_t buffer_size = 320 * BUF_ROWS * sizeof(lv_color_t);

    if (psram_size == 0) {
        mp_printf(&mp_plat_print, "ERROR: PSRAM not available. Cannot allocate display buffers.\n");
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
        return mp_const_none;
    }
    mp_printf(&mp_plat_print, "Display buffers allocated in PSRAM (%d bytes each)\n", (int)buffer_size);
    
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

static mp_obj_t lvml_memory_info(void) {
    mp_printf(&mp_plat_print, "=== Memory Information ===\n");
    
    // Get total memory sizes
    size_t psram_total = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t internal_total = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    size_t dma_total = heap_caps_get_total_size(MALLOC_CAP_DMA);
    size_t executable_total = heap_caps_get_total_size(MALLOC_CAP_EXEC);
    
    // Get free memory sizes
    size_t psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t dma_free = heap_caps_get_free_size(MALLOC_CAP_DMA);
    size_t executable_free = heap_caps_get_free_size(MALLOC_CAP_EXEC);
    
    // Get largest free blocks
    size_t psram_largest = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    size_t internal_largest = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
    size_t dma_largest = heap_caps_get_largest_free_block(MALLOC_CAP_DMA);
    size_t executable_largest = heap_caps_get_largest_free_block(MALLOC_CAP_EXEC);
    
    mp_printf(&mp_plat_print, "PSRAM:     Total=%8d bytes, Free=%8d bytes, Largest=%8d bytes\n", 
              psram_total, psram_free, psram_largest);
    mp_printf(&mp_plat_print, "Internal:  Total=%8d bytes, Free=%8d bytes, Largest=%8d bytes\n", 
              internal_total, internal_free, internal_largest);
    mp_printf(&mp_plat_print, "DMA:       Total=%8d bytes, Free=%8d bytes, Largest=%8d bytes\n", 
              dma_total, dma_free, dma_largest);
    mp_printf(&mp_plat_print, "Executable:Total=%8d bytes, Free=%8d bytes, Largest=%8d bytes\n", 
              executable_total, executable_free, executable_largest);
    
    // Calculate percentages
    if (psram_total > 0) {
        int psram_used_pct = ((psram_total - psram_free) * 100) / psram_total;
        mp_printf(&mp_plat_print, "PSRAM usage: %d%% used\n", psram_used_pct);
    }
    
    if (internal_total > 0) {
        int internal_used_pct = ((internal_total - internal_free) * 100) / internal_total;
        mp_printf(&mp_plat_print, "Internal RAM usage: %d%% used\n", internal_used_pct);
    }
    
    // Show total system memory
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

static const mp_rom_map_elem_t lvml_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_lvml) },
    { MP_ROM_QSTR(MP_QSTR_hello), MP_ROM_PTR(&lvml_hello_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&lvml_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_dark_bg), MP_ROM_PTR(&lvml_set_dark_bg_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_initialized), MP_ROM_PTR(&lvml_is_initialized_obj) },
    { MP_ROM_QSTR(MP_QSTR_memory_info), MP_ROM_PTR(&lvml_memory_info_obj) },
};
static MP_DEFINE_CONST_DICT(lvml_module_globals, lvml_module_globals_table);

const mp_obj_module_t lvml_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&lvml_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_lvml, lvml_user_cmodule);
