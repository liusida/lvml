#ifndef ESP32_S3_BOX3_LCD_H
#define ESP32_S3_BOX3_LCD_H

#include "esp_err.h"
#include "lvgl.h"

// Function declarations for ESP32-S3-Box-3 LCD driver
esp_err_t esp32_s3_box3_lcd_init(void);
void esp32_s3_box3_lcd_deinit(void);
esp_err_t esp32_s3_box3_lcd_set_rotation(lv_display_rotation_t rotation);

// Display creation
lv_display_t * esp32_s3_box3_lcd_create_display(uint32_t width, uint32_t height);

// Test function to try different MADCTL values
esp_err_t esp32_s3_box3_lcd_test_rotation(uint8_t madctl_value);

#endif /* ESP32_S3_BOX3_LCD_H */
