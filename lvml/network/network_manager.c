/**
 * @file network_manager.c
 * @brief Network and WiFi management implementation
 */

#include "network_manager.h"
#include "micropython/py/mphal.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/**********************
 *  STATIC VARIABLES
 **********************/

static bool network_manager_initialized = false;
static wifi_status_t current_wifi_status = WIFI_STATUS_DISCONNECTED;
static esp_netif_t *sta_netif = NULL;
static EventGroupHandle_t s_wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;
static const int WIFI_FAIL_BIT = BIT1;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        mp_printf(&mp_plat_print, "[NETWORK] WiFi station started\n");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        mp_printf(&mp_plat_print, "[NETWORK] WiFi connected to AP\n");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        mp_printf(&mp_plat_print, "[NETWORK] WiFi disconnected from AP\n");
        current_wifi_status = WIFI_STATUS_DISCONNECTED;
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        mp_printf(&mp_plat_print, "[NETWORK] Got IP address: " IPSTR "\n", IP2STR(&event->ip_info.ip));
        current_wifi_status = WIFI_STATUS_CONNECTED;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

lvml_error_t network_manager_init(void) {
    if (network_manager_initialized) {
        return LVML_OK;
    }
    
    mp_printf(&mp_plat_print, "[NETWORK] Initializing network manager\n");
    
    // Initialize TCP/IP adapter
    ESP_ERROR_CHECK(esp_netif_init());
    
    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Create WiFi station interface
    sta_netif = esp_netif_create_default_wifi_sta();
    if (sta_netif == NULL) {
        mp_printf(&mp_plat_print, "[NETWORK] Failed to create WiFi station interface\n");
        return LVML_ERROR_INIT;
    }
    
    // Initialize WiFi with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    
    // Create event group
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        mp_printf(&mp_plat_print, "[NETWORK] Failed to create event group\n");
        return LVML_ERROR_MEMORY;
    }
    
    // Set WiFi mode to station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());
    
    current_wifi_status = WIFI_STATUS_DISCONNECTED;
    network_manager_initialized = true;
    
    mp_printf(&mp_plat_print, "[NETWORK] Network manager initialized successfully\n");
    
    return LVML_OK;
}

void network_manager_deinit(void) {
    if (!network_manager_initialized) {
        return;
    }
    
    mp_printf(&mp_plat_print, "[NETWORK] Deinitializing network manager\n");
    
    // Disconnect WiFi if connected
    if (current_wifi_status == WIFI_STATUS_CONNECTED) {
        network_manager_disconnect_wifi();
    }
    
    // Stop WiFi
    esp_wifi_stop();
    esp_wifi_deinit();
    
    // Clean up event group
    if (s_wifi_event_group) {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
    }
    
    // Clean up network interface
    if (sta_netif) {
        esp_netif_destroy(sta_netif);
        sta_netif = NULL;
    }
    
    network_manager_initialized = false;
    mp_printf(&mp_plat_print, "[NETWORK] Network manager deinitialized\n");
}

lvml_error_t network_manager_connect_wifi(const char* ssid, const char* password) {
    if (!network_manager_initialized) {
        mp_printf(&mp_plat_print, "[NETWORK] Network manager not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    if (ssid == NULL) {
        mp_printf(&mp_plat_print, "[NETWORK] Invalid SSID\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    
    mp_printf(&mp_plat_print, "[NETWORK] Connecting to WiFi: %s\n", ssid);
    
    // Configure WiFi station
    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    if (password != NULL) {
        strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    }
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;
    
    // Set WiFi configuration
    esp_err_t ret = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "[NETWORK] Failed to set WiFi config: %s\n", esp_err_to_name(ret));
        return LVML_ERROR_NETWORK;
    }
    
    // Clear event bits
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
    
    // Set status to connecting
    current_wifi_status = WIFI_STATUS_CONNECTING;
    
    // Start connection
    ret = esp_wifi_connect();
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "[NETWORK] Failed to start WiFi connection: %s\n", esp_err_to_name(ret));
        current_wifi_status = WIFI_STATUS_ERROR;
        return LVML_ERROR_NETWORK;
    }
    
    // Wait for connection result (with timeout)
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                          WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                          pdFALSE,
                                          pdFALSE,
                                          10000 / portTICK_PERIOD_MS); // 10 second timeout
    
    if (bits & WIFI_CONNECTED_BIT) {
        mp_printf(&mp_plat_print, "[NETWORK] WiFi connected successfully\n");
        current_wifi_status = WIFI_STATUS_CONNECTED;
        return LVML_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        mp_printf(&mp_plat_print, "[NETWORK] WiFi connection failed\n");
        current_wifi_status = WIFI_STATUS_ERROR;
        return LVML_ERROR_NETWORK;
    } else {
        mp_printf(&mp_plat_print, "[NETWORK] WiFi connection timeout\n");
        current_wifi_status = WIFI_STATUS_ERROR;
        return LVML_ERROR_NETWORK;
    }
}

lvml_error_t network_manager_disconnect_wifi(void) {
    if (!network_manager_initialized) {
        mp_printf(&mp_plat_print, "[NETWORK] Network manager not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    mp_printf(&mp_plat_print, "[NETWORK] Disconnecting from WiFi\n");
    
    // Disconnect from WiFi
    esp_err_t ret = esp_wifi_disconnect();
    if (ret != ESP_OK) {
        mp_printf(&mp_plat_print, "[NETWORK] Failed to disconnect WiFi: %s\n", esp_err_to_name(ret));
        return LVML_ERROR_NETWORK;
    }
    
    current_wifi_status = WIFI_STATUS_DISCONNECTED;
    
    mp_printf(&mp_plat_print, "[NETWORK] WiFi disconnected\n");
    
    return LVML_OK;
}

wifi_status_t network_manager_get_wifi_status(void) {
    return current_wifi_status;
}

lvml_error_t network_manager_http_get(const char* url, http_response_t* response) {
    if (!network_manager_initialized) {
        mp_printf(&mp_plat_print, "[NETWORK] Network manager not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    if (url == NULL || response == NULL) {
        mp_printf(&mp_plat_print, "[NETWORK] Invalid parameters\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    
    if (current_wifi_status != WIFI_STATUS_CONNECTED) {
        mp_printf(&mp_plat_print, "[NETWORK] WiFi not connected\n");
        return LVML_ERROR_NETWORK;
    }
    
    mp_printf(&mp_plat_print, "[NETWORK] Fetching URL: %s\n", url);
    
    // TODO: Implement actual HTTP GET
    // For now, return empty response
    response->status_code = 200;
    response->content = NULL;
    response->content_length = 0;
    response->content_type = NULL;
    
    mp_printf(&mp_plat_print, "[NETWORK] HTTP GET completed (minimal implementation)\n");
    
    return LVML_OK;
}

// Helper function to parse URL
static int parse_url(const char* url, char* host, int* port, char* path) {
    if (strncmp(url, "https://", 8) == 0) {
        *port = 443;
        url += 8;
    } else if (strncmp(url, "http://", 7) == 0) {
        *port = 80;
        url += 7;
    } else {
        return -1;
    }
    
    const char* slash = strchr(url, '/');
    if (slash == NULL) {
        strcpy(host, url);
        strcpy(path, "/");
    } else {
        int host_len = slash - url;
        strncpy(host, url, host_len);
        host[host_len] = '\0';
        strcpy(path, slash);
    }
    
    return 0;
}

lvml_error_t network_manager_http_post(const char* url, const char* json_data, http_response_t* response) {
    if (!network_manager_initialized) {
        mp_printf(&mp_plat_print, "[NETWORK] Network manager not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    if (url == NULL || response == NULL) {
        mp_printf(&mp_plat_print, "[NETWORK] Invalid parameters\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    
    if (current_wifi_status != WIFI_STATUS_CONNECTED) {
        mp_printf(&mp_plat_print, "[NETWORK] WiFi not connected\n");
        return LVML_ERROR_NETWORK;
    }
    
    mp_printf(&mp_plat_print, "[NETWORK] POST to URL: %s\n", url);
    
    // Initialize response structure
    response->status_code = 0;
    response->content = NULL;
    response->content_length = 0;
    response->content_type = NULL;
    
    // Parse URL
    char host[256];
    char path[512];
    int port;
    
    if (parse_url(url, host, &port, path) != 0) {
        mp_printf(&mp_plat_print, "[NETWORK] Invalid URL format\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    
    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        mp_printf(&mp_plat_print, "[NETWORK] Failed to create socket\n");
        return LVML_ERROR_NETWORK;
    }
    
    // Resolve hostname
    struct hostent *he = gethostbyname(host);
    if (he == NULL) {
        mp_printf(&mp_plat_print, "[NETWORK] Failed to resolve hostname: %s\n", host);
        close(sock);
        return LVML_ERROR_NETWORK;
    }
    
    // Set up server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
    
    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        mp_printf(&mp_plat_print, "[NETWORK] Failed to connect to server\n");
        close(sock);
        return LVML_ERROR_NETWORK;
    }
    
    // Prepare HTTP request
    const char* data = json_data ? json_data : "";
    size_t data_len = json_data ? strlen(json_data) : 0;
    
    char request[2048];
    int request_len = snprintf(request, sizeof(request),
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "User-Agent: LVML/1.0\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        path, host, data_len, data);
    
    // Send request
    if (send(sock, request, request_len, 0) < 0) {
        mp_printf(&mp_plat_print, "[NETWORK] Failed to send request\n");
        close(sock);
        return LVML_ERROR_NETWORK;
    }
    
    // Receive response
    char response_buffer[4096];
    int total_received = 0;
    int bytes_received;
    
    while ((bytes_received = recv(sock, response_buffer + total_received, 
                                 sizeof(response_buffer) - total_received - 1, 0)) > 0) {
        total_received += bytes_received;
        if (total_received >= sizeof(response_buffer) - 1) {
            break; // Buffer full
        }
    }
    
    response_buffer[total_received] = '\0';
    
    // Parse HTTP response
    char* header_end = strstr(response_buffer, "\r\n\r\n");
    if (header_end != NULL) {
        *header_end = '\0';
        char* body = header_end + 4;
        
        // Parse status code
        if (sscanf(response_buffer, "HTTP/1.1 %d", &response->status_code) != 1) {
            if (sscanf(response_buffer, "HTTP/1.0 %d", &response->status_code) != 1) {
                response->status_code = 200; // Default
            }
        }
        
        // Find content type
        char* content_type_header = strstr(response_buffer, "Content-Type:");
        if (content_type_header != NULL) {
            char* content_type_start = content_type_header + 13;
            while (*content_type_start == ' ') content_type_start++;
            char* content_type_end = strstr(content_type_start, "\r\n");
            if (content_type_end != NULL) {
                int content_type_len = content_type_end - content_type_start;
                response->content_type = malloc(content_type_len + 1);
                if (response->content_type != NULL) {
                    strncpy(response->content_type, content_type_start, content_type_len);
                    response->content_type[content_type_len] = '\0';
                }
            }
        }
        
        // Copy response body
        int body_len = total_received - (body - response_buffer);
        if (body_len > 0) {
            response->content = malloc(body_len + 1);
            if (response->content != NULL) {
                memcpy(response->content, body, body_len);
                response->content[body_len] = '\0';
                response->content_length = body_len;
            }
        }
    }
    
    close(sock);
    
    mp_printf(&mp_plat_print, "[NETWORK] HTTP POST completed, status: %d\n", response->status_code);
    
    return LVML_OK;
}

void network_manager_free_response(http_response_t* response) {
    if (response == NULL) {
        return;
    }
    
    mp_printf(&mp_plat_print, "[NETWORK] Freeing HTTP response\n");
    
    if (response->content != NULL) {
        free(response->content);
        response->content = NULL;
    }
    
    if (response->content_type != NULL) {
        free(response->content_type);
        response->content_type = NULL;
    }
    
    response->status_code = 0;
    response->content_length = 0;
}

bool network_manager_is_valid_url(const char* url) {
    if (url == NULL) {
        return false;
    }
    
    // Basic URL validation - check for http:// or https://
    return (strncmp(url, "http://", 7) == 0) || (strncmp(url, "https://", 8) == 0);
}
