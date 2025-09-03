/**
 * @file lvml_core.c
 * @brief Core LVML functionality implementation
 */

#include "lvml_core.h"
#include "py/mphal.h"
#include "src/tick/lv_tick.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void custom_delay_ms(uint32_t ms);

/**********************
 *  STATIC VARIABLES
 **********************/

static bool lvml_initialized = false;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lvml_error_t lvml_core_init(void) {
    if (lvml_initialized) {
        return LVML_OK;
    }
    
    mp_printf(&mp_plat_print, "[LVML] Initializing core system v%s\n", LVML_VERSION);
    
    // Initialize LVGL
    lv_init();
    
    // Set up custom delay function to avoid LVGL tick dependency
    lv_delay_set_cb(custom_delay_ms);
    mp_printf(&mp_plat_print, "[LVML] Custom delay callback set up\n");
    
    lvml_initialized = true;
    mp_printf(&mp_plat_print, "[LVML] Core system initialized successfully\n");
    
    return LVML_OK;
}

void lvml_core_deinit(void) {
    if (!lvml_initialized) {
        return;
    }
    
    mp_printf(&mp_plat_print, "[LVML] Deinitializing core system\n");
    
    lvml_initialized = false;
    mp_printf(&mp_plat_print, "[LVML] Core system deinitialized\n");
}

bool lvml_core_is_initialized(void) {
    return lvml_initialized;
}

const char* lvml_core_get_version(void) {
    return LVML_VERSION;
}

void lvml_core_set_delay_callback(void (*delay_cb)(uint32_t ms)) {
    if (delay_cb != NULL) {
        lv_delay_set_cb(delay_cb);
        mp_printf(&mp_plat_print, "[LVML] Custom delay callback set\n");
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Custom delay function that uses MicroPython's delay instead of LVGL's tick-based delay
 * @param ms        the number of milliseconds to delay
 */
static void custom_delay_ms(uint32_t ms) {
    mp_printf(&mp_plat_print, "[DEBUG] custom_delay_ms: Delaying %d ms\n", ms);
    mp_hal_delay_ms(ms);
    mp_printf(&mp_plat_print, "[DEBUG] custom_delay_ms: Delay completed\n");
}
