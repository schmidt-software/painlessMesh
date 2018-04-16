#ifndef ESP_INTERFACE_H
#define ESP_INTERFACE_H

#ifdef ESP8266

#include <ESP8266WiFi.h>
extern "C" {
    #include "user_interface.h"
}

#elif defined(ESP32)
#include <WiFi.h>
#else
#error Only ESP8266 or ESP32 platform is allowed
#endif // ESP8266

#endif // ESP_INTERFACE_H


