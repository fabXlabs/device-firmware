#pragma once

#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "ntp.h"
#include "xwifi.h"
#include "xdebug.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PSK;
const char* timezone_info = TZ_INFO;
const char* ntp_server = NTP_SERVER;

SerialDisplay serial_display;

Display* displays[] = {
    &serial_display
};

XWiFi wifi;

NTP ntp;

bool drawing = false;

void debug(String msg);