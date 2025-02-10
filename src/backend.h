#pragma once
#include "FS.h"
#include "SPIFFS.h"
#include "cardreader.h"
#include "config.h"
#include "itool.h"
#include "states.h"
#include "trace.h"
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <EEPROM.h>
#include <HTTPUpdate.h>
#include <tiny_websockets/internals/wscrypto/crypto.hpp>

using namespace websockets;

const char echo_org_ssl_ca_cert[] PROGMEM =
    "-----BEGIN CERTIFICATE-----\n"
    "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\n"
    "-----END CERTIFICATE-----\n";

class Backend {

public:
  struct AuthorizedTools {
    String ToolIds[8];
    size_t length;
  };

  Backend(const char *iHost, const int iPort, const char *iUrl,
          const char *iFirmwareVersion);
  void begin();
  void loop(wl_status_t iWifiStatus, States iCurrentState,
            WebsocketStates &oWebsocketState);
  BackendStates getState();
  void sendGetConfig();
  void sendGetAuthorizedTools(CardReader::Uid &iUid,
                              CardReader::CardSecret &secret);
  void sendToolUnlockedNotification(String iToolId, CardReader::Uid &iUid,
                                    CardReader::CardSecret &iSecret);
  void getAuthorizedToolsList(AuthorizedTools &iAuthorizedTools);
  void sendPinChangedNotification(uint8_t iPinState);
  void getUnlockToolData(unlockStruct &oUnlockStruct);
  CloseReason getCloseReason();
  void sendToolUnlockResponse(long commandId);
  void setupSecret(bool iForceNew = false);

private:
  void sendDeviceRestartResponse(long commandId);
  void sendErrorResponse(long commandId, String message);
  void sendDeviceUpdateResponse(long commandId);
  void handleText(const char *iPayload);
  void downloadBg();
  void reconnect();

  static void websocketEventCallback(WebsocketsEvent event, String data);
  static void websocketMsgCallback(WebsocketsMessage message);

  void handleUnlockToolCommand(DynamicJsonDocument &doc);
  void handleRestartDeviceCommand(long iId);
  void handleConfigurationResponse(DynamicJsonDocument &doc);
  void handleAuthorizedToolsResponse(DynamicJsonDocument &doc);
  void handleUpdateDeviceFirmware(DynamicJsonDocument &doc);

private:
  const char *mHost;
  const int mPort;
  const char *mUrl;
  const char *mFirmwareVersion;
  WebsocketsClient mWebSocket;
  WiFiClientSecure mWificlient;
  unlockStruct mUnlockStruct;
  uint32_t mReconnectTime = 0;

  // method add authsource
  // save pointer to authsource
  // class authsource
  // get_auth return: json

public: // TODO make those private and add getters/setters
  String mName;
  String mBackgroundURL;
  String mSecret;
  String mMac;
  std::vector<ITool *> mTools;
  bool mRestartRequest = false;
  bool mUnlockRequest = false;
  long mCurrentCommandId = 0;
  BackendStates mState = BackendStates::UNINIT;
  WebsocketStates mWsState = WebsocketStates::UNAVAILABLE;
  AuthorizedTools mAuthorizedTools;
};

extern Backend sBackend;

inline Backend::Backend(const char *iHost, const int iPort, const char *iUrl,
                        const char *iFirmwareVersion)
    : mHost(iHost), mPort(iPort), mUrl(iUrl),
      mFirmwareVersion(iFirmwareVersion) {}

inline void Backend::begin() {
  X_DEBUG("Backend Begin");
  String wsString = "wss://";
  wsString += mHost;
  wsString += mUrl;
  X_DEBUG("Connection details: %s", wsString.c_str());

  if (mSecret.length() == 0) {
    sBackend.setupSecret();
  }
  X_DEBUG("Device Mac: %s", mMac.c_str());
  String auth = mMac;
  auth += ":";
  auth += mSecret;
  WSString base64 =
      crypto::base64Encode((uint8_t *)auth.c_str(), auth.length());
  String authstr = "Basic ";
  authstr += internals::fromInternalString(base64);
  if (mState == BackendStates::UNINIT) {
    mWebSocket.addHeader("Authorization", authstr);
    mWebSocket.onEvent(websocketEventCallback);
    mWebSocket.onMessage(websocketMsgCallback);
    mWebSocket.setCACert(echo_org_ssl_ca_cert);
    // mWebSocket.setInsecure();
  }

  mWebSocket.connect(wsString);
  mState = BackendStates::INIT;
}

inline void Backend::setupSecret(bool iForceNew) {

  EEPROM.begin(SECRET_LENGTH + 1);
  byte magic = EEPROM.readByte(0);

  if ((magic != 0x42) || iForceNew) {
    X_DEBUG("Generating new Secret!");
    // generate new secret
    EEPROM.writeByte(0, 0x42);
    for (int i = 0; i < SECRET_LENGTH; i++) {
      EEPROM.writeByte(i + 1, (byte)esp_random());
    }
  } else {
    X_DEBUG("Reading Secret from EEPROM");
  }
  sBackend.mSecret = "";
  char buffer[3];
  for (int i = 0; i < SECRET_LENGTH; i++) {
    byte b = EEPROM.readByte(i + 1);
    sprintf(buffer, "%02x", b);
    sBackend.mSecret += buffer;
  }
  EEPROM.end();
}

inline void Backend::loop(wl_status_t iWifiStatus, States iCurrentState,
                          WebsocketStates &oWebsocketState) {
  if (iWifiStatus == WL_CONNECTED) {
    mWebSocket.poll();
    if (mWebSocket.available())
      mWsState = WebsocketStates::AVAILABLE;
    else {
      mWsState = WebsocketStates::RECONNECT;
      if (mReconnectTime + 5000 <= millis()) {
        reconnect();
        mReconnectTime = millis();
      }
    }
  }
  if (mRestartRequest) {
    mRestartRequest = false;
    if (iCurrentState == States::IDLE) {
      handleRestartDeviceCommand(mCurrentCommandId);
    } else {
      sendErrorResponse(mCurrentCommandId, "Device in use.");
    }
  }
}

inline BackendStates Backend::getState() { return mState; }

inline CloseReason Backend::getCloseReason() {
  return mWebSocket.getCloseReason();
}

inline void Backend::websocketEventCallback(WebsocketsEvent event,
                                            String data) {
  if (sBackend.mWsState == WebsocketStates::RECONNECT) {
    return;
  }
  CloseReason reason = sBackend.getCloseReason();

  switch (event) {
  case WebsocketsEvent::ConnectionOpened:
    X_DEBUG("connected");
    sBackend.sendGetConfig();
    sBackend.mWsState = WebsocketStates::AVAILABLE;
    break;
  case WebsocketsEvent::ConnectionClosed:
    switch (reason) {
    case CloseReason::CloseReason_PolicyViolation:
      X_DEBUG("Policy Violation");
      sBackend.mState = BackendStates::PROVISIONING;
      break;
    case CloseReason::CloseReason_AbnormalClosure:
      X_DEBUG("Abnormal Closure");
      break;
    default:
      X_DEBUG("disconnected reason: %d", reason);
      sBackend.mState = BackendStates::ERROR;
      break;
    }

  case WebsocketsEvent::GotPing:
    X_DEBUG("ping");
    break;
  case WebsocketsEvent::GotPong:
    X_DEBUG("pong");
    break;
  default:
    X_DEBUG("other websocket event");
    break;
  }
}

inline void Backend::websocketMsgCallback(WebsocketsMessage message) {
  sBackend.handleText(message.data().c_str());
}

inline void Backend::sendGetConfig() {
  X_DEBUG("sending get config");
  int commandId = (int)esp_random();
  mCurrentCommandId = commandId;
  String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws.GetConfiguration\","
               "\"commandId\":";
  msg += commandId;
  msg += ", \"actualFirmwareVersion\": \"";
  msg += mFirmwareVersion;
  msg += "\"}";
  if (!mWebSocket.send(msg)) {
    X_DEBUG("sending get config failed");
  }
}

inline void Backend::sendGetAuthorizedTools(CardReader::Uid &iUid,
                                            CardReader::CardSecret &iSecret) {
  X_DEBUG("sending get auth tools");
  int commandId = (int)esp_random();
  mCurrentCommandId = commandId;
  String id;
  for (byte i = 0; i < iUid.size; i++) {
    char buf[3];
    sprintf(buf, "%02X", iUid.uidByte[i]);
    id += String(buf);
  }
  String secret;
  for (byte i = 0; i < 32; i++) {
    char buf[3];
    sprintf(buf, "%02X", iSecret.secret[i]);
    secret += String(buf);
  }
  X_DEBUG("ID: %s", id.c_str());
  String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws."
               "GetAuthorizedTools\",\"commandId\":";
  msg += commandId;
  msg += ",\"phoneNrIdentity\":null,\"cardIdentity\":{\"cardId\":\"";
  msg += id;
  msg += "\",\"cardSecret\":\"";
  msg += secret;
  msg += "\"}}";
  if (!mWebSocket.send(msg)) {
    X_DEBUG("sending get authorized tools failed");
    return;
  }
  mState = BackendStates::WAITING;
}

inline void
Backend::sendToolUnlockedNotification(String iToolId, CardReader::Uid &iUid,
                                      CardReader::CardSecret &iSecret) {
  String id;
  for (byte i = 0; i < iUid.size; i++) {
    char buf[3];
    sprintf(buf, "%02X", iUid.uidByte[i]);
    id += String(buf);
  }
  String secret;
  for (byte i = 0; i < 32; i++) {
    char buf[3];
    sprintf(buf, "%02X", iSecret.secret[i]);
    secret += String(buf);
  }
  String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws."
               "ToolUnlockedNotification\",\"toolId\":\"";
  msg += iToolId;
  msg += "\",\"phoneNrIdentity\":null,\"cardIdentity\":{\"cardId\":\"";
  msg += id;
  msg += "\",\"cardSecret\":\"";
  msg += secret;
  msg += "\"}}";
  if (!mWebSocket.send(msg)) {
    X_DEBUG("sending get authorized tools failed");
    return;
  }
}

inline void Backend::sendPinChangedNotification(uint8_t iPinState) {
  String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws."
               "PinStatusNotification\",\"inputPins\":{";
  bool value;
  uint8_t pinState = iPinState;
  for (uint8_t i = 0; i < 8; i++) {
    X_DEBUG("%d", pinState);
    value = pinState & 1;
    msg += "\"" + String(i) + "\":";
    msg += value ? "true" : "false";
    if (i != 7)
      msg += ",";
    pinState = pinState >> 1;
  }
  msg += "}}";
  X_DEBUG(msg.c_str());

  if (!mWebSocket.send(msg)) {
    X_DEBUG("sending get authorized tools failed");
    return;
  }
}

inline void Backend::getAuthorizedToolsList(AuthorizedTools &iAuthorizedTools) {
  iAuthorizedTools = mAuthorizedTools;
  mAuthorizedTools.length = 0;
}

inline void Backend::sendToolUnlockResponse(long commandId) {
  String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws."
               "ToolUnlockResponse\",\"commandId\":";
  msg += commandId;
  msg += "}";

  X_DEBUG("sending tool unlock response %s", msg.c_str());

  if (!mWebSocket.send(msg)) {
    X_DEBUG("sending tool unlock response failed");
  }
}

inline void Backend::sendDeviceRestartResponse(long commandId) {
  String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws."
               "DeviceRestartResponse\",\"commandId\":";
  msg += commandId;
  msg += "}";

  X_DEBUG("sending device restart response");

  if (!mWebSocket.send(msg)) {
    X_DEBUG("sending device restart response failed");
  }
}

inline void Backend::sendErrorResponse(long commandId, String message) {
  String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws.ErrorResponse\","
               "\"message\":\"";
  msg += message;
  msg += "\",\"commandId\":";
  msg += commandId;
  msg += "\"parameters\":{},\"correlationId\":null";
  msg += "}";
  X_DEBUG("sending error response");
  if (!mWebSocket.send(msg)) {
    X_DEBUG("sending device restart response failed");
  }
}

inline void Backend::sendDeviceUpdateResponse(long commandId) {
  String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws."
               "UpdateFirmwareResponse\",\"commandId\":";
  msg += commandId;
  msg += "}";

  X_DEBUG("sending device update response");

  if (!mWebSocket.send(msg)) {
    X_DEBUG("sending device update response failed");
  }
}

inline void Backend::handleText(const char *iPayload) {
  DynamicJsonDocument doc(2048);
  DynamicJsonDocument response(1024);
  X_DEBUG("text %s ", (char *)iPayload);

  DeserializationError deserialization_error;
  deserialization_error = deserializeJson(doc, iPayload);

  if (deserialization_error) {
    X_DEBUG("deserialization failed: %s ", deserialization_error.f_str());
    return;
  }
  sBackend.mCurrentCommandId = doc["commandId"];
  if (strcmp(doc["type"],
             "cloud.fabX.fabXaccess.device.ws.ConfigurationResponse") == 0) {
    sBackend.handleConfigurationResponse(doc);
  } else if (strcmp(doc["type"],
                    "cloud.fabX.fabXaccess.device.ws.UnlockTool") == 0) {
    sBackend.handleUnlockToolCommand(doc);
  } else if (strcmp(doc["type"],
                    "cloud.fabX.fabXaccess.device.ws.RestartDevice") == 0) {
    sBackend.mRestartRequest = true;
  } else if (strcmp(
                 doc["type"],
                 "cloud.fabX.fabXaccess.device.ws.AuthorizedToolsResponse") ==
             0) {
    sBackend.handleAuthorizedToolsResponse(doc);
  } else if (strcmp(doc["type"],
                    "cloud.fabX.fabXaccess.device.ws.UpdateDeviceFirmware") ==
             0) {
    sBackend.handleUpdateDeviceFirmware(doc);
  }
}

inline void Backend::handleUnlockToolCommand(DynamicJsonDocument &doc) {
  X_DEBUG("handling unlock tool command");
  long commandId = doc["commandId"];
  String toolId = doc["toolId"];
  mUnlockStruct.commandId = commandId;
  mUnlockStruct.toolId = toolId;
  mState = BackendStates::UNLOCK_PENDING;
}

inline void Backend::getUnlockToolData(unlockStruct &oUnlockStruct) {
  oUnlockStruct = mUnlockStruct;

  mState = BackendStates::IDLE;
}

inline void Backend::handleRestartDeviceCommand(long iId) {
  X_DEBUG("handling restart device command");
  sendDeviceRestartResponse(iId);
  delay(3000);
  ESP.restart();
}
inline void Backend::downloadBg() {
  File bgFile = SPIFFS.open("/fablab.bmp", "w");
  if (bgFile) {
    X_INFO("Downloading Background image...");
    HTTPClient hc;

    hc.begin(mBackgroundURL);
    X_INFO("Background image url: %s\n", mBackgroundURL.c_str());

    int httpCode = hc.GET();
    if (httpCode == HTTP_CODE_OK) {
      hc.writeToStream(&bgFile);
      Serial.println();
      bgFile.close();
    } else {
      X_ERROR("Did not recieve HTTP OK: %i\n", httpCode);
    }

    bgFile.close();
  } else {
    X_ERROR("Could not open SPIFFS /bg.jpg");
  }
}

inline void Backend::reconnect() {
  String wsString = "wss://";
  wsString += mHost;
  wsString += mUrl;
  X_DEBUG("Connection details: %s", wsString.c_str());
  mWebSocket.close();
  mWebSocket.connect(wsString);
  X_INFO("Reconnecting started");
  mWsState = WebsocketStates::UNAVAILABLE;
}

inline void Backend::handleConfigurationResponse(DynamicJsonDocument &doc) {
  mTools.clear();
  X_DEBUG("handling configuration response");
  if (doc["commandId"] != mCurrentCommandId)
    return;
  sBackend.mName = String((const char *)doc["name"]);
  sBackend.mBackgroundURL = String((const char *)doc["background"]);
  int i = 0;
  JsonObject attachedTools = doc["attachedTools"];
  for (JsonPair tool : attachedTools) {
    // parse Json
    JsonObject toolData = tool.value();
    int pin = atoi(tool.key().c_str());
    bool r2FA = toolData["requires2FA"];
    int time = toolData["time"];
    String name = toolData["name"];
    ToolType type;
    if (toolData["type"] == "UNLOCK")
      type = ToolType::UNLOCK;
    if (toolData["type"] == "KEEP")
      type = ToolType::KEEP;
    IdleState idleState;
    if (toolData["idleState"] == "IDLE_LOW")
      idleState = IdleState::IDLE_LOW;
    if (toolData["idleState"] == "IDLE_HIGH")
      idleState = IdleState::IDLE_HIGH;
    // create tool
    String id = toolData["id"];
    ITool *currentTool = new ITool(name, type, r2FA, time, idleState, pin, id);
    if (!currentTool) {
      X_DEBUG("Allocation error");
      return;
    }
    mTools.push_back(currentTool);
    ITool *testTool = mTools.back();
    Serial.printf("Tool: Name: %s, Pin: %d ID: %s\n", testTool->mName.c_str(),
                  testTool->mPin, testTool->mToolId.c_str());
    // TODO save this to the sd card
  }

  mState = BackendStates::IDLE;
}

inline void Backend::handleAuthorizedToolsResponse(DynamicJsonDocument &doc) {
  X_DEBUG("Handling Authorized Tool response");
  JsonArray toolIds = doc["toolIds"].as<JsonArray>();
  int index = 0;
  for (JsonVariant toolId : toolIds) {
    X_DEBUG("Tool ID");
    mAuthorizedTools.ToolIds[index] = String(toolId.as<const char *>());
    X_DEBUG("ToolId : %s", toolId.as<const char *>());
    mAuthorizedTools.length = ++index;
  }
  mState = BackendStates::IDLE;
}

inline void Backend::handleUpdateDeviceFirmware(DynamicJsonDocument &doc) {
  X_DEBUG("Handling device update response");
  long commandId = doc["commandId"];
  sendDeviceUpdateResponse(commandId);
  String str = "https://";
  str += mHost;
  str += "/api/v1/device/me/firmware-update";
  X_DEBUG(str.c_str());
  mWificlient.setCACert(echo_org_ssl_ca_cert);
  // mWificlient.setInsecure();
  httpUpdate.update(mWificlient, str.c_str(), mFirmwareVersion,
                    [this](HTTPClient *client) {
                      client->setAuthorization(mMac.c_str(), mSecret.c_str());
                    });
}
