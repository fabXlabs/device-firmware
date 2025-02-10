#pragma once
#include "backend.h"
#include "cardreader.h"
#include "display.h"
#include "itool.h"
#include "ntp.h"
#include "result.h"
#include "states.h"
#include "xwifi.h"
#include <Adafruit_MCP23008.h>

TaskHandle_t taskRecSound;

void playRecSoundTask(void *param) {
  const uint8_t sin_wav[] = {128, 152, 176, 198, 218, 234, 245, 253,
                             255, 253, 245, 234, 218, 198, 176, 152,
                             128, 103, 79,  57,  37,  21,  10,  2,
                             0,   2,   10,  21,  37,  57,  79,  103};
  auto spk_cfg = M5.Speaker.config();
  spk_cfg.sample_rate = 96000;
  M5.Speaker.config(spk_cfg);
  M5.Speaker.begin();
  M5.Speaker.setVolume(100);
  M5.Speaker.tone(523, 300, 0, true, sin_wav, sizeof(sin_wav));
  delay(150);
  M5.Speaker.tone(659, 300, 0, true, sin_wav, sizeof(sin_wav));
  delay(150);
  M5.Speaker.tone(784, 300, 0, true, sin_wav, sizeof(sin_wav));
  delay(150);
  M5.Speaker.tone(1047, 300, 0, true, sin_wav, sizeof(sin_wav));
  delay(150);

  M5.Speaker.stop();
  M5.Speaker.end();
  vTaskDelete(NULL);
}

class FabXDevice {

public:
  FabXDevice();
  void loop();
  void playRequestSound();
  Result addBackend(Backend &iBackend);
  Result addNTP(NTP &iNTP);
  Result addDisplay(X5Display &iDisplay);
  Result addWifi(XWiFi &iWifi);
  Result addReader(CardReader &iCardReader);
  Result addOutputExpander(Adafruit_MCP23008 &iOutputExpander);
  Result addInputExpander(Adafruit_MCP23008 &iInputExpander);

private:
  NTP *mNTP = nullptr;
  Backend *mBackend = nullptr;
  X5Display *mDisplay = nullptr;
  XWiFi *mWifi = nullptr;
  CardReader *mCardReader = nullptr;
  Adafruit_MCP23008 *mOutputExpander = nullptr;
  Adafruit_MCP23008 *mInputExpander = nullptr;
  States mCurrentState = States::INIT;
  wl_status_t mCurrentWifiState;
  int mSelectedTool = 0;
  CardReader::CardSecret mCardSecret;
  CardReader::Uid mUid;
  Backend::AuthorizedTools mAuthorizedToolIds;
  WebsocketStates mWebsocketState;
  ITool *mCurrentTool = nullptr;
  uint8_t mInputState = 0;
};

inline FabXDevice::FabXDevice() {}

inline void FabXDevice::loop() {

  switch (mCurrentState) {

  case States::INIT: {
    X_DEBUG("INIT");
    mDisplay->begin();
    mCardReader->begin();
    mWifi->begin();
    mBackend->mMac = mWifi->getMac();

    int timeout = 5000;
    do { // wait for wifi to connect
      mWifi->loop();
      mWifi->getStatus(mCurrentWifiState);
      mDisplay->clear();
      mDisplay->drawBootScreen();
      mDisplay->drawWifiStatus(mCurrentWifiState);
      mDisplay->pushCanvas();
      timeout -= 1;
    } while (mCurrentWifiState != WL_CONNECTED &&
             timeout >
                 0); // insert timeout, if timed out use cached config from sd

    if (mCurrentWifiState == WL_CONNECTED) {
      mCurrentState = States::CONFIGURE;
    }
    break;
  }
  case States::CONFIGURE: {
    X_DEBUG("CONFIGURE");
    mBackend->begin(); // start backend websocket
    mBackend->loop(mCurrentWifiState, mCurrentState, mWebsocketState);
    int timeout = 500;
    do { // wait for configuration response, after WS connect, config gets
         // requested, response handler configures the tools
      mWifi->loop();
      mWifi->getStatus(mCurrentWifiState);
      mBackend->loop(mCurrentWifiState, mCurrentState, mWebsocketState);
      mDisplay->clear();
      mDisplay->drawConfigScreen();
      mDisplay->drawWifiStatus(mCurrentWifiState);
      mDisplay->pushCanvas();
      delay(10);
      timeout -= 1;

    } while (mBackend->getState() == BackendStates::INIT && timeout > 0);
    if (mBackend->getState() == BackendStates::PROVISIONING) {
      mBackend->setupSecret(true);
      String qrCode = mBackend->mMac;
      qrCode += "\n";
      qrCode += mBackend->mSecret;
      mDisplay->clear();
      mDisplay->drawQr(qrCode);
      mDisplay->pushCanvas();

      while (!M5.BtnB.wasPressed()) {
        M5.update();
      }
      ESP.restart();
    }
    if (mBackend->getState() == BackendStates::IDLE) {
      for (ITool *tool : mBackend->mTools) {
        int pin = tool->mPin;
        IdleState state = tool->mIdleState;
        mOutputExpander->digitalWrite(pin,
                                      state == IdleState::IDLE_HIGH ? 1 : 0);
        mOutputExpander->pinMode(pin, 0);
      }
      for (int i = 0; i < 8; i++) {
        mInputExpander->pullUp(i, HIGH);
      }
      mInputState = mInputExpander->readGPIO();
      mBackend->sendPinChangedNotification(mInputState);
      mCurrentState = States::IDLE;
    }
    break;
  }
  case States::IDLE: {

    uint8_t newState = mInputExpander->readGPIO();
    if (newState != mInputState) {
      X_INFO("Changed");
      mBackend->sendPinChangedNotification(newState);
      mInputState = newState;
    }
    mCurrentTool = nullptr;
    mAuthorizedToolIds.length = 0;
    for (int i = 0; i < mUid.size; i++) {
      mUid.uidByte[i] = 0;
    }
    for (int i = 0; i < 32; i++) {
      mCardSecret.secret[i] = 0;
    }

    mDisplay->clear();
    mDisplay->drawName(mBackend->mName);
    mDisplay->drawBackground();
    mWifi->loop();
    mWifi->getStatus(mCurrentWifiState);
    mBackend->loop(mCurrentWifiState, mCurrentState, mWebsocketState);
    mDisplay->drawWifiStatus(mCurrentWifiState);
    mDisplay->pushCanvas();
    delay(10);
    if (mCurrentWifiState != WL_CONNECTED) {
      mCurrentState = States::INIT;
      return;
    }
    Result result = mCardReader->read(mUid, mCardSecret);
    if (result == Result::OK) {
      mCurrentState = States::REQUEST_AUTH_TOOLS;
    }
    BackendStates backendState = mBackend->getState();
    if (backendState == BackendStates::ERROR)
      ESP.restart();
    if (backendState == BackendStates::UNLOCK_PENDING) {
      unlockStruct currentUnlockStruct;
      mBackend->getUnlockToolData(currentUnlockStruct);
      for (ITool *tool : mBackend->mTools) {
        if (tool->mToolId == currentUnlockStruct.toolId) {
          mCurrentTool = tool;
          X_DEBUG("Tool ID: %s", mCurrentTool->mToolId.c_str());
          mBackend->sendToolUnlockResponse(currentUnlockStruct.commandId);
          mCurrentState = States::TOOL_UNLOCK;
          break;
        }
      }
    }
    if (backendState == BackendStates::CREATE_CARD_PENDING) {
      mCurrentState = States::CREATE_CARD;
      break;
    }

  } break;
  case States::REQUEST_AUTH_TOOLS: {
    playRequestSound();
    mBackend->sendGetAuthorizedTools(mUid, mCardSecret);
    int timeout = 500;
    while (mBackend->getState() != BackendStates::IDLE &&
           timeout > 0) // wait for backend to receive response from server, is
                        // list of authorized tools
    {
      wl_status_t oStatus;
      mWifi->loop();
      mWifi->getStatus(oStatus);
      mBackend->loop(oStatus, mCurrentState, mWebsocketState);
      timeout--;
      delay(10);
    }
    if (timeout == 0) {
      mCurrentState = States::IDLE;
      return;
    }
    mAuthorizedToolIds = {};
    mBackend->getAuthorizedToolsList(mAuthorizedToolIds);
    X_INFO(mAuthorizedToolIds.ToolIds[0].c_str());
    mCurrentState = States::TOOL_SELECT;
    break;
  }
  case States::TOOL_SELECT:
    // tool selection menue with scrolling menue
    // if single tool unlock instantly
    // check if second factor is needed, send validate request if needed
    // wait for ValidSecondFactorResponse or ErrorResponse
    {
      int now = millis();
      mSelectedTool = 0;
      M5.BtnA.setDebounceThresh(1);
      M5.BtnB.setDebounceThresh(1);
      M5.BtnC.setDebounceThresh(1);
      X_INFO("mAuthorizedToolIds.length %d", mAuthorizedToolIds.length);
      if (mAuthorizedToolIds.length == 0) {
        mCurrentState = States::IDLE;
        return;
      }

      while (!M5.BtnB.wasPressed() && mAuthorizedToolIds.length > 1) {
        M5.update();
        mDisplay->clear();
        mWifi->loop();
        mWifi->getStatus(mCurrentWifiState);
        mBackend->loop(mCurrentWifiState, mCurrentState, mWebsocketState);

        mDisplay->drawWifiStatus(mCurrentWifiState);
        mDisplay->drawControls(
            ((mAuthorizedToolIds.length > 5) && (mSelectedTool > 4)),
            ((mAuthorizedToolIds.length > 5) &&
             (mSelectedTool < mAuthorizedToolIds.length - 1)));
        mDisplay->drawToolList(mBackend->mTools, mAuthorizedToolIds,
                               mSelectedTool);
        mDisplay->pushCanvas();
        delay(10);
        if (M5.BtnC.wasPressed()) {
          if (mSelectedTool == 0)
            mSelectedTool = mAuthorizedToolIds.length - 1;
          else
            mSelectedTool -= 1;
          now = millis();
        }
        if (M5.BtnA.wasPressed()) {
          if (mSelectedTool == mAuthorizedToolIds.length - 1)
            mSelectedTool = 0;
          else
            mSelectedTool += 1;
          now = millis();
        }
        if (millis() > now + 10000) {
          X_DEBUG("Tool Selection Timeout");
          mCurrentState = States::IDLE;
          return;
        }
      }

      X_DEBUG("Selected Tool %d", mSelectedTool);
      for (ITool *tool : mBackend->mTools) {
        if (tool->mToolId == mAuthorizedToolIds.ToolIds[mSelectedTool]) {
          mCurrentTool = tool;
          X_DEBUG("Tool ID: %s", mCurrentTool->mToolId.c_str());
          if (tool->mRequires2FA)
            mCurrentState = States::REQUEST_SECOND_FACTOR;
          else
            mCurrentState = States::TOOL_UNLOCK;

          break;
        }
      }
    }
    break;
  case States::REQUEST_SECOND_FACTOR:
    X_DEBUG("Second Factor");
    mCurrentState = States::IDLE;
    break;
  case States::TOOL_UNLOCK: {
    int pin = mCurrentTool->mPin;
    X_DEBUG("UNLOCK Pin %d", pin);
    IdleState state = mCurrentTool->mIdleState;
    mOutputExpander->digitalWrite(pin, state == IdleState::IDLE_HIGH ? 0 : 1);
    mBackend->sendToolUnlockedNotification(mCurrentTool->mToolId, mUid,
                                           mCardSecret);
    if (mCurrentTool->mToolType == ToolType::KEEP) {
      mCurrentState = States::TOOL_KEEP;
      return;
    }
    int now = millis();
    while (millis() < now + mCurrentTool->mTime) {
      mDisplay->clear();
      mWifi->loop();
      mWifi->getStatus(mCurrentWifiState);
      mBackend->loop(mCurrentWifiState, mCurrentState, mWebsocketState);
      mDisplay->drawWifiStatus(mCurrentWifiState);
      mDisplay->drawName(mBackend->mName);
      mDisplay->drawUnlockedTool(mCurrentTool);
      mDisplay->pushCanvas();
      delay(10);
    }
    mCurrentState = States::TOOL_LOCK;

    break;
  }
  case States::TOOL_KEEP: {
    X_DEBUG("Keep");

    CardReader::Uid uid;
    CardReader::CardSecret secret;
    Result result = mCardReader->read(uid, secret);
    int lastChecked = millis();
    while (result == Result::OK) {
      if (millis() > lastChecked + 100) {
        lastChecked = millis();
        result = mCardReader->read(uid, secret);
      }
      mDisplay->clear();
      mWifi->loop();
      mWifi->getStatus(mCurrentWifiState);
      mBackend->loop(mCurrentWifiState, mCurrentState, mWebsocketState);
      mDisplay->drawWifiStatus(mCurrentWifiState);
      mDisplay->drawName(mBackend->mName);
      mDisplay->drawUnlockedTool(mCurrentTool);
      mDisplay->pushCanvas();
      M5.update();
      delay(10);
    }
    int lastValid = millis(); // if card is not present anymore
    while (millis() < (lastValid + mCurrentTool->mTime)) { // wait for time
      if (millis() > lastChecked + 100) {
        lastChecked = millis();
        result = mCardReader->read(uid, secret);
      }
      if (result == Result::OK)
        return; // check if card is present, if go back to checking regularly
      mWifi->loop();
      mWifi->getStatus(mCurrentWifiState);
      mBackend->loop(mCurrentWifiState, mCurrentState, mWebsocketState);
      int secsLeft =
          (mCurrentTool->mTime) / 1000 - (millis() - lastValid) / 1000;
      mDisplay->drawCooldown(secsLeft);
      mDisplay->pushCanvas();
      M5.update();
      delay(10);
    }
    mCurrentState = States::TOOL_LOCK;
    break;
  }
  case States::TOOL_LOCK: {
    X_DEBUG("LOCK");
    int pin = mCurrentTool->mPin;
    IdleState state = mCurrentTool->mIdleState;
    mOutputExpander->digitalWrite(pin, state == IdleState::IDLE_HIGH ? 1 : 0);
    delay(1000);
    mCurrentState = States::IDLE;
    break;
  }

  case States::CREATE_CARD: {
    X_DEBUG("Create Card");
    int start = millis();
    M5.update();
    String userName = mBackend->mCardProvisioningDetails.userName;
    long commandId = mBackend->mCardProvisioningDetails.commandId;
    String cardSecret = mBackend->mCardProvisioningDetails.cardSecret;
    X_DEBUG("Get details done");

    CardReader::Uid uid;
    mDisplay->clear();
    mWifi->loop();
    mWifi->getStatus(mCurrentWifiState);
    mBackend->loop(mCurrentWifiState, mCurrentState, mWebsocketState);
    mDisplay->drawWifiStatus(mCurrentWifiState);
    X_DEBUG("Get name");

    mDisplay->drawCardCreate(userName);
    X_DEBUG("Get name done");
    mDisplay->pushCanvas();
    X_DEBUG("Get secret");

    CardReader::CardSecret secret;
    hex2byte(cardSecret, secret);
    X_DEBUG("Get secret done");

    Result result = Result::ERROR;
    X_DEBUG("read");

    while (result != Result::OK && millis() < (start + 12000)) {
      result = mCardReader->createCard(uid, secret);
      // result = mCardReader->clearCard();
      M5.update();
      delay(10);
    }
    if (result == Result::OK) {
      X_DEBUG("command id %ld", commandId);
      mBackend->sendCardCreateResponse(commandId, uid);
    }
    mBackend->mState = BackendStates::IDLE;
    mCurrentState = States::IDLE;
    break;
  }
  default:
    break;
  }
}

inline void FabXDevice::playRequestSound() {

  xTaskCreate(playRecSoundTask, /* Function to implement the task */
              "playRecSoundT",  /* Name of the task */
              10000,            /* Stack size in words */
              NULL,             /* Task input parameter */
              100,              /* Priority of the task */
              &taskRecSound);   /* Task handle. */
}

inline Result FabXDevice::addBackend(Backend &iBackend) {
  mBackend = &iBackend;
  return Result::OK;
}

inline Result FabXDevice::addNTP(NTP &iNTP) {
  mNTP = &iNTP;
  return Result::OK;
}

inline Result FabXDevice::addDisplay(X5Display &iDisplay) {
  mDisplay = &iDisplay;
  return Result::OK;
}

inline Result FabXDevice::addWifi(XWiFi &iWifi) {
  mWifi = &iWifi;
  return Result::OK;
}

inline Result FabXDevice::addReader(CardReader &iCardReader) {
  mCardReader = &iCardReader;
  return Result::OK;
}

inline Result
FabXDevice::addOutputExpander(Adafruit_MCP23008 &iOutputExpander) {
  mOutputExpander = &iOutputExpander;
  return Result::OK;
}

inline Result FabXDevice::addInputExpander(Adafruit_MCP23008 &iInputExpander) {
  mInputExpander = &iInputExpander;
  return Result::OK;
}
