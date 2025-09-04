/* src/lv_conf.h  (or move to include/) */
#ifndef LV_CONF_H
#define LV_CONF_H
// #warning "Using my own lv_conf.h"

#define LV_USE_GENERIC_MIPI 1
#define LV_USE_ILI9341 1

// LVGL Memory Management Configuration
#define LV_USE_STDLIB_MALLOC LV_STDLIB_BUILTIN
#define LV_MEM_SIZE (4 * 1024 * 1024U)  // 4MB for LVGL memory pool
#define LV_MEM_POOL_EXPAND_SIZE 0

// Let LVGL allocate memory from PSRAM
// Ensure the header is included before calling the allocator macro
#define LV_MEM_POOL_INCLUDE "esp_heap_caps.h"
// Allocate LVGL's memory pool from PSRAM with 8-bit access
#define LV_MEM_POOL_ALLOC(size) heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)

// Fix mp_printf %zu problem
#define LV_USE_STDLIB_STRING  LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 1

#define LV_FONT_MONTSERRAT_12	1
#define LV_FONT_MONTSERRAT_14	1
#define LV_FONT_MONTSERRAT_16	1
#define LV_FONT_MONTSERRAT_18	1
#define LV_FONT_MONTSERRAT_20	1
#define LV_FONT_MONTSERRAT_22	1
#define LV_FONT_MONTSERRAT_24	1
#define LV_FONT_MONTSERRAT_26	1
#define LV_FONT_MONTSERRAT_28	1
#define LV_FONT_MONTSERRAT_30	1
#define LV_FONT_MONTSERRAT_32	1
#define LV_FONT_MONTSERRAT_34	1
#define LV_FONT_MONTSERRAT_36	1
#define LV_FONT_MONTSERRAT_38	1
#define LV_FONT_MONTSERRAT_40	1
#define LV_FONT_MONTSERRAT_42	1
#define LV_FONT_MONTSERRAT_44	1
#define LV_FONT_MONTSERRAT_46	1
#define LV_FONT_MONTSERRAT_48	1

#define LV_FONT_DEFAULT        &lv_font_montserrat_14

#define CONFIG_LV_USE_XML 1
#define LV_USE_XML 1
#define LV_USE_DRAW_SW_COMPLEX_GRADIENTS 1

#define LV_USE_LODEPNG 1
#define LV_USE_FS_IF        1
#define LV_FS_IF_LITTLEFS  'S'    // choose the letter you want to use

#define LV_USE_LOG      1
#define LV_LOG_LEVEL    LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF 0


#define LV_USE_STDLIB_STRING  LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB

// ILI9341 Display Pin Configuration
#define ILI9341_SPI_HOST    SPI2_HOST
#define ILI9341_PIN_NUM_MISO -1  // Not used for display
#define ILI9341_PIN_NUM_MOSI 6
#define ILI9341_PIN_NUM_CLK  7
#define ILI9341_PIN_NUM_CS   5
#define ILI9341_PIN_NUM_DC   4
#define ILI9341_PIN_NUM_RST  48 // [Note: it is inverted, so keep low when working]
#define ILI9341_PIN_NUM_BCKL 47

// Display reset and backlight pins
#define DISPLAY_RESET_PIN   48
#define DISPLAY_BACKLIGHT_PIN 47


// Temporarily disable LVGL FreeRTOS integration due to compilation error
// #define LV_USE_OS LV_OS_FREERTOS
// #define LV_TICK_CUSTOM 1
// #define LV_TICK_CUSTOM_INCLUDE "freertos/FreeRTOS.h"
// #define LV_TICK_CUSTOM_SYS_TIME_EXPR (xTaskGetTickCount() * portTICK_PERIOD_MS)

#endif /* LV_CONF_H */
