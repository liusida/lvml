/**
 * @file esp32_s3_box3_touch.c
 * @brief ESP32-S3-Box-3 Touch Driver using GT911 Controller
 * 
 * This file provides the LVGL input device interface for the GT911 touch controller
 * on the ESP32-S3-Box-3. It uses the GT911 driver to handle low-level communication
 * and provides a clean interface for LVGL touch input.
 */

#include "esp32_s3_box3_touch.h"
#include "GT911.h"
#include "micropython/py/mphal.h"

// GT911 Configuration for ESP32-S3-Box-3
#define GT911_I2C_ADDR 0x5D  // Default I2C address
#define GT911_I2C_PORT I2C_NUM_0
#define GT911_SDA_PIN 8      // From board config
#define GT911_SCL_PIN 18     // From board config
#define GT911_INT_PIN 3      // Interrupt pin
#define GT911_RST_PIN 48     // Reset pin
#define GT911_I2C_FREQ 100000 // 100kHz

// Static variables
static bool touch_initialized = false;
static lv_indev_t *touch_indev = NULL;

/**
 * @brief Initialize the GT911 touch controller for ESP32-S3-Box-3
 * 
 * This function initializes the GT911 touch controller using the dedicated GT911 driver.
 * It configures the I2C communication, GPIO pins, and performs the necessary reset
 * sequence for the ESP32-S3-Box-3 hardware.
 * 
 * @return esp_err_t ESP_OK on success, error code on failure
 */
esp_err_t esp32_s3_box3_touch_init(void) {
    if (touch_initialized) {
        return ESP_OK;
    }
    
    mp_printf(&mp_plat_print, "[GT911] Initializing GT911 touch controller\n");
    
    // Initialize GT911 driver
    bool success = gt911_begin(
        GT911_INT_PIN,      // Interrupt pin
        GT911_RST_PIN,      // Reset pin
        GT911_I2C_ADDR,     // I2C address
        GT911_I2C_PORT,     // I2C port
        GT911_SDA_PIN,      // SDA pin
        GT911_SCL_PIN,      // SCL pin
        GT911_I2C_FREQ      // I2C frequency
    );
    
    if (!success) {
        mp_printf(&mp_plat_print, "[GT911] Failed to initialize GT911\n");
        return ESP_FAIL;
    }
    
    // Read device info for debugging
    GTInfo* info = gt911_read_info();
    if (info != NULL) {
        mp_printf(&mp_plat_print, "[GT911] Product ID: %.4s, Resolution: %dx%d\n", 
                  info->productId, info->xResolution, info->yResolution);
    }
    
    touch_initialized = true;
    mp_printf(&mp_plat_print, "[GT911] Touch controller initialized successfully\n");
    
    return ESP_OK;
}

/**
 * @brief LVGL input device read callback for touch input
 * 
 * This function is called by LVGL when it needs to read touch input data. It uses
 * the GT911 driver to check for touch events and converts the touch data into
 * LVGL's input format. The function maintains the last touch position to provide
 * proper touch release events when the user lifts their finger from the screen.
 * 
 * @param indev Pointer to the LVGL input device (unused)
 * @param data Pointer to LVGL input data structure to fill with touch information
 */
static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
    static GTPoint last_touch = {0};
    static bool last_touch_valid = false;
    
    // Check for touch events using polling mode
    uint8_t touch_count = gt911_touched(GT911_MODE_POLLING);
    
    if (touch_count > 0) {
        // Get the first touch point (we only handle single touch for now)
        GTPoint current_touch = gt911_get_point(0);
        
        // Scale coordinates to display resolution (320x240)
        // GT911 typically reports coordinates in its native resolution
        GTInfo* info = gt911_read_info();
        if (info != NULL && info->xResolution > 0 && info->yResolution > 0) {
            data->point.x = (current_touch.x * 320) / info->xResolution;
            data->point.y = (current_touch.y * 240) / info->yResolution;
        } else {
            // Fallback scaling if info is not available
            data->point.x = (current_touch.x * 320) / 1024;
            data->point.y = (current_touch.y * 240) / 1024;
        }
        
        // Clamp coordinates to display bounds
        if (data->point.x >= 320) data->point.x = 319;
        if (data->point.y >= 240) data->point.y = 239;
        
        data->state = LV_INDEV_STATE_PRESSED;
        last_touch = current_touch;
        last_touch_valid = true;
        
        mp_printf(&mp_plat_print, "[TOUCH] Pressed: x=%d, y=%d, id=%d\n", 
                  data->point.x, data->point.y, current_touch.trackId);
    } else {
        // No touch detected
        if (last_touch_valid) {
            // Use last known position for release event
            GTInfo* info = gt911_read_info();
            if (info != NULL && info->xResolution > 0 && info->yResolution > 0) {
                data->point.x = (last_touch.x * 320) / info->xResolution;
                data->point.y = (last_touch.y * 240) / info->yResolution;
            } else {
                data->point.x = (last_touch.x * 320) / 1024;
                data->point.y = (last_touch.y * 240) / 1024;
            }
            
            // Clamp coordinates to display bounds
            if (data->point.x >= 320) data->point.x = 319;
            if (data->point.y >= 240) data->point.y = 239;
        } else {
            // No previous touch, use center
            data->point.x = 160;
            data->point.y = 120;
        }
        
        data->state = LV_INDEV_STATE_RELEASED;
        last_touch_valid = false;
    }
}

/**
 * @brief Create an LVGL input device for touch input
 * 
 * This function creates and configures an LVGL input device that can be used to
 * receive touch events from the GT911 controller. The input device must be created
 * after the touch controller has been initialized.
 * 
 * @return lv_indev_t* Pointer to the created input device, or NULL on failure
 */
lv_indev_t *esp32_s3_box3_touch_create_indev(void) {
    if (!touch_initialized) {
        mp_printf(&mp_plat_print, "[GT911] Touch controller not initialized\n");
        return NULL;
    }
    
    touch_indev = lv_indev_create();
    if (touch_indev == NULL) {
        mp_printf(&mp_plat_print, "[GT911] Failed to create LVGL input device\n");
        return NULL;
    }
    
    // Configure the input device
    lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch_indev, touchpad_read);
    
    mp_printf(&mp_plat_print, "[GT911] LVGL input device created successfully\n");
    
    return touch_indev;
}

/**
 * @brief Check if the touch controller is initialized
 * 
 * This function returns the current initialization state of the touch controller.
 * It can be used to verify that the touch system is ready before attempting to
 * create input devices or process touch events.
 * 
 * @return bool true if initialized, false otherwise
 */
bool esp32_s3_box3_touch_is_initialized(void) {
    return touch_initialized;
}

/**
 * @brief Deinitialize the GT911 touch controller
 * 
 * This function cleans up resources used by the touch controller, including deleting
 * the LVGL input device and deinitializing the GT911 driver. It should be called when
 * the touch functionality is no longer needed.
 */
void esp32_s3_box3_touch_deinit(void) {
    if (!touch_initialized) {
        return;
    }
    
    // Delete LVGL input device
    if (touch_indev != NULL) {
        lv_indev_delete(touch_indev);
        touch_indev = NULL;
    }
    
    // Deinitialize GT911 driver
    gt911_deinit();
    
    touch_initialized = false;
    mp_printf(&mp_plat_print, "[GT911] Touch controller deinitialized\n");
}