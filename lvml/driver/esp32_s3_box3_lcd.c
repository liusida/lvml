#include "esp32_s3_box3_lcd.h"
#include "lv_conf.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "src/drivers/display/ili9341/lv_ili9341.h"
#include "src/drivers/display/lcd/lv_lcd_generic_mipi.h"
#include "src/tick/lv_tick.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

// Global variables
static bool lcd_initialized = false;
static spi_device_handle_t spi_device = NULL;



// ESP32-S3-Box-3 pin configuration (matching lv_conf.h)
#define LCD_PIN_NUM_MOSI 6
#define LCD_PIN_NUM_CLK  7
#define LCD_PIN_NUM_CS   5
#define LCD_PIN_NUM_DC   4
#define LCD_PIN_NUM_RST  48
#define LCD_PIN_NUM_BCKL 47

// LCD configuration
#define LCD_H_RES 320
#define LCD_V_RES 240

// Helper function to send command to ILI9341
static void ili9341_send_cmd(uint8_t cmd) {
    if (spi_device == NULL) return;
    
    gpio_set_level(LCD_PIN_NUM_DC, 0); // Command mode
    spi_transaction_t trans = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    esp_err_t ret = spi_device_transmit(spi_device, &trans);
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "ERROR: SPI command 0x%02X failed, ret=%d\n", cmd, ret);
    }
}

// Helper function to send data to ILI9341
static void ili9341_send_data(uint8_t *data, size_t len) {
    if (spi_device == NULL) return;
    
    gpio_set_level(LCD_PIN_NUM_DC, 1); // Data mode
    spi_transaction_t trans = {
        .length = len * 8,
        .tx_buffer = data,
    };
    esp_err_t ret = spi_device_transmit(spi_device, &trans);
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "ERROR: SPI data send failed, len=%d, ret=%d\n", len, ret);
    }
}

// LVGL callback function to send commands to ILI9341
static void ili9341_send_cmd_cb(lv_display_t * disp, const uint8_t * cmd, size_t cmd_size, 
                               const uint8_t * param, size_t param_size) {
    if (spi_device == NULL) {
        mp_printf(&mp_plat_print, "ERROR: SPI device not initialized in command callback\n");
        return;
    }
    
    // Send command
    if (cmd_size > 0) {
        ili9341_send_cmd(cmd[0]);
    }
    
    // Send parameters if any
    if (param && param_size > 0) {
        ili9341_send_data((uint8_t*)param, param_size);
    }
}

// LVGL callback function to send color data to ILI9341
static void ili9341_send_color_cb(lv_display_t * disp, const uint8_t * cmd, size_t cmd_size,
                                 uint8_t * param, size_t param_size) {
    if (spi_device == NULL) {
        mp_printf(&mp_plat_print, "ERROR: SPI device not initialized in color callback\n");
        lv_display_flush_ready(disp);
        return;
    }
    
    // Send command if provided
    if (cmd && cmd_size > 0) {
        ili9341_send_cmd(cmd[0]);
    }
    
    // Send color data
    if (param && param_size > 0) {
        gpio_set_level(LCD_PIN_NUM_DC, 1); // Data mode
        
        // Split large transfers into smaller chunks to avoid SPI issues
        const int max_chunk_size = 4096; // 4KB chunks
        size_t remaining = param_size;
        uint8_t *data_ptr = param;
        
        while (remaining > 0) {
            size_t chunk_size = (remaining > max_chunk_size) ? max_chunk_size : remaining;
            
            spi_transaction_t trans = {
                .length = chunk_size * 8, // Convert bytes to bits
                .tx_buffer = data_ptr,
            };
            
            esp_err_t ret = spi_device_transmit(spi_device, &trans);
            if (ret != ESP_OK) {
                mp_printf(&mp_plat_print, "ERROR: SPI color transmit failed, chunk size=%d, remaining=%d, ret=%d\n", 
                          chunk_size, remaining, ret);
                break;
            }
            
            data_ptr += chunk_size;
            remaining -= chunk_size;
        }
    }
    
    // Tell LVGL that the flush is complete
    lv_display_flush_ready(disp);
}

// Initialize ESP-IDF SPI for ESP32-S3-Box-3 LCD
esp_err_t esp32_s3_box3_lcd_init(void) {
    if (lcd_initialized) {
        return ESP_OK;
    }
    
    mp_printf(&mp_plat_print, "Initializing ESP32-S3-Box-3 LCD driver...\n");
    
    // Initialize SPI for ILI9341
    spi_bus_config_t buscfg = {
        .miso_io_num = -1, // Not used for display
        .mosi_io_num = LCD_PIN_NUM_MOSI,
        .sclk_io_num = LCD_PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * 2, // Max transfer size for full screen
    };
    
    // Initialize SPI bus with automatic DMA channel selection
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "ERROR: SPI bus initialization failed, ret=%d\n", ret);
        return ret;
    }
    mp_printf(&mp_plat_print, "SPI bus initialized successfully\n");
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 27 * 1000 * 1000, // 27 MHz for ESP32-S3-Box-3
        .mode = 0,
        .spics_io_num = LCD_PIN_NUM_CS,
        .queue_size = 7,
        .pre_cb = NULL,
        .post_cb = NULL,
        .flags = SPI_DEVICE_NO_DUMMY, // No dummy bits
    };
    
    // Add device to SPI bus
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_device);
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "ERROR: SPI device add failed, ret=%d\n", ret);
        spi_bus_free(SPI2_HOST);
        return ret;
    }
    mp_printf(&mp_plat_print, "SPI device added successfully\n");
    
    // Configure control pins
    mp_printf(&mp_plat_print, "Configuring GPIO pins: DC=%d, RST=%d, BCKL=%d\n", 
              LCD_PIN_NUM_DC, LCD_PIN_NUM_RST, LCD_PIN_NUM_BCKL);
    
    uint64_t pin_bit_mask = (1ULL << LCD_PIN_NUM_DC) | (1ULL << LCD_PIN_NUM_RST) | (1ULL << LCD_PIN_NUM_BCKL);
    
    gpio_config_t ctrl_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = pin_bit_mask,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    
    ret = gpio_config(&ctrl_conf);
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "ERROR: GPIO configuration failed, ret=%d\n", ret);
        spi_bus_remove_device(spi_device);
        spi_bus_free(SPI2_HOST);
        return ret;
    }
    mp_printf(&mp_plat_print, "GPIO pins configured successfully\n");
    
    // Reset ILI9341
    gpio_set_level(LCD_PIN_NUM_RST, 1); // High for reset
    mp_hal_delay_ms(10);
    gpio_set_level(LCD_PIN_NUM_RST, 0); // Low for normal operation
    mp_hal_delay_ms(10);
    
    // Turn on backlight
    gpio_set_level(LCD_PIN_NUM_BCKL, 1);
    
    mp_printf(&mp_plat_print, "ESP32-S3-Box-3 LCD driver initialized successfully\n");
    
    lcd_initialized = true;
    return ESP_OK;
}

// Deinitialize LCD driver
void esp32_s3_box3_lcd_deinit(void) {
    if (spi_device != NULL) {
        spi_bus_remove_device(spi_device);
        spi_device = NULL;
    }
    spi_bus_free(SPI2_HOST);
    lcd_initialized = false;
    mp_printf(&mp_plat_print, "ESP32-S3-Box-3 LCD driver deinitialized\n");
}

// Set display rotation
esp_err_t esp32_s3_box3_lcd_set_rotation(lv_display_rotation_t rotation) {
    if (!lcd_initialized || spi_device == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t madctl_value = 0;
    
    // Set MADCTL register value based on rotation
    switch (rotation) {
        case LV_DISPLAY_ROTATION_0:
            madctl_value = 0x00; // Normal orientation
            break;
        case LV_DISPLAY_ROTATION_90:
            madctl_value = 0x60; // 90° clockwise (MY=1, MX=1, MV=1)
            break;
        case LV_DISPLAY_ROTATION_180:
            madctl_value = 0xC0; // 180° (MY=1, MX=1)
            break;
        case LV_DISPLAY_ROTATION_270:
            madctl_value = 0xA0; // 270° clockwise (MX=1, MV=1)
            break;
        default:
            madctl_value = 0x00;
            break;
    }
    
    // Send MADCTL command (0x36) with rotation value
    uint8_t madctl_cmd[] = {0x36};
    ili9341_send_cmd_cb(NULL, madctl_cmd, 1, &madctl_value, 1);
    
    mp_printf(&mp_plat_print, "Display rotation set to %d degrees (MADCTL=0x%02X)\n", 
              rotation * 90, madctl_value);
    
    return ESP_OK;
}

// Test function to verify LVGL memory allocation works
static lv_display_t * test_lvgl_memory_allocation(uint32_t width, uint32_t height) {
    mp_printf(&mp_plat_print, "Testing LVGL memory allocation...\n");
    
    // Test basic LVGL display creation first
    mp_printf(&mp_plat_print, "Creating basic LVGL display...\n");
    lv_display_t *disp = lv_display_create(width, height);
    if (disp == NULL) {
        mp_printf(&mp_plat_print, "ERROR: Basic LVGL display creation failed!\n");
        return NULL;
    }
    mp_printf(&mp_plat_print, "Basic LVGL display created successfully!\n");
    
    // Test LVGL memory allocation
    mp_printf(&mp_plat_print, "Testing lv_malloc...\n");
    void *test_mem = lv_malloc(1024);
    if (test_mem == NULL) {
        mp_printf(&mp_plat_print, "ERROR: lv_malloc failed!\n");
        lv_display_delete(disp);
        return NULL;
    }
    mp_printf(&mp_plat_print, "lv_malloc successful, freeing test memory...\n");
    lv_free(test_mem);
    
    // Clean up test display
    lv_display_delete(disp);
    mp_printf(&mp_plat_print, "LVGL memory allocation test passed!\n");
    
    return NULL; // Return NULL since we cleaned up the test display
}

// Test function to verify lv_lcd_generic_mipi_create works
static lv_display_t * test_generic_mipi_create(uint32_t width, uint32_t height) {
    mp_printf(&mp_plat_print, "Testing lv_lcd_generic_mipi_create (%dx%d)...\n", width, height);
    
    // Test the generic MIPI create function directly
    mp_printf(&mp_plat_print, "[DEBUG] About to call lv_lcd_generic_mipi_create\n");
    lv_display_t *disp = lv_lcd_generic_mipi_create(width, height, LV_LCD_FLAG_NONE, 
                                                   ili9341_send_cmd_cb, ili9341_send_color_cb);
    mp_printf(&mp_plat_print, "[DEBUG] lv_lcd_generic_mipi_create returned\n");
    
    if (disp == NULL) {
        mp_printf(&mp_plat_print, "ERROR: Generic MIPI display creation failed!\n");
        return NULL;
    }
    
    mp_printf(&mp_plat_print, "Generic MIPI display created successfully!\n");
    return disp;
}

// Create LVGL display with ESP32-S3-Box-3 LCD driver
lv_display_t * esp32_s3_box3_lcd_create_display(uint32_t width, uint32_t height) {
    mp_printf(&mp_plat_print, "Creating ESP32-S3-Box-3 LCD display (%dx%d)...\n", width, height);
    
    // First test LVGL memory allocation
    test_lvgl_memory_allocation(width, height);
    
    // Then test the generic MIPI driver
    lv_display_t *disp = test_generic_mipi_create(width, height);
    if (disp == NULL) {
        mp_printf(&mp_plat_print, "ERROR: Generic MIPI test failed!\n");
        return NULL;
    }
    
    // If generic MIPI works, try the full ILI9341 driver
    lv_display_delete(disp); // Clean up test display
    
    mp_printf(&mp_plat_print, "Now trying full ILI9341 driver...\n");
    disp = lv_ili9341_create(width, height, LV_LCD_FLAG_NONE, 
                            ili9341_send_cmd_cb, ili9341_send_color_cb);
    if (disp == NULL) {
        mp_printf(&mp_plat_print, "ERROR: ILI9341 display creation failed!\n");
        return NULL;
    }
    
    mp_printf(&mp_plat_print, "ESP32-S3-Box-3 LCD display created successfully!\n");
    
    return disp;
}
