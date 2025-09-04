#include "esp32_s3_box3_lcd.h"
#include "lv_conf.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "src/drivers/display/ili9341/lv_ili9341.h"
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
    spi_device_transmit(spi_device, &trans);
}

// Helper function to send data to ILI9341
static void ili9341_send_data(uint8_t *data, size_t len) {
    if (spi_device == NULL) return;
    
    gpio_set_level(LCD_PIN_NUM_DC, 1); // Data mode
    spi_transaction_t trans = {
        .length = len * 8,
        .tx_buffer = data,
    };
    spi_device_transmit(spi_device, &trans);
}

// LVGL callback function to send commands to ILI9341
static void ili9341_send_cmd_cb(lv_display_t * disp, const uint8_t * cmd, size_t cmd_size, 
                               const uint8_t * param, size_t param_size) {
    if (spi_device == NULL) return;
    
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
            
            spi_device_transmit(spi_device, &trans);
            
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
        return ret;
    }
    
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
        spi_bus_free(SPI2_HOST);
        return ret;
    }
    
    // Configure control pins
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
        spi_bus_remove_device(spi_device);
        spi_bus_free(SPI2_HOST);
        return ret;
    }
    
    // Reset ILI9341
    gpio_set_level(LCD_PIN_NUM_RST, 1); // High for reset
    mp_hal_delay_ms(10);
    gpio_set_level(LCD_PIN_NUM_RST, 0); // Low for normal operation
    mp_hal_delay_ms(10);
    
    // Turn on backlight
    gpio_set_level(LCD_PIN_NUM_BCKL, 1);
    
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
}

// Set display rotation
esp_err_t esp32_s3_box3_lcd_set_rotation(lv_display_rotation_t rotation) {
    if (!lcd_initialized || spi_device == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t madctl_value = 0;
    
    // With LV_COLOR_16_SWAP 1 + BGR order:
    // Set MADCTL register value based on rotation using TFT_eSPI values
    // MADCTL bit definitions (from TFT_eSPI ILI9341_Defines.h):
    // MY (bit 7): Row Address Order (0x80)
    // MX (bit 6): Column Address Order (0x40)
    // MV (bit 5): Row/Column Exchange (0x20)
    // ML (bit 4): Vertical Refresh Order (0x10)
    // BGR (bit 3): Color Order (0x08 = BGR, 0x00 = RGB)
    // MH (bit 2): Horizontal Refresh Order (0x04)
    switch (rotation) {
        case LV_DISPLAY_ROTATION_0:
            madctl_value = 0x80 | 0x20 | 0x08; // MY=1, MV=1, BGR order (TFT_eSPI case 0 M5STACK with BGR)
            break;
        case LV_DISPLAY_ROTATION_90:
            madctl_value = 0x08; // BGR order (TFT_eSPI case 1 M5STACK with BGR)
            break;
        case LV_DISPLAY_ROTATION_180:
            madctl_value = 0x20 | 0x40 | 0x08; // MV=1, MX=1, BGR order (TFT_eSPI case 2 M5STACK with BGR)
            break;
        case LV_DISPLAY_ROTATION_270:
            madctl_value = 0x40 | 0x80 | 0x08; // MX=1, MY=1, BGR order (TFT_eSPI case 3 M5STACK with BGR)
            break;
        default:
            madctl_value = 0x80 | 0x20 | 0x08; // Default to 0° rotation
            break;
    }
    
    // Send MADCTL command (0x36) with rotation value
    ili9341_send_cmd(0x36);  // MADCTL command
    ili9341_send_data(&madctl_value, 1);  // Send the rotation value
    
    return ESP_OK;
}




// Create LVGL display with ESP32-S3-Box-3 LCD driver
lv_display_t * esp32_s3_box3_lcd_create_display(uint32_t width, uint32_t height) {
    // Create ILI9341 display
    lv_display_t *disp = lv_ili9341_create(width, height, LV_LCD_FLAG_NONE, 
                            ili9341_send_cmd_cb, ili9341_send_color_cb);
    if (disp == NULL) {
        return NULL;
    }
    
    // Set default rotation
    esp32_s3_box3_lcd_set_rotation(LV_DISPLAY_ROTATION_270);
    
    return disp;
}
