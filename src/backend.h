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
    "MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw\n"
    "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
    "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjAwOTA0MDAwMDAw\n"
    "WhcNMjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n"
    "RW5jcnlwdDELMAkGA1UEAxMCUjMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"
    "AoIBAQC7AhUozPaglNMPEuyNVZLD+ILxmaZ6QoinXSaqtSu5xUyxr45r+XXIo9cP\n"
    "R5QUVTVXjJ6oojkZ9YI8QqlObvU7wy7bjcCwXPNZOOftz2nwWgsbvsCUJCWH+jdx\n"
    "sxPnHKzhm+/b5DtFUkWWqcFTzjTIUu61ru2P3mBw4qVUq7ZtDpelQDRrK9O8Zutm\n"
    "NHz6a4uPVymZ+DAXXbpyb/uBxa3Shlg9F8fnCbvxK/eG3MHacV3URuPMrSXBiLxg\n"
    "Z3Vms/EY96Jc5lP/Ooi2R6X/ExjqmAl3P51T+c8B5fWmcBcUr2Ok/5mzk53cU6cG\n"
    "/kiFHaFpriV1uxPMUgP17VGhi9sVAgMBAAGjggEIMIIBBDAOBgNVHQ8BAf8EBAMC\n"
    "AYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMBMBIGA1UdEwEB/wQIMAYB\n"
    "Af8CAQAwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYfr52LFMLGMB8GA1UdIwQYMBaA\n"
    "FHm0WeZ7tuXkAXOACIjIGlj26ZtuMDIGCCsGAQUFBwEBBCYwJDAiBggrBgEFBQcw\n"
    "AoYWaHR0cDovL3gxLmkubGVuY3Iub3JnLzAnBgNVHR8EIDAeMBygGqAYhhZodHRw\n"
    "Oi8veDEuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYGZ4EMAQIBMA0GCysGAQQB\n"
    "gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W\n"
    "PTNlclQtgaDqw+34IL9fzLdwALduO/ZelN7kIJ+m74uyA+eitRY8kc607TkC53wl\n"
    "ikfmZW4/RvTZ8M6UK+5UzhK8jCdLuMGYL6KvzXGRSgi3yLgjewQtCPkIVz6D2QQz\n"
    "CkcheAmCJ8MqyJu5zlzyZMjAvnnAT45tRAxekrsu94sQ4egdRCnbWSDtY7kh+BIm\n"
    "lJNXoB1lBMEKIq4QDUOXoRgffuDghje1WrG9ML+Hbisq/yFOGwXD9RiX8F6sw6W4\n"
    "avAuvDszue5L3sz85K+EC4Y/wFVDNvZo4TYXao6Z0f+lQKc0t8DQYzk1OXVu8rp2\n"
    "yJMC6alLbBfODALZvYH7n7do1AZls4I9d1P4jnkDrQoxB3UqQ9hVl3LEKQ73xF1O\n"
    "yK5GhDDX8oVfGKF5u+decIsH4YaTw7mP3GFxJSqv3+0lUFJoi5Lc5da149p90Ids\n"
    "hCExroL1+7mryIkXPeFM5TgO9r0rvZaBFOvV2z0gp35Z0+L4WPlbuEjN/lxPFin+\n"
    "HlUjr8gRsI3qfJOQFy/9rKIJR0Y/8Omwt/8oTWgy1mdeHmmjk7j1nYsvC9JSQ6Zv\n"
    "MldlTTKB3zhThV1+XWYp6rjd5JW1zbVWEkLNxE7GJThEUG3szgBVGP7pSWTUTsqX\n"
    "nLRbwHOoq7hHwg==\n"
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
      oWebsocketState = WebsocketStates::AVAILABLE;
    else
      oWebsocketState = WebsocketStates::UNAVAILABLE;
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
  CloseReason reason = sBackend.getCloseReason();

  switch (event) {
  case WebsocketsEvent::ConnectionOpened:
    X_DEBUG("connected");
    sBackend.sendGetConfig();

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
