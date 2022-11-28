#pragma once

#include "trace.h"

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
#include <M5Unified.h>
#endif

class IDisplay
{
public:
    virtual void begin() = 0;
    virtual void clear() = 0;
};

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)

class X5Display : public IDisplay, public ILogger
{
public:
    void begin();
    void clear();
    void pushCanvas();
    void drawTime(int iHour, int iMin); // draw time
    void drawName(const char *iName);   // draw name
    void drawControls();                // draws arrows beside buttons
    void drawWifiStatus(wl_status_t iStatus);
    void log(const char *iMessage, DebugLevel iLevel, size_t length);

private:
    M5GFX &mLcd = M5.Display;
    M5Canvas mCanvas{&mLcd};
    bool mInit = false;
};

inline void X5Display::begin()
{
    if (!mInit)
    {
        mLcd.begin();
        mCanvas.createSprite(mLcd.width(), mLcd.height());
        mCanvas.setRotation(1);
        mCanvas.setColorDepth(8);
        mInit = true;
    }
}

inline void X5Display::clear()
{
    mCanvas.clear();
    pushCanvas();
}

inline void X5Display::pushCanvas()
{
    mCanvas.pushSprite(&mLcd, 0, 0);
}

inline void X5Display::drawTime(int iHour, int iMin)
{
    char buf[16];
    sprintf(buf, "%02i:%02i", iHour, iMin);
    mCanvas.setTextDatum(BR_DATUM);
    mCanvas.setTextColor(TFT_WHITE);
    mCanvas.setTextSize(1);
    mCanvas.drawString(buf, 240, 320);
}

inline void X5Display::drawName(const char *iName)
{
    int len = strlen(iName);
    int textsize = (240 / 6) / len;
    if (textsize > 4)
        textsize = 4;
    mCanvas.setTextDatum(TL_DATUM);
    mCanvas.setTextColor(TFT_WHITE);
    mCanvas.setTextSize(textsize);
    mCanvas.drawString(iName, 0, 0);
    mCanvas.drawLine(0, 7 * textsize + 5, 240, 7 * textsize + 5, TFT_ORANGE);
}

inline void X5Display::drawControls()
{
    int x = 220;
    mCanvas.drawTriangle(x, 45, x - 10, 60, x + 10, 60, TFT_ORANGE);
    mCanvas.drawCircle(220, 160, 5, TFT_ORANGE);
    mCanvas.drawTriangle(x, 320 - 45, x - 10, 320 - 60, x + 10, 320 - 60, TFT_ORANGE);
}

inline void X5Display::drawWifiStatus(wl_status_t iStatus)
{
    mCanvas.setTextDatum(BL_DATUM);
    mCanvas.setTextSize(1);
    mCanvas.setTextColor(TFT_WHITE);

    const int x = 0;
    const int y = 320;
    switch (iStatus)
    {
    case WL_IDLE_STATUS:
        mCanvas.drawString("WiFi IDLE", x, y);
        break;
    case WL_NO_SSID_AVAIL:
        mCanvas.setTextColor(TFT_RED);
        mCanvas.drawString("WiFi N/A", x, y);
        break;
    case WL_SCAN_COMPLETED:
        mCanvas.setTextColor(TFT_RED);
        mCanvas.drawString("WiFi COMP", x, y);
        break;
    case WL_CONNECTED:
        mCanvas.setTextColor(TFT_GREEN);
        mCanvas.drawString("WiFi CONN", x, y);
        break;
    case WL_CONNECT_FAILED:
        mCanvas.setTextColor(TFT_RED);
        mCanvas.drawString("WiFi FAIL", x, y);
        break;
    case WL_CONNECTION_LOST:
        mCanvas.setTextColor(TFT_RED);
        mCanvas.drawString("WiFi LOSS", x, y);
        break;
    case WL_DISCONNECTED:
        mCanvas.setTextColor(TFT_RED);
        mCanvas.drawString("WiFi DISC", x, y);
        break;
    default:
        mCanvas.setTextColor(TFT_WHITE);
        mCanvas.drawString("WiFi UNKN", x, y);
        break;
    }
}

inline void X5Display::log(const char *iMessage, DebugLevel iLevel, size_t length)
{
#ifdef DEBUG
    String tag;
    if (iLevel == DebugLevel::INFO_LEVEL)
    {
        tag = "[Info] ";
    }
    if (iLevel == DebugLevel::DEBUG_LEVEL)
    {
        tag = "[Debug] ";
    }
    if (iLevel == DebugLevel::ERROR_LEVEL)
    {
        tag = "[Error] ";
    }
    tag.concat(iMessage);
    mCanvas.setTextColor(TFT_RED);
    mCanvas.setTextDatum(TL_DATUM);
    mCanvas.setTextSize(1);
    mCanvas.drawString(tag, 0, mLcd.width() - 10);
    pushCanvas();
#endif
}
#endif
