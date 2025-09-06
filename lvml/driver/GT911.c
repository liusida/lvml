/**
 * @file GT911.c
 * @brief GT911 Touch Controller Driver for ESP-IDF
 * 
 * Translated from https://github.com/alex-code/GT911 Arduino library.
 *
 * This file provides a complete ESP-IDF implementation of the GT911 touch controller
 * driver, translated from the Arduino GT911 library. It includes all the core
 * functionality for initializing, reading touch data, and managing the GT911 controller.
 */

#include "GT911.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "string.h"

static const char *TAG = "GT911";

// Static variables for interrupt handling
static volatile bool gt911_irq = false;
static gpio_num_t int_pin = GPIO_NUM_NC;
static gpio_num_t rst_pin = GPIO_NUM_NC;
static uint8_t i2c_addr = GT911_I2C_ADDR_BA;
static i2c_port_t i2c_port = I2C_NUM_0;

// GT911 instance data
static bool config_loaded = false;
static GTConfig gt_config;
static GTInfo gt_info;
static GTPoint gt_points[GT911_MAX_CONTACTS];
static GT911_Rotate_t rotation = GT911_ROTATE_0;

// Forward declarations
static void gt911_reset(void);
static esp_err_t gt911_i2c_start(uint16_t reg);
static esp_err_t gt911_write(uint16_t reg, uint8_t data);
static uint8_t gt911_read(uint16_t reg);
static esp_err_t gt911_write_bytes(uint16_t reg, uint8_t *data, uint16_t size);
static esp_err_t gt911_read_bytes(uint16_t reg, uint8_t *data, uint16_t size);
static uint8_t gt911_calc_checksum(uint8_t *buf, uint8_t len);
static uint8_t gt911_read_checksum(void);
static int8_t gt911_read_touches(void);
static bool gt911_read_touch_points(void);

/**
 * @brief IRQ handler for GT911 interrupt pin
 * 
 * This function is called when the GT911 interrupt pin triggers.
 * It sets the global interrupt flag that can be checked by the main code.
 */
static void IRAM_ATTR gt911_irq_handler(void *arg) {
    gt911_irq = true;
}

/**
 * @brief Reset the GT911 touch controller
 * 
 * Performs the GT911-specific reset sequence with precise timing requirements.
 * This function configures the interrupt and reset pins, performs the reset
 * sequence, and sets up the controller for normal operation.
 */
static void gt911_reset(void) {
    ESP_LOGD(TAG, "Performing GT911 reset sequence");
    
    // Configure interrupt pin as output
    gpio_config_t int_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << int_pin),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&int_conf);
    
    // Configure reset pin as output
    gpio_config_t rst_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << rst_pin),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&rst_conf);
    
    // Reset sequence
    gpio_set_level(int_pin, 0);
    gpio_set_level(rst_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(11));
    
    // Set interrupt pin based on I2C address
    gpio_set_level(int_pin, (i2c_addr == GT911_I2C_ADDR_28) ? 1 : 0);
    vTaskDelay(pdMS_TO_TICKS(1)); // 110us delay
    
    // Release reset pin (set as input to let it float high)
    rst_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&rst_conf);
    
    vTaskDelay(pdMS_TO_TICKS(6));
    gpio_set_level(int_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(51));
}

/**
 * @brief Start I2C transmission to a specific register
 * 
 * @param reg Register address to write to
 * @return esp_err_t ESP_OK on success, error code on failure
 */
static esp_err_t gt911_i2c_start(uint16_t reg) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg >> 8, true);
    i2c_master_write_byte(cmd, reg & 0xFF, true);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief Write a single byte to a GT911 register
 * 
 * @param reg Register address
 * @param data Data to write
 * @return esp_err_t ESP_OK on success, error code on failure
 */
static esp_err_t gt911_write(uint16_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg >> 8, true);
    i2c_master_write_byte(cmd, reg & 0xFF, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief Read a single byte from a GT911 register
 * 
 * @param reg Register address
 * @return uint8_t Read value, 0 on error
 */
static uint8_t gt911_read(uint16_t reg) {
    // Start transmission to set register
    esp_err_t ret = gt911_i2c_start(reg);
    if (ret != ESP_OK) {
        return 0;
    }
    
    // Read from register
    uint8_t data = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    return (ret == ESP_OK) ? data : 0;
}

/**
 * @brief Write multiple bytes to GT911 registers
 * 
 * @param reg Starting register address
 * @param data Data buffer to write
 * @param size Number of bytes to write
 * @return esp_err_t ESP_OK on success, error code on failure
 */
static esp_err_t gt911_write_bytes(uint16_t reg, uint8_t *data, uint16_t size) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg >> 8, true);
    i2c_master_write_byte(cmd, reg & 0xFF, true);
    
    for (uint16_t i = 0; i < size; i++) {
        i2c_master_write_byte(cmd, data[i], true);
    }
    
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief Read multiple bytes from GT911 registers
 * 
 * @param reg Starting register address
 * @param data Data buffer to read into
 * @param size Number of bytes to read
 * @return esp_err_t ESP_OK on success, error code on failure
 */
static esp_err_t gt911_read_bytes(uint16_t reg, uint8_t *data, uint16_t size) {
    // Start transmission to set register
    esp_err_t ret = gt911_i2c_start(reg);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Read data
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

/**
 * @brief Calculate checksum for GT911 configuration
 * 
 * @param buf Buffer to calculate checksum for
 * @param len Length of buffer
 * @return uint8_t Calculated checksum
 */
static uint8_t gt911_calc_checksum(uint8_t *buf, uint8_t len) {
    uint8_t ccsum = 0;
    for (uint8_t i = 0; i < len; i++) {
        ccsum += buf[i];
    }
    return (~ccsum) + 1;
}

/**
 * @brief Read checksum from GT911
 * 
 * @return uint8_t Checksum value
 */
static uint8_t gt911_read_checksum(void) {
    return gt911_read(GT911_REG_CHECKSUM);
}

/**
 * @brief Read touch count from GT911 status register
 * 
 * This function polls the GT911 status register to check for valid touch data.
 * It waits for the touch flag to be set and validates the touch count.
 * 
 * @return int8_t Number of touches (0-5), -1 on error
 */
static int8_t gt911_read_touches(void) {
    uint32_t timeout = xTaskGetTickCount() + pdMS_TO_TICKS(20);
    
    do {
        uint8_t flag = gt911_read(GT911_REG_COORD_ADDR);
        if ((flag & 0x80) && ((flag & 0x0F) < GT911_MAX_CONTACTS)) {
            gt911_write(GT911_REG_COORD_ADDR, 0); // Clear status register
            return flag & 0x0F;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    } while (xTaskGetTickCount() < timeout);
    
    return 0;
}

/**
 * @brief Read touch point data from GT911
 * 
 * Reads the actual touch coordinate data from the GT911 controller.
 * Applies rotation if configured.
 * 
 * @return bool true on success, false on failure
 */
static bool gt911_read_touch_points(void) {
    bool result = (gt911_read_bytes(GT911_REG_COORD_ADDR + 1, (uint8_t*)gt_points, 
                                   sizeof(GTPoint) * GT911_MAX_CONTACTS) == ESP_OK);
    
    if (result && rotation != GT911_ROTATE_0) {
        for (uint8_t i = 0; i < GT911_MAX_CONTACTS; i++) {
            if (rotation == GT911_ROTATE_180) {
                gt_points[i].x = gt_info.xResolution - gt_points[i].x;
                gt_points[i].y = gt_info.yResolution - gt_points[i].y;
            }
        }
    }
    
    return result;
}

// Public API functions

/**
 * @brief Initialize the GT911 touch controller
 * 
 * @param int_pin_num Interrupt pin number
 * @param rst_pin_num Reset pin number  
 * @param addr I2C address (GT911_I2C_ADDR_28 or GT911_I2C_ADDR_BA)
 * @param i2c_port_num I2C port number
 * @param sda_pin SDA pin number
 * @param scl_pin SCL pin number
 * @param clk_freq I2C clock frequency
 * @return bool true on success, false on failure
 */
bool gt911_begin(gpio_num_t int_pin_num, gpio_num_t rst_pin_num, uint8_t addr, 
                 i2c_port_t i2c_port_num, gpio_num_t sda_pin, gpio_num_t scl_pin, 
                 uint32_t clk_freq) {
    int_pin = int_pin_num;
    rst_pin = rst_pin_num;
    i2c_addr = addr;
    i2c_port = i2c_port_num;
    
    ESP_LOGI(TAG, "Initializing GT911 on I2C port %d, address 0x%02X", i2c_port, i2c_addr);
    
    // Configure I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = clk_freq,
    };
    
    esp_err_t ret = i2c_param_config(i2c_port, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C param config failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    ret = i2c_driver_install(i2c_port, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    // Reset if reset pin is valid
    if (rst_pin != GPIO_NUM_NC) {
        vTaskDelay(pdMS_TO_TICKS(300));
        gt911_reset();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    // Configure interrupt pin if valid
    if (int_pin != GPIO_NUM_NC) {
        gpio_config_t int_conf = {
            .intr_type = GPIO_INTR_NEGEDGE,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = (1ULL << int_pin),
            .pull_down_en = 0,
            .pull_up_en = 1,
        };
        gpio_config(&int_conf);
        gpio_install_isr_service(0);
        gpio_isr_handler_add(int_pin, gt911_irq_handler, NULL);
    }
    
    // Test I2C communication
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C communication test failed: %s", esp_err_to_name(ret));
        i2c_driver_delete(i2c_port);
        return false;
    }
    
    // Read device info
    gt911_read_info();
    
    ESP_LOGI(TAG, "GT911 initialized successfully");
    return true;
}

/**
 * @brief Read product ID from GT911
 * 
 * @param buf Buffer to store product ID (minimum 4 bytes)
 * @param len Length of buffer
 * @return bool true on success, false on failure
 */
bool gt911_product_id(uint8_t *buf, uint8_t len) {
    if (len < 4) {
        return false;
    }
    
    memset(buf, 0, 4);
    return (gt911_read_bytes(GT911_REG_ID, buf, 4) == ESP_OK);
}

/**
 * @brief Read configuration from GT911
 * 
 * @return GTConfig* Pointer to configuration structure, NULL on error
 */
GTConfig* gt911_read_config(void) {
    esp_err_t ret = gt911_read_bytes(GT911_REG_CFG, (uint8_t*)&gt_config, sizeof(gt_config));
    if (ret != ESP_OK) {
        return NULL;
    }
    
    if (gt911_read_checksum() == gt911_calc_checksum((uint8_t*)&gt_config, sizeof(gt_config))) {
        config_loaded = true;
        return &gt_config;
    }
    
    return NULL;
}

/**
 * @brief Write configuration to GT911
 * 
 * @return bool true if configuration was written, false if no changes needed
 */
bool gt911_write_config(void) {
    uint8_t checksum = gt911_calc_checksum((uint8_t*)&gt_config, sizeof(gt_config));
    if (config_loaded && gt911_read_checksum() != checksum) {
        gt911_write_bytes(GT911_REG_CFG, (uint8_t*)&gt_config, sizeof(gt_config));
        
        uint8_t buf[2] = { checksum, 1 };
        gt911_write_bytes(GT911_REG_CHECKSUM, buf, sizeof(buf));
        return true;
    }
    return false;
}

/**
 * @brief Read device information from GT911
 * 
 * @return GTInfo* Pointer to info structure
 */
GTInfo* gt911_read_info(void) {
    gt911_read_bytes(GT911_REG_DATA, (uint8_t*)&gt_info, sizeof(gt_info));
    return &gt_info;
}

/**
 * @brief Check for touch events
 * 
 * @param mode Touch detection mode (interrupt or polling)
 * @return uint8_t Number of touches detected
 */
uint8_t gt911_touched(uint8_t mode) {
    bool irq = false;
    
    if (mode == GT911_MODE_INTERRUPT) {
        portENTER_CRITICAL(&gt911_irq);
        irq = gt911_irq;
        gt911_irq = false;
        portEXIT_CRITICAL(&gt911_irq);
    } else if (mode == GT911_MODE_POLLING) {
        irq = true;
    }
    
    uint8_t contacts = 0;
    if (irq) {
        contacts = gt911_read_touches();
        
        if (contacts > 0) {
            gt911_read_touch_points();
        }
    }
    
    return contacts;
}

/**
 * @brief Get a specific touch point
 * 
 * @param num Touch point index (0-4)
 * @return GTPoint Touch point data
 */
GTPoint gt911_get_point(uint8_t num) {
    if (num >= GT911_MAX_CONTACTS) {
        GTPoint empty = {0};
        return empty;
    }
    return gt_points[num];
}

/**
 * @brief Get all touch points
 * 
 * @return GTPoint* Pointer to touch points array
 */
GTPoint* gt911_get_points(void) {
    return gt_points;
}

/**
 * @brief Set display rotation
 * 
 * @param rot Rotation setting
 */
void gt911_set_rotation(GT911_Rotate_t rot) {
    rotation = rot;
}

/**
 * @brief Deinitialize GT911 driver
 * 
 * Cleans up resources and removes interrupt handlers.
 */
void gt911_deinit(void) {
    if (int_pin != GPIO_NUM_NC) {
        gpio_isr_handler_remove(int_pin);
    }
    i2c_driver_delete(i2c_port);
    ESP_LOGI(TAG, "GT911 deinitialized");
}
