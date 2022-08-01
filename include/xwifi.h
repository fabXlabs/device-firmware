#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <esp32-hal-bt.c>
#include <esp_wifi.h>
#include "display.h"
#include "xdebug.h"

class XWiFi {
public:
    void begin(const char* ssid, const char *passphrase) {
        WiFi.begin(ssid, passphrase);
        esp_err_t wifi_success = esp_wifi_set_ps(WIFI_PS_NONE);
        if (wifi_success == ESP_OK) {
            debug("WiFi set to no powersaving");
        } else {
            debug("Error when trying to set WiFi to no powersaving");
        }
    }

    void draw(Display* display) {
        display->wifi_status(_status);
    }

    bool redraw_request() {
        return _redraw_request;
    }

    void reset_redraw_request() {
        _redraw_request = false;
    }

    void loop() {
        wl_status_t status_new = WiFi.status();
        if (status_new != _status) {
            _status = status_new;
            _redraw_request = true;
        }
    }

    wl_status_t status() {
        return _status;
    }

private:
    bool _redraw_request = false;
    wl_status_t _status;

    void debug(String message) {
        String tag = "[WiFi] ";
        tag.concat(message);
        xdebug(tag);
    }
};