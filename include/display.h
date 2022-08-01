#pragma once

#include "Arduino.h"
#include "WiFiType.h"

#if defined(ARDUINO_M5Stack_Core_ESP32)
#include "M5Stack.h"
#include "M5Display.h"
#endif

#if defined(ARDUINO_M5STACK_Core2)
#include "M5Unified.h"
#endif

class Display {
public: 
    virtual void begin() = 0;
    virtual void clear() = 0;
    virtual void debug(String message) = 0;
    virtual void name(String name);
    virtual void time(int hour, int min);
    virtual void wifi_status(wl_status_t status);
    virtual ~Display() {}
};

class SerialDisplay : public Display {
public:
    virtual void begin() {
        Serial.begin(115200);
    }

    virtual void clear() {}

    virtual void debug(String message) {
        Serial.print("[DEBUG] ");
        Serial.println(message);
    }

    virtual void time(int hour, int min) {
        Serial.printf("[Time] %02i:%02i\n", hour, min);
    }

    virtual void name(String name) {
        Serial.print("[Name] ");
        Serial.println(name);
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

#if defined(ARDUINO_M5Stack_Core_ESP32)
class XM5Display : public Display {
public:
    virtual void begin();
    virtual void clear();
    virtual void debug(String message);
    virtual void name(String name);
    virtual void time(int hour, int min);
    virtual void wifi_status(wl_status_t status);
private:
    M5Display lcd;
};
#endif

#if defined(ARDUINO_M5STACK_Core2)
class XM5Display : public Display {
public:
    virtual void begin();
    virtual void clear();
    virtual void debug(String message);
    virtual void name(String name);
    virtual void time(int hour, int min);
    virtual void wifi_status(wl_status_t status);
private:
    M5GFX & lcd = M5.Display;
};
#endif