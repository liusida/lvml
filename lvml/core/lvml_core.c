/**
 * @file lvml_core.c
 * @brief Core LVML functionality implementation
 */

#include "lvml_core.h"
#include "py/mphal.h"
#include "src/tick/lv_tick.h"
#include "esp_heap_caps.h"
#include "driver/esp32_s3_box3_lcd.h"
#include "esp_timer.h"
#include "lvml.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void custom_delay_ms(uint32_t ms);

/**********************
 *  STATIC VARIABLES
 **********************/

static bool lvml_initialized = false;
static esp_timer_handle_t lvgl_timer = NULL;
static lv_color_t *display_buf1 = NULL;
static lv_color_t *display_buf2 = NULL;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lvml_error_t lvml_core_init(void) {
    if (lvml_initialized) {
        mp_printf(&mp_plat_print, "[LVML] Already initialized\n");
        return LVML_OK;
    }
    
    mp_printf(&mp_plat_print, "[LVML] Initializing core system v%s with display...\n", LVML_VERSION);
    
    // Initialize LVGL
    lv_init();
    
    // Set up custom delay function to avoid LVGL tick dependency
    lv_delay_set_cb(custom_delay_ms);
    mp_printf(&mp_plat_print, "[LVML] Custom delay callback set up\n");
    
    // Initialize ESP-IDF LCD driver for ESP32-S3-Box-3
    esp_err_t ret = esp32_s3_box3_lcd_init();
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "[LVML] ERROR: ESP32-S3-Box-3 LCD initialization failed\n");
        return LVML_ERROR_INIT;
    }
    
    // Check PSRAM availability
    size_t psram_size = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t internal_size = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    mp_printf(&mp_plat_print, "[LVML] Memory: PSRAM=%d bytes, Internal=%d bytes\n", psram_size, internal_size);
    
    // Core LVML initialization complete - other subsystems will be initialized by lvml_init_all()
    
    // Set up display buffer (you may need to adjust this for your hardware)
    // Require PSRAM for display buffers; do not fall back to internal RAM
    #define BUF_ROWS 120
    size_t buffer_size = 320 * BUF_ROWS * sizeof(lv_color_t);

    if (psram_size == 0) {
        mp_printf(&mp_plat_print, "[LVML] ERROR: PSRAM not available. Cannot allocate display buffers.\n");
        lvml_deinit_all();
        esp32_s3_box3_lcd_deinit();
        return LVML_ERROR_MEMORY;
    }

    // Allocate PSRAM with 8-bit capable capability for framebuffer usage
    const int psram_caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;
    display_buf1 = (lv_color_t*)heap_caps_malloc(buffer_size, psram_caps);
    display_buf2 = (lv_color_t*)heap_caps_malloc(buffer_size, psram_caps);

    if (display_buf1 == NULL || display_buf2 == NULL) {
        if (display_buf1) heap_caps_free(display_buf1);
        if (display_buf2) heap_caps_free(display_buf2);
        display_buf1 = NULL;
        display_buf2 = NULL;
        mp_printf(&mp_plat_print, "[LVML] ERROR: Failed to allocate display buffers in PSRAM (size=%d bytes each).\n", (int)buffer_size);
        esp32_s3_box3_lcd_deinit();
        return LVML_ERROR_MEMORY;
    }
    mp_printf(&mp_plat_print, "[LVML] Display buffers allocated in PSRAM (%d bytes each)\n", (int)buffer_size);
    
    // Create ESP-IDF LCD display
    lv_display_t *disp = esp32_s3_box3_lcd_create_display(320, 240);
    if (disp == NULL) {
        mp_printf(&mp_plat_print, "[LVML] ERROR: ESP32-S3-Box-3 LCD display creation failed!\n");
        heap_caps_free(display_buf1);
        heap_caps_free(display_buf2);
        display_buf1 = NULL;
        display_buf2 = NULL;
        esp32_s3_box3_lcd_deinit();
        return LVML_ERROR_INIT;
    }
    
    // Set up display buffers
    lv_display_set_buffers(disp, display_buf1, display_buf2, buffer_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    mp_printf(&mp_plat_print, "[LVML] ESP32-S3-Box-3 LCD display created successfully!\n");
    
    lvml_initialized = true;
    mp_printf(&mp_plat_print, "[LVML] Initialization complete\n");
    
    return LVML_OK;
}

bool lvml_core_is_initialized(void) {
    return lvml_initialized;
}

const char* lvml_core_get_version(void) {
    return LVML_VERSION;
}

lvml_error_t lvml_core_print_memory_info(void) {
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
    
    return LVML_OK;
}


lvml_error_t lvml_core_set_rotation(int rotation) {
    if (!lvml_initialized) {
        mp_printf(&mp_plat_print, "[LVML] Core system not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    // Validate rotation value (0, 1, 2, 3 for 0°, 90°, 180°, 270°)
    if (rotation < 0 || rotation > 3) {
        mp_printf(&mp_plat_print, "[LVML] Invalid rotation value: %d\n", rotation);
        return LVML_ERROR_INVALID_PARAM;
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
        mp_printf(&mp_plat_print, "[LVML] Failed to set display rotation\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    
    mp_printf(&mp_plat_print, "[LVML] Display rotation set to %d degrees\n", rotation * 90);
    
    return LVML_OK;
}

lvml_error_t lvml_core_deinit(void) {
    if (!lvml_initialized) {
        mp_printf(&mp_plat_print, "[LVML] Core system not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    mp_printf(&mp_plat_print, "[LVML] Deinitializing core system\n");
    
    // Stop and delete timer if it exists
    if (lvgl_timer != NULL) {
        esp_timer_stop(lvgl_timer);
        esp_timer_delete(lvgl_timer);
        lvgl_timer = NULL;
        mp_printf(&mp_plat_print, "[LVML] Timer stopped and deleted\n");
    }
    
    // Free display buffers
    if (display_buf1 != NULL) {
        heap_caps_free(display_buf1);
        display_buf1 = NULL;
    }
    if (display_buf2 != NULL) {
        heap_caps_free(display_buf2);
        display_buf2 = NULL;
    }
    mp_printf(&mp_plat_print, "[LVML] Display buffers freed\n");
    
    // Deinitialize ESP-IDF LCD driver
    esp32_s3_box3_lcd_deinit();
    mp_printf(&mp_plat_print, "[LVML] ESP-IDF LCD driver deinitialized\n");
    
    lvml_initialized = false;
    mp_printf(&mp_plat_print, "[LVML] Core system deinitialized successfully\n");
    
    return LVML_OK;
}

lvml_error_t lvml_core_tick(void) {
    if (!lvml_initialized) {
        mp_printf(&mp_plat_print, "[LVML] Core system not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    lv_tick_inc(1);
    lv_timer_handler();
    
    return LVML_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Custom delay function that uses MicroPython's delay instead of LVGL's tick-based delay
 * @param ms        the number of milliseconds to delay
 */
static void custom_delay_ms(uint32_t ms) {
    mp_hal_delay_ms(ms);
}
