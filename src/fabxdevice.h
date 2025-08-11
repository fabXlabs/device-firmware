#pragma once
#include "backend.h"
#include "cardreader.h"
#include "display.h"
#include "itool.h"
#include "keypad.h"
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
  void update();
  void playRequestSound();
  Result addBackend(Backend &iBackend);
  Result addNTP(NTP &iNTP);
  Result addDisplay(X5Display &iDisplay);
  Result addWifi(XWiFi &iWifi);
  Result addReader(CardReader &iCardReader);
  Result addOutputExpander(Adafruit_MCP23008 &iOutputExpander);
  Result addInputExpander(Adafruit_MCP23008 &iInputExpander);
  Result addKeypad(Keypad &iKeypad);

private:
  NTP *mNTP = nullptr;
  Backend *mBackend = nullptr;
  X5Display *mDisplay = nullptr;
  XWiFi *mWifi = nullptr;
  CardReader *mCardReader = nullptr;
  Adafruit_MCP23008 *mOutputExpander = nullptr;
  Adafruit_MCP23008 *mInputExpander = nullptr;
  Keypad *mKeypad = nullptr;
  States mCurrentState = States::INIT;
  wl_status_t mCurrentWifiState;
  int mSelectedTool = 0;
  CardReader::CardSecret mCardSecret;
  CardReader::Uid mUid;
  Backend::AuthorizedTools mAuthorizedToolIds;
  WebsocketStates mWebsocketState;
  ITool *mCurrentTool = nullptr;
  uint8_t mInputState = 0;
  bool mKeypadPresent = false;
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
      update();
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
        if (tool->mRequires2FA) {
          mKeypadPresent = true;
        }
      }
      for (int i = 0; i < 8; i++) {
        mInputExpander->pullUp(i, HIGH);
      }
      mInputState = mInputExpander->readGPIO();
      mBackend->sendPinChangedNotification(mInputState);

      if (mKeypadPresent) {
        mKeypad->begin();
      }

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
    mKeypad->setCommand(Keypad::Command::IDLE);
    update();

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
      update();
      timeout--;
      delay(10);
    }
    if (timeout == 0) {
      mCurrentState = States::IDLE;
      return;
    }
    if (mBackend->mAuthorizedTools.pending)
      return;

    if (!mBackend->mAuthorizedTools.authOk) {
      mKeypad->setCommand(Keypad::Command::AUTH_FAIL_CARD);
      int now = millis();
      while (millis() < now + 1000) {
        mDisplay->clear();
        update();
        mDisplay->drawWifiStatus(mCurrentWifiState);
        mDisplay->drawName(mBackend->mName);
        mDisplay->pushCanvas();
        delay(10);
      }
      mKeypad->setCommand(Keypad::Command::IDLE);
      mCurrentState = States::IDLE;
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
        mKeypad->setCommand(Keypad::Command::AUTH_FAIL_CARD);
        int now = millis();
        while (millis() < now + 1000) {
          mDisplay->clear();
          update();
          mDisplay->drawWifiStatus(mCurrentWifiState);
          mDisplay->drawName(mBackend->mName);
          mDisplay->drawNoToolAccess();
          mDisplay->pushCanvas();
          delay(10);
        }
        mKeypad->setCommand(Keypad::Command::IDLE);
        mCurrentState = States::IDLE;
        return;
      }

      while (!M5.BtnB.wasPressed() && mAuthorizedToolIds.length > 1) {
        mDisplay->clear();
        update();

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
        if (mSelectedTool < mAuthorizedToolIds.length &&
            tool->mToolId == mAuthorizedToolIds.ToolIds[mSelectedTool]) {
          mCurrentTool = tool;
          X_DEBUG("Tool ID: %s", mCurrentTool->mToolId.c_str());
          if (tool->mRequires2FA)
            mCurrentState = States::REQUEST_SECOND_FACTOR;
          else
            mCurrentState = States::TOOL_UNLOCK;

          break;
        }
      }

      // Tool not found, e.g. backend configuration was changed
      if (mCurrentState == States::TOOL_SELECT) {
        X_DEBUG("Tool %s not found",
                mAuthorizedToolIds.ToolIds[mSelectedTool].c_str());
        mCurrentState = States::IDLE;
      }
    }
    break;
  case States::REQUEST_SECOND_FACTOR: {
    X_DEBUG("Requesting 2FA from keypad");
    mKeypad->setCommand(Keypad::Command::CODE_REQUIRED);
    int stateWasEnterCodeCount = 0;
    Keypad::State state = mKeypad->getState();
    Keypad::State prevState = state;
    bool internalTimeout = true;
    unsigned long now = millis();
    while (millis() < now + 10000) {
      state = mKeypad->getState();
      if (prevState != state && state == Keypad::State::TYPING) {
        X_DEBUG("User is typing");
      }
      prevState = state;
      // first wait for keypad to enter ENTER_CODE state
      // (at least 5 iterations, as sometimes there are some glitches)
      // then wait for user to finish entering code
      const bool stateIsEnterCode = (state == Keypad::State::ENTER_CODE ||
                                     state == Keypad::State::TYPING);
      if (stateWasEnterCodeCount > 5 && !stateIsEnterCode) {
        internalTimeout = false;
        break;
      }
      if (stateIsEnterCode) {
        // stop forcing the keypad to a status
        // to allow it to enter the CODE_READY state
        mKeypad->setCommand(Keypad::Command::NO_COMMAND);
        stateWasEnterCodeCount++;
      }

      mDisplay->clear();
      update();
      mDisplay->drawWifiStatus(mCurrentWifiState);
      mDisplay->drawName(mBackend->mName);
      mDisplay->draw2FARequest();
      mDisplay->pushCanvas();
      // X_DEBUG("Keypad wait C:%d S:%d T:%d",
      //               static_cast<int>(mKeypad->getCommand()),
      //               static_cast<int>(mKeypad->getState()), millis()-now);
      // X_DEBUG("Keypad wait %08lx T:%d", mKeypad->getRaw(), millis()-now);
      delay(10);
    }

    if (internalTimeout) {
      X_DEBUG("Getting 2FA from keypad timed out");
    } else {
      X_DEBUG("Keypad returned to state %d", static_cast<int>(state));
    }

    switch (state) {
    case Keypad::State::CODE_READY:
      X_DEBUG("Got code from keypad");
      mBackend->sendValidateSecondFactor(mKeypad->getCodeAsString(), mUid,
                                         mCardSecret);
      mCurrentState = States::REQUEST_SECOND_FACTOR_VALIDATION;
      break;
    case Keypad::State::IDLE: // timeout in keypad
      X_DEBUG("Keypad timed out during code entry");
      // fall through
    default:
      mCurrentState = States::IDLE;
      break;
    }
    break;
  }
  case States::REQUEST_SECOND_FACTOR_VALIDATION: {
    mKeypad->setCommand(Keypad::Command::ACT_BUSY);
    int timeout = 500;
    while (mBackend->getState() != BackendStates::IDLE &&
           timeout > 0) // wait for backend to receive response from server, is
                        // list of authorized tools
    {
      update();
      timeout--;
      delay(10);
    }
    if (timeout == 0) {
      mCurrentState = States::IDLE;
      return;
    }
    X_DEBUG("2FA response: pending=%d valid=%d",
            mBackend->mSecondFactorCheck.pending,
            mBackend->mSecondFactorCheck.valid);
    if (!mBackend->mSecondFactorCheck.pending) {
      if (mBackend->mSecondFactorCheck.valid) {
        X_DEBUG("2FA OK");
        mDisplay->pushCanvas();
        mKeypad->setCommand(Keypad::Command::AUTH_OK);
        mCurrentState = States::TOOL_UNLOCK;
      } else {
        X_DEBUG("2FA FAIL");
        mKeypad->setCommand(Keypad::Command::AUTH_FAIL_CODE);
        int now = millis();
        while (millis() < now + 1000) {
          mDisplay->clear();
          update();
          mDisplay->drawWifiStatus(mCurrentWifiState);
          mDisplay->drawName(mBackend->mName);
          mDisplay->draw2FAResult(false);
          mDisplay->pushCanvas();
          delay(10);
        }
        mKeypad->setCommand(Keypad::Command::IDLE);
        mCurrentState = States::IDLE;
      }
    } else {
      mKeypad->setCommand(Keypad::Command::NO_COMMAND);
      mCurrentState = States::IDLE;
    }
    if (mKeypadPresent)
      mKeypad->update();
    break;
  }
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
      mKeypad->setCommand(Keypad::Command::TOOL_UNLOCKED);
      update();
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
      mKeypad->setCommand(Keypad::Command::TOOL_UNLOCKED);
      update();
      mDisplay->drawWifiStatus(mCurrentWifiState);
      mDisplay->drawName(mBackend->mName);
      mDisplay->drawUnlockedTool(mCurrentTool);
      mDisplay->pushCanvas();
      delay(10);
    }
    int lastValid = millis(); // if card is not present anymore
    while (millis() < (lastValid + mCurrentTool->mTime)) { // wait for time
      if (millis() > lastChecked + 100) {
        lastChecked = millis();
        result = mCardReader->read(uid, secret);
      }
      if (result == Result::OK) {
        bool equal = true;
        for (uint8_t i = 0; i < 32; ++i) {
          if (secret.secret[i] != mCardSecret.secret[i])
            equal = false;
        }
        if (equal)
          return;
      }
      update();
      int secsLeft =
          (mCurrentTool->mTime) / 1000 - (millis() - lastValid) / 1000;
      mDisplay->drawCooldown(secsLeft);
      mDisplay->pushCanvas();
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
    int now = millis();
    while (millis() < now + 1000) {
      mDisplay->clear();
      mKeypad->setCommand(Keypad::Command::TOOL_LOCKED);
      update();
      mDisplay->drawWifiStatus(mCurrentWifiState);
      mDisplay->drawName(mBackend->mName);
      mDisplay->pushCanvas();
      delay(10);
    }
    mKeypad->setCommand(Keypad::Command::IDLE);
    mCurrentState = States::IDLE;
    break;
  }

  case States::CREATE_CARD: {
    X_DEBUG("Create Card");
    int start = millis();
    String userName = mBackend->mCardProvisioningDetails.userName;
    long commandId = mBackend->mCardProvisioningDetails.commandId;
    String cardSecret = mBackend->mCardProvisioningDetails.cardSecret;
    X_DEBUG("Get details done");

    CardReader::Uid uid;
    mDisplay->clear();
    update();
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

inline void FabXDevice::update() {
  mWifi->loop();
  mWifi->getStatus(mCurrentWifiState);
  mBackend->loop(mCurrentWifiState, mCurrentState, mWebsocketState);
  if (mKeypadPresent)
    mKeypad->update();
  M5.update();
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

inline Result FabXDevice::addKeypad(Keypad &iKeypad) {
  mKeypad = &iKeypad;
  return Result::OK;
}