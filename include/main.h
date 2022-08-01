#pragma once

#include <Arduino.h>
#include "backend.h"
#include "config.h"
#include "display.h"
#include "ntp.h"
#include "xwifi.h"
#include "xdebug.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PSK;

const char* timezone_info = TZ_INFO;
const char* ntp_server = NTP_SERVER;

const char* backend_host = BACKEND_HOST;
const int backend_port = BACKEND_PORT;
const char* backend_url = BACKEND_URL;

SerialDisplay serial_display;

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
XM5Display m5_display;
#endif

Display* displays[] = {
    &serial_display
#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
    , &m5_display
#endif
};

const int nr_displays = sizeof(displays) / sizeof(Display*);

XWiFi wifi;

NTP ntp;

Backend backend;

bool drawing = false;

void debug(String msg);