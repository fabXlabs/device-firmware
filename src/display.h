#pragma once

#include "FS.h"
#include "SPIFFS.h"
#include "backend.h"
#include "itool.h"
#include "trace.h"
#include <HTTPClient.h>
#include <M5GFX.h>

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
#include <M5Unified.h>
#endif

class IDisplay {
public:
  virtual void begin() = 0;
  virtual void clear() = 0;
};

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)

class ToolListState {
  public:
    bool entryClicked() const { return mSelectedIndex >= 0; }
    int getSelectedIndex() const { return mSelectedIndex; }
    void setSelectedIndex(int iIndex) { mSelectedIndex = iIndex; }

    int mScrollPositionBeforeDrag = 0;
    int mScrollPosition = 1000; // always start at top (will be clamped on first render)
  private:
    int mSelectedIndex = -1;
};

class X5Display : public IDisplay, public ILogger {
public:
  void begin();
  void clear();
  void pushCanvas();
  void drawTime(int iHour, int iMin); // draw time
  void drawName(String iName);        // draw name
  void drawControls(bool iFillUp = false,
                    bool iFillDown = false); // draws arrows beside buttons
  void drawBootScreen();
  void drawConfigScreen();
  void drawCardCreate(String iUsername);
  void drawNoToolAccess();
  void draw2FARequest();
  void draw2FAResult(bool ok);
  void drawUnlockedTool(ITool *iTool);
  void drawCooldown(int iTime);
  void drawWifiStatus(wl_status_t iStatus);
  void drawBackground();
  bool drawToolList(std::vector<ITool *> &iToolList, ToolListState &iState);
  void drawQr(String iQr);
  void log(const char *iMessage, DebugLevel iLevel, size_t length);

private:
  static int autosizeText(String text, int width, int maxTextSize = 4);

  M5GFX &mLcd = M5.Display;
  M5Canvas mCanvas{&mLcd};
  bool mInit = false;
};

inline int X5Display::autosizeText(String text, int width, int maxTextSize) {
  int len = text.length();
  int textsize;
  if (len != 0) {
    textsize = (width / 6) / len;
  } else {
    textsize = maxTextSize;
  }

  if (textsize > maxTextSize)
    textsize = maxTextSize;
  return textsize;
}

inline void X5Display::begin() {
  if (!mInit) {
    mLcd.begin();

    mCanvas.createSprite(mLcd.width(), mLcd.height());
    mCanvas.setRotation(1);
    mCanvas.setColorDepth(8);

    M5.Touch.setFlickThresh(16);

    // rotate touchscreen to match the LCD
    auto touchConfig = mLcd.touch()->config();
    touchConfig.offset_rotation = 1;
    mLcd.touch()->config(touchConfig);

    // fix touch calibration, without y goes from -80 .. 240
    uint16_t h = static_cast<uint16_t>(mLcd.height());
    uint16_t w = static_cast<uint16_t>(mLcd.width());
    uint16_t offset = w - h;
    uint16_t parameters[8] =
      { offset, 0
      , offset, w
      , static_cast<uint16_t>(offset+h), 0
      , static_cast<uint16_t>(offset+h), w };
    M5.Display.setTouchCalibrate(parameters);

    mInit = true;
  }
}

inline void X5Display::clear() { mCanvas.clear(TFT_BLACK); }

inline void X5Display::pushCanvas() { mCanvas.pushSprite(&mLcd, 0, 0); }

inline void X5Display::drawTime(int iHour, int iMin) {
  char buf[16];
  sprintf(buf, "%02i:%02i", iHour, iMin);
  mCanvas.setTextDatum(BR_DATUM);
  mCanvas.setTextColor(TFT_WHITE);
  mCanvas.setTextSize(1);
  mCanvas.drawString(buf, 240, 320);
}

inline void X5Display::drawName(String iName) {
  int textsize = X5Display::autosizeText(iName, mLcd.height());
  mCanvas.setTextDatum(TL_DATUM);
  mCanvas.setTextColor(TFT_WHITE);
  mCanvas.setTextSize(textsize);
  mCanvas.drawString(iName, 0, 0);
  mCanvas.drawLine(0, 7 * textsize + 5, 240, 7 * textsize + 5, TFT_ORANGE);
}

inline void X5Display::drawBackground() {
  mCanvas.drawBmpFile(SPIFFS, "/fablab.bmp", 0, 50, 300, 300, 0, 0, 1, 1);
}

inline void X5Display::drawControls(bool iFillUp, bool iFillDown) {
  int x = 220;
  if (iFillUp) {
    mCanvas.fillTriangle(x, 45, x - 10, 60, x + 10, 60, TFT_ORANGE);
  } else {
    mCanvas.drawTriangle(x, 45, x - 10, 60, x + 10, 60, TFT_ORANGE);
  }
  mCanvas.drawCircle(x, 160, 5, TFT_ORANGE);

  if (iFillDown) {
    mCanvas.fillTriangle(x, 320 - 45, x - 10, 320 - 60, x + 10, 320 - 60,
                         TFT_ORANGE);
  } else {
    mCanvas.drawTriangle(x, 320 - 45, x - 10, 320 - 60, x + 10, 320 - 60,
                         TFT_ORANGE);
  }
}
inline void X5Display::drawBootScreen() {
  mCanvas.setTextDatum(TL_DATUM);
  mCanvas.setTextColor(TFT_WHITE);
  mCanvas.setTextSize(2);
  mCanvas.drawString("Booted!", 0, 20);
  mCanvas.drawString("Connecting to WiFi", 0, 40);
}

inline void X5Display::drawConfigScreen() {
  mCanvas.setTextDatum(TL_DATUM);
  mCanvas.setTextColor(TFT_WHITE);
  mCanvas.setTextSize(2);
  mCanvas.drawString("Configuring...", 0, 20);
  mCanvas.drawString("Establishing backend connection...", 0, 40);
}

inline void X5Display::drawCardCreate(String iUsername) {
  mCanvas.setTextDatum(TL_DATUM);
  mCanvas.setTextColor(TFT_WHITE);
  mCanvas.setTextSize(2);
  mCanvas.drawString("Create Card Mode...", 0, 20);
  mCanvas.drawString("Scan empty card to provision for fabx", 0, 40);
  mCanvas.drawString(iUsername, 0, 60);
}

inline void X5Display::drawNoToolAccess() {
  mCanvas.setTextDatum(MC_DATUM);
  mCanvas.setTextColor(TFT_RED);
  mCanvas.setTextSize(3);
  mCanvas.drawString("Unqualified", 120, 100);
}

inline void X5Display::draw2FARequest() {
  mCanvas.setTextDatum(MC_DATUM);
  mCanvas.setTextColor(TFT_YELLOW);
  mCanvas.setTextSize(3);
  mCanvas.drawString("Wait for 2FA", 120, 100);
}

inline void X5Display::draw2FAResult(bool ok) {
  mCanvas.setTextDatum(MC_DATUM);
  mCanvas.setTextColor(ok ? TFT_GREEN : TFT_RED);
  mCanvas.setTextSize(3);
  mCanvas.drawString(ok ? "2FA OK" : "2FA FAIL", 120, 100);
}

inline void X5Display::drawUnlockedTool(ITool *iTool) {
  mCanvas.setTextDatum(MC_DATUM);
  mCanvas.setTextColor(TFT_GREEN);
  mCanvas.setTextSize(3);
  mCanvas.drawString(iTool->mName, 120, 100);
  if (iTool->mToolType == ToolType::KEEP) {
    mCanvas.setTextColor(TFT_WHITE);
    mCanvas.setTextSize(2);
    mCanvas.drawString("Keep Card in", 120, 160);
    mCanvas.drawString("Range", 120, 190);
  }
}

inline void X5Display::drawCooldown(int iTime) {
  char time[5];
  sprintf(time, "%i", iTime);

  mCanvas.clearDisplay(TFT_RED);
  mCanvas.setTextDatum(MC_DATUM);
  mCanvas.setTextColor(TFT_BLACK);
  mCanvas.setTextSize(7);
  mCanvas.drawString(time, 120, 160);
}

inline void X5Display::drawWifiStatus(wl_status_t iStatus) {
  mCanvas.setTextDatum(BL_DATUM);
  mCanvas.setTextSize(1);
  mCanvas.setTextColor(TFT_WHITE);

  const int x = 0;
  const int y = 320;
  switch (iStatus) {
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

inline bool X5Display::drawToolList(std::vector<ITool *> &iToolList,
                                    ToolListState &iState) {

  mCanvas.setTextSize(3);
  mCanvas.setTextColor(TFT_WHITE);
  mCanvas.setTextDatum(ML_DATUM);

  const int rowHeight = 70; //mCanvas.fontHeight() * 2;
  const int buttonHorzPadding = 2;
  const int yOffset = rowHeight/2; // drawString draws text vertically centered
  const int buttonWidth = 240;
  const int buttonVertMargin = 2;
  const int scrollIndicatorBarHeight = 10;

  const int topScrollPos = scrollIndicatorBarHeight;
  const int botScrollPos = -rowHeight * iToolList.size() + mLcd.width() - 2*scrollIndicatorBarHeight;

  m5::touch_detail_t touchEvent = M5.Touch.getDetail();
  const int touchedIndex = (touchEvent.y - iState.mScrollPosition) / rowHeight;
  const bool validIndex = touchedIndex >= 0 && touchedIndex < iToolList.size();
  bool anyInteraction = false;

  // event handling
  if (touchEvent.wasClicked()) {
    if (validIndex) {
      anyInteraction = true;
      iState.setSelectedIndex(touchedIndex);
      return true;
    }
  }

  if (touchEvent.isFlicking()) {
    anyInteraction = true;
    iState.mScrollPosition = iState.mScrollPositionBeforeDrag + touchEvent.distanceY();
  }

  // clamp scroll position
  if (iState.mScrollPosition > topScrollPos) iState.mScrollPosition = topScrollPos;
  if (iState.mScrollPosition < botScrollPos) iState.mScrollPosition = botScrollPos;

  if (touchEvent.wasFlicked()) {
    iState.mScrollPositionBeforeDrag = iState.mScrollPosition;
  }

  // rendering
  for (int i = 0; i < iToolList.size(); i++) {
    ITool *tool = iToolList.at(i);
    String name = String(tool->mName);

    int textsize = X5Display::autosizeText(name,
                      buttonWidth - 2*buttonHorzPadding, 3);
    mCanvas.setTextSize(textsize);

    const int yPos = yOffset + rowHeight * i +  iState.mScrollPosition;
    if (!touchEvent.isFlicking() && !touchEvent.isDragging() && touchEvent.isPressed() && validIndex && touchedIndex == i) {
      anyInteraction = true;
      mCanvas.setTextColor(TFT_BLACK);
      mCanvas.fillRoundRect(0, yPos - yOffset + buttonVertMargin, buttonWidth, rowHeight - buttonVertMargin*2, 10, TFT_LIGHTGRAY);
    }
    else {
      mCanvas.setTextColor(TFT_WHITE);
      mCanvas.drawRoundRect(0, yPos - yOffset + buttonVertMargin, buttonWidth, rowHeight - buttonVertMargin*2, 10, TFT_ORANGE);
    }
    //mCanvas.drawString(name, buttonHorzPadding, yPos);
    mCanvas.setTextDatum(textdatum_t::middle_center);
    mCanvas.drawString(name, mCanvas.width()/2, yPos);
    mCanvas.setTextColor(TFT_WHITE);

    // top scroll indicator bar
    mCanvas.fillRect(0,  0, mLcd.height(), scrollIndicatorBarHeight, BLACK);
    if (iState.mScrollPosition < topScrollPos) {
      const int x = mLcd.height()/2;
      const int y = scrollIndicatorBarHeight/2;
      mCanvas.fillTriangle(x, y - 5, x - 7, y + 5, x + 7, y + 5, ORANGE);
    }

    // bottom scroll indicator bar
    mCanvas.fillRect(0,  mLcd.width()-scrollIndicatorBarHeight, mLcd.height(), scrollIndicatorBarHeight, BLACK);
    if (iState.mScrollPosition > botScrollPos) {
      const int x = mLcd.height()/2;
      const int y = mLcd.width() - scrollIndicatorBarHeight/2;
      mCanvas.fillTriangle(x, y + 5, x - 7, y - 5, x + 7, y - 5, ORANGE);
    }
  }
  return anyInteraction;
}

inline void X5Display::drawQr(String iQr) {
  mCanvas.qrcode(iQr.c_str());
  mCanvas.setTextColor(TFT_WHITE);
  mCanvas.setTextDatum(ML_DATUM);
  mCanvas.drawString("Push Select for Reboot", 0, 300);
}

inline void X5Display::log(const char *iMessage, DebugLevel iLevel,
                           size_t length) {
#ifdef DEBUG
  String tag;
  if (iLevel == DebugLevel::INFO_LEVEL) {
    tag = "[Info] ";
  }
  if (iLevel == DebugLevel::DEBUG_LEVEL) {
    tag = "[Debug] ";
  }
  if (iLevel == DebugLevel::ERROR_LEVEL) {
    tag = "[Error] ";
  }
  tag.concat(iMessage);
  mCanvas.setTextColor(TFT_RED);
  mCanvas.setTextDatum(BL_DATUM);
  mCanvas.setTextSize(1);
  mCanvas.drawString(tag, 0, mLcd.width() - 10);
  pushCanvas();
#endif
}
#endif
