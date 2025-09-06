#ifndef ESP32_S3_BOX3_TOUCH_H
#define ESP32_S3_BOX3_TOUCH_H

#include "esp_err.h"
#include "lvgl/lvgl.h"

// Function declarations for ESP32-S3-Box-3 GT911 Touch driver

/**
 * @brief Initialize the GT911 touch controller for ESP32-S3-Box-3
 * 
 * This function performs the complete initialization sequence for the GT911 touch controller,
 * including GPIO configuration, I2C setup, and the specific reset sequence required by the GT911.
 * The function handles the complex reset timing requirements and configures the touch controller
 * to work with the ESP32-S3-Box-3 hardware.
 * 
 * @return esp_err_t ESP_OK on success, error code on failure
 */
esp_err_t esp32_s3_box3_touch_init(void);

/**
 * @brief Deinitialize the GT911 touch controller
 * 
 * This function cleans up resources used by the touch controller, including deleting
 * the LVGL input device and uninstalling the I2C driver. It should be called when
 * the touch functionality is no longer needed.
 */
void esp32_s3_box3_touch_deinit(void);

/**
 * @brief Create an LVGL input device for touch input
 * 
 * This function creates and configures an LVGL input device that can be used to
 * receive touch events from the GT911 controller. The input device must be created
 * after the touch controller has been initialized.
 * 
 * @return lv_indev_t* Pointer to the created input device, or NULL on failure
 */
lv_indev_t *esp32_s3_box3_touch_create_indev(void);

/**
 * @brief Check if the touch controller is initialized
 * 
 * This function returns the current initialization state of the touch controller.
 * It can be used to verify that the touch system is ready before attempting to
 * create input devices or process touch events.
 * 
 * @return bool true if initialized, false otherwise
 */
bool esp32_s3_box3_touch_is_initialized(void);

#endif /* ESP32_S3_BOX3_TOUCH_H */
