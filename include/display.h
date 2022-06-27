#pragma once

#include "Arduino.h"
#include "WiFiType.h"

class Display {
public: 
    virtual void begin() = 0;
    virtual void debug(String message) = 0;
    virtual void wifi_status(wl_status_t status);
    virtual ~Display() {}
};

class SerialDisplay : public Display {
public:
    virtual void begin() {
        Serial.begin(115200);
    }

    virtual void debug(String message) {
        Serial.print("[DEBUG] ");
        Serial.println(message);
    }

    virtual void wifi_status(wl_status_t status) {
        Serial.print("[WiFi] status: ");
        switch (status){
        case WL_IDLE_STATUS:
            Serial.println("IDLE");
            break;
        case WL_NO_SSID_AVAIL:
            Serial.println("NO_SSID_AVAIL");
            break;
        case WL_SCAN_COMPLETED:
            Serial.println("SCAN_COMPLETED");
            break;
        case WL_CONNECTED:
            Serial.println("CONNECTED");
            break;
        case WL_CONNECT_FAILED:
            Serial.println("CONNECT_FAILED");
            break;
        case WL_CONNECTION_LOST:
            Serial.println("CONNECTION_LOST");
            break;
        case WL_DISCONNECTED:
            Serial.println("DISCONNECTED");
            break;
        default:
            Serial.println("UNKNOWN");
            break;
        }
    }
};