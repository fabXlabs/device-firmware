#pragma once

#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "xwifi.h"
#include "xdebug.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PSK;

SerialDisplay serial_display;

Display* displays[] = {
    &serial_display
};

XWiFi wifi;

bool drawing = false;

void debug(String msg);