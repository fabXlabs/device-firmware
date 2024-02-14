#pragma once

#include "trace.h"
#include "iTool.h"
#include "backend.h"
#include <HTTPClient.h>
#include <M5GFX.h>
#include "FS.h"
#include "SPIFFS.h"

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
    void drawName(String iName);   // draw name
    void drawControls(bool iFillUp = false, bool iFillDown = false);// draws arrows beside buttons
    void drawBootScreen();
    void drawConfigScreen();
    void drawUnlockedTool(ITool* iTool);
    void drawCooldown(int iTime);
    void drawWifiStatus(wl_status_t iStatus);
    void drawBackground();
    void drawToolList(std::vector<ITool*> &iToolList, Backend::AuthorizedTools iAuthorizedTools, int iSelected);
    void drawQr(String iQr);
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
    mCanvas.clear(TFT_BLACK);
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

inline void X5Display::drawName(String iName)
{
    int len = iName.length();
    int textsize;
    if(len!= 0){
        textsize = (240 / 6) / len;
    }
    else
    {
        textsize = 4;
    }

    if (textsize > 4)
        textsize = 4;
    mCanvas.setTextDatum(TL_DATUM);
    mCanvas.setTextColor(TFT_WHITE);
    mCanvas.setTextSize(textsize);
    mCanvas.drawString(iName, 0, 0);
    mCanvas.drawLine(0, 7 * textsize + 5, 240, 7 * textsize + 5, TFT_ORANGE);
}

inline void X5Display::drawBackground()
{
    mCanvas.drawBmpFile(SPIFFS,"/fablab.bmp",0,50,300,300,0,0,1,1);
}


inline void X5Display::drawControls(bool iFillUp, bool iFillDown)
{
    int x = 220;
    if(iFillUp)
    {
        mCanvas.fillTriangle(x, 45, x - 10, 60, x + 10, 60, TFT_ORANGE);
    }
    else
    {
        mCanvas.drawTriangle(x, 45, x - 10, 60, x + 10, 60, TFT_ORANGE);
    }
    mCanvas.drawCircle(220, 160, 5, TFT_ORANGE);

    if(iFillDown)
    {
        mCanvas.fillTriangle(x, 320 - 45, x - 10, 320 - 60, x + 10, 320 - 60, TFT_ORANGE);
    }
    else
    {
        mCanvas.drawTriangle(x, 320 - 45, x - 10, 320 - 60, x + 10, 320 - 60, TFT_ORANGE);
    }
}
inline void X5Display::drawBootScreen()
{
    mCanvas.setTextDatum(TL_DATUM);
    mCanvas.setTextColor(TFT_WHITE);
    mCanvas.setTextSize(2);
    mCanvas.drawString("Booted!", 0, 20);
    mCanvas.drawString("Connecting to WiFi", 0, 40);
}

inline void X5Display::drawConfigScreen()
{
    mCanvas.setTextDatum(TL_DATUM);
    mCanvas.setTextColor(TFT_WHITE);
    mCanvas.setTextSize(2);
    mCanvas.drawString("Configuring...", 0, 20);
    mCanvas.drawString("Establishing backend connection...", 0, 40);
}

inline void X5Display::drawUnlockedTool(ITool* iTool)
{
    mCanvas.setTextDatum(MC_DATUM);
    mCanvas.setTextColor(TFT_GREEN);
    mCanvas.setTextSize(3);
    mCanvas.drawString(iTool->mName, 120, 100);
    if (iTool->mToolType == ToolType::KEEP)
    {
        mCanvas.setTextColor(TFT_WHITE);
        mCanvas.setTextSize(2);
        mCanvas.drawString("Keep Card in", 120, 160);
        mCanvas.drawString("Range", 120, 190);

    }
}

inline void X5Display::drawCooldown(int iTime)
{
    char time[5];
    sprintf(time,"%i",iTime);

    mCanvas.clearDisplay(TFT_RED);
    mCanvas.setTextDatum(MC_DATUM);
    mCanvas.setTextColor(TFT_BLACK);
    mCanvas.setTextSize(7);
    mCanvas.drawString(time, 120, 160);
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

inline void X5Display::drawToolList(std::vector<ITool*> &iList, Backend::AuthorizedTools iAuthorizedTools, int iSelected)
{
    
    int listSize = iAuthorizedTools.length;
    int maxSize = 5;
    std::vector<ITool*> filteredList;
    for(int i = 0; i < iList.size();i++)
    {
        ITool* tool = iList.at(i);
        for (int j = 0; j<iAuthorizedTools.length; j++)
        {
            if(tool->mToolId == iAuthorizedTools.ToolIds[j])
            {
                filteredList.push_back(tool);
            }
        }
    }

    mCanvas.setTextSize(3);
    mCanvas.setTextColor(TFT_WHITE);
    mCanvas.setTextDatum(ML_DATUM);

    int start = 0;
    if (iSelected > (maxSize-1) && iAuthorizedTools.length>maxSize-1)
    {
        start = iSelected-(maxSize-1);
    }
    int j = 0;
    for (int i = start; i < filteredList.size(); i++)
    {
        if(j==iSelected-start)
        {
            mCanvas.setTextColor(TFT_ORANGE);
        }
        ITool* tool = filteredList.at(i);
        //bool draw = false;
        //for (int k = 0; k<listSize; k++)
        //{   
        //   if (tool->mToolId == iAuthorizedTools.ToolIds[k])
        //    {
        //      draw = true;
        //        break;
        //    }
//
        //}
        //if (!draw) continue;
        if (j>maxSize-1) break;
        String name = String(tool->mName);
        mCanvas.drawString(name,0,60+50*(j));
        j++;
        mCanvas.setTextColor(TFT_WHITE);
    }


}

inline void X5Display::drawQr(String iQr)
{
    mCanvas.qrcode(iQr.c_str());
    mCanvas.setTextColor(TFT_WHITE);
    mCanvas.setTextDatum(ML_DATUM);
    mCanvas.drawString("Push Select for Reboot", 0,300);
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
