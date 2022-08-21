#pragma once

#include <Arduino.h>
#include <time.h>
#include "WiFiType.h"
#include "xdebug.h"

#define CONFIG_LWIP_SNTP_UPDATE_DELAY 15000

class NTP {
public:
    void begin(const char* timezone_info, const char* ntp_server) {
        tz = timezone_info;
        server = ntp_server;
    }

    void draw(Display* display) {
        if (ntp_synced) {
            struct tm local;
            ntp_synced = getLocalTime(&local, 1000);

            // const char * f = "time: %d.%m.%y Time: %H:%M:%S";
            // char buf[64];
            // size_t written = strftime(buf, 64, f, &local);
            // debug(buf);

            display->time(hour, min, sec);
        }
    }

    bool redraw_request() {
        return _redraw_request;
    }

    void reset_redraw_request() {
        _redraw_request = false;
    }

    void loop(wl_status_t wifi_status) {
        struct tm local;
        if (wifi_status == WL_CONNECTED && !ntp_synced) {
            debug("attempting sync");
		    configTzTime(tz, server);
            ntp_synced = getLocalTime(&local, 5000);
            String m = "sync done: ";
            m += ntp_synced;
            debug(m);
            _redraw_request = true;
        }

        unsigned long current_time = millis();

        if (ntp_synced && (current_time - last_time_check > 1000)) {
            last_time_check = current_time;

            time_t now;
            time(&now);
            localtime_r(&now, &local);
            if (local.tm_hour != hour) {
                hour = local.tm_hour;
                _redraw_request = true;
            }
            if (local.tm_min != min) {
                min = local.tm_min;
                _redraw_request = true;
            }
            if (local.tm_sec != sec) {
                sec = local.tm_sec;
                _redraw_request = true;
            }
        }
    }

private:
    bool _redraw_request = false;
    const char* tz;
    const char* server;
    bool ntp_synced = false;
    unsigned long last_time_check = 0;
    int hour = -1;
    int min = -1;
    int sec = -1;

    void debug(String message) {
        String tag = "[NTP] ";
        tag.concat(message);
        xdebug(tag);
    }
};