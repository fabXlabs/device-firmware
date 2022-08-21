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

void setup() {
    for (int i = 0; i < sizeof(displays) / sizeof(Display *); i++) {
        displays[i]->begin();
    }

    debug("hello, world");

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
    M5.begin();
#endif

    wifi.begin(ssid, password);

    ntp.begin(timezone_info, ntp_server);
    backend.begin(backend_host, backend_port, backend_url);
}

void loop() {
    // draw if there was a redraw request in last loop iteration
    if (drawing) {
        debug("Drawing");
        for(int i = 0; i < sizeof(displays) / sizeof(Display*); i++) {
            displays[i]->start_draw();
            wifi.draw(displays[i]);
            ntp.draw(displays[i]);
            backend.draw(displays[i]);
            displays[i]->end_draw();
        }
        wifi.reset_redraw_request();
        ntp.reset_redraw_request();
        backend.reset_redraw_request();
        drawing = false;
    }

    // execute module loops
    wifi.loop();
    ntp.loop(wifi.status());
    backend.loop(wifi.status());

    // check for redraw requests
    drawing |= wifi.redraw_request();
    //drawing |= ntp.redraw_request();
    drawing |= backend.redraw_request();

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
    M5.update();

    // TODO introduce abstraction for input
    if (M5.BtnA.wasClicked()) {
        debug("BTN A CLICK");
    }
#endif
}

void debug(String msg) {
    String tag = "[main] ";
    tag.concat(msg);
    xdebug(tag);
};