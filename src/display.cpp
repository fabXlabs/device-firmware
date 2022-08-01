#include "display.h"

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
void XM5Display::begin() {
    lcd.begin();
}

void XM5Display::clear() {
    lcd.clearDisplay();
}

void XM5Display::debug(String message) {
    // noop
}

void XM5Display::time(int hour, int min) {
    lcd.setTextDatum(BR_DATUM);
    lcd.setTextSize(3);
    lcd.setTextColor(TFT_WHITE);
    char buf[8];
    sprintf(buf, "%02i:%02i", hour, min);
    lcd.drawString(buf, 320, 240);
}

void XM5Display::name(String name) {
    lcd.setTextDatum(TL_DATUM);
    lcd.setTextSize(3);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString(name, 0, 0);
}

void XM5Display::wifi_status(wl_status_t status) {
    lcd.setTextDatum(TR_DATUM);
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_WHITE);
    
    switch (status){
    case WL_IDLE_STATUS:
        lcd.drawString("WiFi IDLE", 320, 12);
        break;
    case WL_NO_SSID_AVAIL:
        lcd.setTextColor(TFT_RED);
        lcd.drawString("WiFi N/A", 320, 12);
        break;
    case WL_SCAN_COMPLETED:
        lcd.setTextColor(TFT_RED);
        lcd.drawString("WiFi COMP", 320, 12);
        break;
    case WL_CONNECTED:
        lcd.setTextColor(TFT_GREEN);
        lcd.drawString("WiFi CONN", 320, 12);
        break;
    case WL_CONNECT_FAILED:
        lcd.setTextColor(TFT_RED);
        lcd.drawString("WiFi FAIL", 320, 12);
        break;
    case WL_CONNECTION_LOST:
        lcd.setTextColor(TFT_RED);
        lcd.drawString("WiFi LOSS", 320, 12);
        break;
    case WL_DISCONNECTED:
        lcd.setTextColor(TFT_RED);
        lcd.drawString("WiFi DISC", 320, 12);
        break;
    default:
        lcd.setTextColor(TFT_WHITE);
        lcd.drawString("WiFi UNKN", 320, 12);
        Serial.println("UNKNOWN");
        break;
    }
}
#endif