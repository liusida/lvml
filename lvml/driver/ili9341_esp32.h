#ifndef ILI9341_ESP32_H
#define ILI9341_ESP32_H

#include "esp_err.h"
#include "driver/spi_master.h"
#include "lvgl.h"

// Function declarations for ESP-IDF specific implementation
esp_err_t ili9341_esp32_init(void);
void ili9341_esp32_deinit(void);
void ili9341_esp32_reset(void);
void ili9341_esp32_backlight_on(void);
void ili9341_esp32_backlight_off(void);

// LVGL callback functions for the official ILI9341 driver
void ili9341_send_cmd_cb(lv_display_t * disp, const uint8_t * cmd, size_t cmd_size, 
                        const uint8_t * param, size_t param_size);
void ili9341_send_color_cb(lv_display_t * disp, const uint8_t * cmd, size_t cmd_size,
                          uint8_t * param, size_t param_size);

// Custom display creation and flush callback
lv_display_t * ili9341_esp32_create_display(uint32_t width, uint32_t height);

// Display rotation control
void ili9341_esp32_set_rotation(lv_display_t * disp, lv_display_rotation_t rotation);

#endif /* ILI9341_ESP32_H */