#include "ili9341_esp32.h"
#include "lv_conf.h"
#include "py/mphal.h"
#include "py/runtime.h"

// Global variables
static spi_device_handle_t spi_device = NULL;
static bool ili9341_initialized = false;

// Helper function to send command to ILI9341
static void ili9341_send_cmd(uint8_t cmd) {
    if (spi_device == NULL) return;
    
    gpio_set_level(ILI9341_PIN_NUM_DC, 0); // Command mode
    spi_transaction_t trans = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    esp_err_t ret = spi_device_transmit(spi_device, &trans);
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "ERROR: SPI command 0x%02X failed, ret=%d\n", cmd, ret);
    }
    // Remove delay to improve performance
}

// Helper function to send data to ILI9341
static void ili9341_send_data(uint8_t *data, size_t len) {
    if (spi_device == NULL) return;
    
    gpio_set_level(ILI9341_PIN_NUM_DC, 1); // Data mode
    spi_transaction_t trans = {
        .length = len * 8,
        .tx_buffer = data,
    };
    esp_err_t ret = spi_device_transmit(spi_device, &trans);
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "ERROR: SPI data send failed, len=%d, ret=%d\n", len, ret);
    }
    // Remove delay to improve performance
}

// Validate pin configuration
static esp_err_t validate_pin_config(void) {
    // Check if pins are within valid range for ESP32
    if (ILI9341_PIN_NUM_MOSI < 0 || ILI9341_PIN_NUM_MOSI > 48) {
        mp_printf(&mp_plat_print, "ERROR: Invalid MOSI pin %d\n", ILI9341_PIN_NUM_MOSI);
        return ESP_ERR_INVALID_ARG;
    }
    if (ILI9341_PIN_NUM_CLK < 0 || ILI9341_PIN_NUM_CLK > 48) {
        mp_printf(&mp_plat_print, "ERROR: Invalid CLK pin %d\n", ILI9341_PIN_NUM_CLK);
        return ESP_ERR_INVALID_ARG;
    }
    if (ILI9341_PIN_NUM_CS < 0 || ILI9341_PIN_NUM_CS > 48) {
        mp_printf(&mp_plat_print, "ERROR: Invalid CS pin %d\n", ILI9341_PIN_NUM_CS);
        return ESP_ERR_INVALID_ARG;
    }
    if (ILI9341_PIN_NUM_DC < 0 || ILI9341_PIN_NUM_DC > 48) {
        mp_printf(&mp_plat_print, "ERROR: Invalid DC pin %d\n", ILI9341_PIN_NUM_DC);
        return ESP_ERR_INVALID_ARG;
    }
    if (ILI9341_PIN_NUM_BCKL < 0 || ILI9341_PIN_NUM_BCKL > 48) {
        mp_printf(&mp_plat_print, "ERROR: Invalid BCKL pin %d\n", ILI9341_PIN_NUM_BCKL);
        return ESP_ERR_INVALID_ARG;
    }
    
    // mp_printf(&mp_plat_print, "Pin configuration validated: MOSI=%d, CLK=%d, CS=%d, DC=%d, BCKL=%d\n",
    //           ILI9341_PIN_NUM_MOSI, ILI9341_PIN_NUM_CLK, ILI9341_PIN_NUM_CS, 
    //           ILI9341_PIN_NUM_DC, ILI9341_PIN_NUM_BCKL);
    return ESP_OK;
}

// Initialize ESP-IDF SPI for ILI9341
esp_err_t ili9341_esp32_init(void) {
    if (ili9341_initialized) {
        return ESP_OK;
    }
    
    // Validate pin configuration first
    esp_err_t ret = validate_pin_config();
    if (ret != ESP_OK) {
        return ret;
    }
    
    mp_printf(&mp_plat_print, "Initializing SPI host %d\n", ILI9341_SPI_HOST);
    
    // Initialize SPI for ILI9341
    spi_bus_config_t buscfg = {
        .miso_io_num = ILI9341_PIN_NUM_MISO,
        .mosi_io_num = ILI9341_PIN_NUM_MOSI,
        .sclk_io_num = ILI9341_PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 320 * 240 * 2, // Max transfer size for full screen
    };
    
    // Initialize SPI bus with automatic DMA channel selection
    ret = spi_bus_initialize(ILI9341_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "ERROR: ILI9341 SPI bus initialization failed, ret=%d\n", ret);
        return ret;
    }
    mp_printf(&mp_plat_print, "SPI bus initialized successfully\n");
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000, // 1 MHz (reduced for stability)
        .mode = 0,
        .spics_io_num = ILI9341_PIN_NUM_CS,
        .queue_size = 7,
        .pre_cb = NULL,
        .post_cb = NULL,
        .flags = SPI_DEVICE_NO_DUMMY, // No dummy bits
    };
    
    // Add device to SPI bus
    ret = spi_bus_add_device(ILI9341_SPI_HOST, &devcfg, &spi_device);
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "ERROR: ILI9341 SPI device add failed, ret=%d\n", ret);
        // Try to free the SPI bus if device addition failed
        spi_bus_free(ILI9341_SPI_HOST);
        return ret;
    }
    mp_printf(&mp_plat_print, "SPI device added successfully\n");
    
    // Configure control pins
    mp_printf(&mp_plat_print, "Configuring GPIO pins: DC=%d, RST=%d, BCKL=%d\n", 
              ILI9341_PIN_NUM_DC, ILI9341_PIN_NUM_RST, ILI9341_PIN_NUM_BCKL);
    
    uint64_t pin_bit_mask = (1ULL << ILI9341_PIN_NUM_DC);
    
    // Only include RST pin if it's defined and valid
    #if ILI9341_PIN_NUM_RST >= 0
    pin_bit_mask |= (1ULL << ILI9341_PIN_NUM_RST);
    #endif
    
    // Only include BCKL pin if it's defined and valid  
    #if ILI9341_PIN_NUM_BCKL >= 0
    pin_bit_mask |= (1ULL << ILI9341_PIN_NUM_BCKL);
    #endif
    
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
        spi_bus_free(ILI9341_SPI_HOST);
        return ret;
    }
    mp_printf(&mp_plat_print, "GPIO pins configured successfully\n");
    
    // Reset ILI9341
    ili9341_esp32_reset();
    
    // Turn on backlight
    ili9341_esp32_backlight_on();
    
    mp_printf(&mp_plat_print, "ILI9341 ESP32 SPI initialized successfully\n");
    
    ili9341_initialized = true;
    return ESP_OK;
}

// Deinitialize ESP-IDF SPI
void ili9341_esp32_deinit(void) {
    if (spi_device != NULL) {
        spi_bus_remove_device(spi_device);
        spi_device = NULL;
    }
    ili9341_initialized = false;
}

// LVGL callback function to send commands to ILI9341
void ili9341_send_cmd_cb(lv_display_t * disp, const uint8_t * cmd, size_t cmd_size, 
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
void ili9341_send_color_cb(lv_display_t * disp, const uint8_t * cmd, size_t cmd_size,
                          uint8_t * param, size_t param_size) {
    if (spi_device == NULL) {
        mp_printf(&mp_plat_print, "ERROR: SPI device not initialized in color callback\n");
        lv_display_flush_ready(disp);
        return;
    }
    
    // Debug: Track screen creation
    static int flush_count = 0;
    flush_count++;
    if (flush_count <= 5) { // Only show first few flushes
        mp_printf(&mp_plat_print, "Screen flush #%d: cmd_size=%d, param_size=%d\n", 
                  flush_count, cmd_size, param_size);
    }
    
    // Send command if provided
    if (cmd && cmd_size > 0) {
        ili9341_send_cmd(cmd[0]);
    }
    
    // Send color data
    if (param && param_size > 0) {
        gpio_set_level(ILI9341_PIN_NUM_DC, 1); // Data mode
        
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
        
        if (flush_count <= 5) {
            mp_printf(&mp_plat_print, "Screen flush #%d completed successfully\n", flush_count);
        }
    }
    
    // Tell LVGL that the flush is complete
    lv_display_flush_ready(disp);
}

// Reset ILI9341 display
void ili9341_esp32_reset(void) {
    #if ILI9341_PIN_NUM_RST >= 0
    gpio_set_level(ILI9341_PIN_NUM_RST, 1); // High for reset
    mp_hal_delay_ms(10);
    gpio_set_level(ILI9341_PIN_NUM_RST, 0); // Low for normal operation
    mp_hal_delay_ms(10);
    #endif
}

// Turn on backlight
void ili9341_esp32_backlight_on(void) {
    #if ILI9341_PIN_NUM_BCKL >= 0
    gpio_set_level(ILI9341_PIN_NUM_BCKL, 1);
    #endif
}

// Turn off backlight
void ili9341_esp32_backlight_off(void) {
    #if ILI9341_PIN_NUM_BCKL >= 0
    gpio_set_level(ILI9341_PIN_NUM_BCKL, 0);
    #endif
}

// Set display rotation using MADCTL register
void ili9341_esp32_set_rotation(lv_display_t * disp, lv_display_rotation_t rotation) {
    if (spi_device == NULL) {
        mp_printf(&mp_plat_print, "ERROR: SPI device not initialized for rotation\n");
        return;
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
    ili9341_send_cmd_cb(disp, madctl_cmd, 1, &madctl_value, 1);
    
    mp_printf(&mp_plat_print, "ILI9341 rotation set to %d degrees (MADCTL=0x%02X)\n", 
              rotation * 90, madctl_value);
}

// Custom flush callback for ILI9341 display
static void ili9341_custom_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map) {
    // Calculate addresses like LVGL generic MIPI driver does
    int32_t x_start = area->x1;
    int32_t x_end = area->x2 + 1;
    int32_t y_start = area->y1;
    int32_t y_end = area->y2 + 1;
    
    // Set column address (0x2A) - use x_end - 1 like LVGL does
    uint8_t col_cmd[] = {0x2A};
    uint8_t col_data[] = {
        (x_start >> 8) & 0xFF, x_start & 0xFF,
        ((x_end - 1) >> 8) & 0xFF, (x_end - 1) & 0xFF
    };
    ili9341_send_cmd_cb(disp, col_cmd, 1, col_data, 4);
    
    // Set row address (0x2B) - use y_end - 1 like LVGL does
    uint8_t row_cmd[] = {0x2B};
    uint8_t row_data[] = {
        (y_start >> 8) & 0xFF, y_start & 0xFF,
        ((y_end - 1) >> 8) & 0xFF, (y_end - 1) & 0xFF
    };
    ili9341_send_cmd_cb(disp, row_cmd, 1, row_data, 4);
    
    // Memory write command (0x2C)
    uint8_t mem_write_cmd[] = {0x2C};
    ili9341_send_cmd_cb(disp, mem_write_cmd, 1, NULL, 0);
    
    // Send pixel data - calculate size like LVGL does
    size_t len = (x_end - x_start) * (y_end - y_start) * 2; // 2 bytes per pixel for RGB565
    ili9341_send_color_cb(disp, NULL, 0, px_map, len);
}

// Create ILI9341 display with proper initialization
lv_display_t * ili9341_esp32_create_display(uint32_t width, uint32_t height) {
    mp_printf(&mp_plat_print, "Creating ILI9341 display (%dx%d)...\n", width, height);
    
    // Create basic LVGL display
    lv_display_t *disp = lv_display_create(width, height);
    if (disp == NULL) {
        mp_printf(&mp_plat_print, "ERROR: Basic display creation failed!\n");
        return NULL;
    }
    
    mp_printf(&mp_plat_print, "Basic display created successfully!\n");
    
    // Manually initialize ILI9341 with minimal commands
    mp_printf(&mp_plat_print, "Initializing ILI9341 manually...\n");
    
    // Soft reset
    uint8_t soft_reset[] = {0x01};
    ili9341_send_cmd_cb(disp, soft_reset, 1, NULL, 0);
    mp_hal_delay_ms(120);
    
    // Sleep out
    uint8_t sleep_out[] = {0x11};
    ili9341_send_cmd_cb(disp, sleep_out, 1, NULL, 0);
    mp_hal_delay_ms(120);
    
    // Display on
    uint8_t display_on[] = {0x29};
    ili9341_send_cmd_cb(disp, display_on, 1, NULL, 0);
    mp_hal_delay_ms(20);
    
    mp_printf(&mp_plat_print, "ILI9341 manual initialization complete!\n");
    
    // Set up custom flush callback
    lv_display_set_flush_cb(disp, ili9341_custom_flush_cb);
    
    mp_printf(&mp_plat_print, "ILI9341 display created successfully!\n");
    
    return disp;
}