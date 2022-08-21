#pragma once

#include "Arduino.h"
#include "WiFiType.h"

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
#include "M5Unified.h"
#endif

class Display {
public: 
    virtual void begin() = 0;
    virtual void start_draw() = 0;
    virtual void end_draw() = 0;
    virtual void debug(String message) = 0;
    virtual void name(String name);
    virtual void time(int hour, int min, int sec);
    virtual void wifi_status(wl_status_t status);
    virtual ~Display() {}
};

class SerialDisplay : public Display {
public:
    virtual void begin() {
        Serial.begin(115200);
    }

    virtual void start_draw() {}

    virtual void end_draw() {}

    virtual void debug(String message) {
        Serial.print("[DEBUG] ");
        Serial.println(message);
    }

    virtual void time(int hour, int min, int sec) {
        // Serial.printf("[Time] %02i:%02i\n", hour, min);
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

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
class XM5Display : public Display {
public:
    virtual void begin();
    virtual void start_draw();
    virtual void end_draw();
    virtual void debug(String message);
    virtual void name(String name);
    virtual void time(int hour, int min, int sec);
    virtual void wifi_status(wl_status_t status);
private:
    M5GFX & lcd = M5.Display;
    M5Canvas canvas { &lcd };
};
#endif


#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
inline void XM5Display::begin() {
    lcd.begin();
    canvas.setColorDepth(8);
    canvas.createSprite(lcd.width(), lcd.height());
}

inline void XM5Display::start_draw(){
    lcd.startWrite();
    canvas.clearDisplay();
}

inline void XM5Display::end_draw(){
    canvas.pushSprite(&lcd, 0, 0);
    lcd.endWrite();
}

inline void XM5Display::debug(String message) {
    // noop
}

inline void XM5Display::time(int hour, int min, int sec) {
    canvas.setTextDatum(BR_DATUM);
    canvas.setTextSize(3);
    canvas.setTextColor(TFT_WHITE);
    char buf[16];
    sprintf(buf, "%02i:%02i:%02i", hour, min, sec);
    canvas.drawString(buf, 320, 240);
}

inline void XM5Display::name(String name) {
    canvas.setTextDatum(TL_DATUM);
    canvas.setTextSize(3);
    canvas.setTextColor(TFT_WHITE);
    canvas.drawString(name, 0, 0);
}

inline void XM5Display::wifi_status(wl_status_t status) {
    canvas.setTextDatum(TR_DATUM);
    canvas.setTextSize(1);
    canvas.setTextColor(TFT_WHITE);
    
    switch (status){
    case WL_IDLE_STATUS:
        canvas.drawString("WiFi IDLE", 320, 12);
        break;
    case WL_NO_SSID_AVAIL:
        canvas.setTextColor(TFT_RED);
        canvas.drawString("WiFi N/A", 320, 12);
        break;
    case WL_SCAN_COMPLETED:
        canvas.setTextColor(TFT_RED);
        canvas.drawString("WiFi COMP", 320, 12);
        break;
    case WL_CONNECTED:
        canvas.setTextColor(TFT_GREEN);
        canvas.drawString("WiFi CONN", 320, 12);
        break;
    case WL_CONNECT_FAILED:
        canvas.setTextColor(TFT_RED);
        canvas.drawString("WiFi FAIL", 320, 12);
        break;
    case WL_CONNECTION_LOST:
        canvas.setTextColor(TFT_RED);
        canvas.drawString("WiFi LOSS", 320, 12);
        break;
    case WL_DISCONNECTED:
        canvas.setTextColor(TFT_RED);
        canvas.drawString("WiFi DISC", 320, 12);
        break;
    default:
        canvas.setTextColor(TFT_WHITE);
        canvas.drawString("WiFi UNKN", 320, 12);
        Serial.println("UNKNOWN");
        break;
    }
}
#endif