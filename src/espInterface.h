#ifndef ESP_INTERFACE_H
#define ESP_INTERFACE_H

#ifdef ESP8266

#include <ESP8266WiFi.h>
extern "C" {
    #include "user_interface.h"
}

#define WIFI_AUTH_OPEN         AUTH_OPEN
#define WIFI_AUTH_WEP          AUTH_WEP
#define WIFI_AUTH_WPA_PSK      AUTH_WPA_PSK
#define WIFI_AUTH_WPA2_PSK     AUTH_WPA2_PSK
#define WIFI_AUTH_WPA_WPA2_PSK AUTH_WPA_WPA2_PSK
#define WIFI_AUTH_MAX          AUTH_MAX

#define WIFI_PROTOCOL_11B      PHY_MODE_11B
#define WIFI_PROTOCOL_11G      PHY_MODE_11G
#define WIFI_PROTOCOL_11N      PHY_MODE_11N

#define ESP_OK          0
#define ESP_FAIL        -1

typedef AUTH_MODE wifi_auth_mode_t;
typedef int32_t esp_err_t;
typedef struct bss_info wifi_ap_record_t;

typedef ip_info tcpip_adapter_ip_info_t;

typedef enum {
    WIFI_STORAGE_FLASH,  /**< all configuration will store in both memory and flash */
    WIFI_STORAGE_RAM    /**< all configuration will only store in the memory */
} wifi_storage_t;

typedef enum {
    ESP_IF_WIFI_STA = STATION_IF,
    ESP_IF_WIFI_AP  = SOFTAP_IF
} wifi_interface_t;

typedef softap_config  wifi_ap_config_t;
typedef station_config wifi_sta_config_t;

typedef union {
    wifi_ap_config_t  ap;   /**< configuration of AP  */
    wifi_sta_config_t sta;  /**< configuration of STA */
} wifi_config_t;

typedef struct scan_config wifi_scan_config_t;

#elif defined(ESP32)
#include <WiFi.h>
#else
#error Only ESP8266 or ESP32 platform is allowed
#endif // ESP8266

#endif // ESP_INTERFACE_H


