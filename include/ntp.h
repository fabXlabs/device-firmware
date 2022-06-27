#pragma once

#include <Arduino.h>
#include <time.h>
#include "WiFiType.h"
#include "xdebug.h"

class NTP {
public:
    void begin(const char* timezone_info, const char* ntp_server) {
        tz = timezone_info;
        server = ntp_server;
    }

    void draw(Display* display) {
        display->debug("NTP draw");
        if (ntp_synced) {
            struct tm local;
            ntp_synced = getLocalTime(&local, 5000);

            const char * f = "time: %d.%m.%y Time: %H:%M:%S";
            char buf[64];
            size_t written = strftime(buf, 64, f, &local);
            debug(buf);

            // TODO draw current time on display?
        }
    }

    bool redraw_request() {
        return _redraw_request;
    }

    void reset_redraw_request() {
        _redraw_request = false;
    }

    void loop(wl_status_t wifi_status) {
        if (wifi_status == WL_CONNECTED && !ntp_synced) {
            debug("attempting sync");
		    configTzTime(tz, server);
            struct tm local;
            ntp_synced = getLocalTime(&local, 5000);
            String m = "sync done: ";
            m += ntp_synced;
            debug(m);
            _redraw_request = true;
        }
    }

private:
    bool _redraw_request = false;
    const char* tz;
    const char* server;
    bool ntp_synced = false;

    void debug(String message) {
        String tag = "[NTP] ";
        tag.concat(message);
        xdebug(tag);
    }
};