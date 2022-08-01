#include "display.h"

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
void XM5Display::begin() {
    lcd.begin();
    canvas.createSprite(lcd.width(), lcd.height());
}

void XM5Display::start_draw(){
    lcd.startWrite();
    canvas.clearDisplay();
}

void XM5Display::end_draw(){
    canvas.pushSprite(&lcd, 0, 0);
    lcd.endWrite();
}

void XM5Display::debug(String message) {
    // noop
}

void XM5Display::time(int hour, int min, int sec) {
    canvas.setTextDatum(BR_DATUM);
    canvas.setTextSize(3);
    canvas.setTextColor(TFT_WHITE);
    char buf[16];
    sprintf(buf, "%02i:%02i:%02i", hour, min, sec);
    canvas.drawString(buf, 320, 240);
}

void XM5Display::name(String name) {
    canvas.setTextDatum(TL_DATUM);
    canvas.setTextSize(3);
    canvas.setTextColor(TFT_WHITE);
    canvas.drawString(name, 0, 0);
}

void XM5Display::wifi_status(wl_status_t status) {
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