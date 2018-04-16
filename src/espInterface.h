#ifndef ESP_INTERFACE_H
#define ESP_INTERFACE_H

#ifdef ESP8266

#include <ESP8266WiFi.h>
extern "C" {
    #include "user_interface.h"
}

#define WIFI_PROTOCOL_11B      PHY_MODE_11B
#define WIFI_PROTOCOL_11G      PHY_MODE_11G
#define WIFI_PROTOCOL_11N      PHY_MODE_11N

#elif defined(ESP32)
#include <WiFi.h>
#else
#error Only ESP8266 or ESP32 platform is allowed
#endif // ESP8266

#endif // ESP_INTERFACE_H


