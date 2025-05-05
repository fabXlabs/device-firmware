#pragma once
#include "cardreader.h"
#include "trace.h"
#include <Arduino.h>

enum class States {
  INIT,
  CONFIGURE,
  IDLE,
  REQUEST_AUTH_TOOLS,
  REQUEST_SECOND_FACTOR,
  REQUEST_SECOND_FACTOR_VALIDATION,
  TOOL_SELECT,
  TOOL_UNLOCK,
  TOOL_KEEP,
  TOOL_LOCK,
  CREATE_CARD
};

enum class BackendStates {
  UNINIT,
  INIT,
  PROVISIONING,
  IDLE,
  ERROR,
  WAITING,
  UNLOCK_PENDING,
  CREATE_CARD_PENDING,
};

typedef struct {
  long commandId;
  String toolId;
} unlockStruct;

typedef struct {
  long commandId;
  String userName;
  String cardSecret;
} cardProvisioningDetails;

typedef struct {
  long commandId;
  bool pending;
  bool valid;
} secondFactorValidation;

enum class WebsocketStates {
  AVAILABLE,
  UNAVAILABLE,
  RECONNECT,
};

void hex2byte(String iHex, CardReader::CardSecret &iSecret) {

  for (int i = 0, j = 0; i < iHex.length(); i += 2, j++) {
    String byteString = iHex.substring(i, i + 2);
    X_DEBUG(byteString.c_str());
    uint8_t byteValue = static_cast<uint8_t>(strtol(byteString.c_str(), 0, 16));
    X_DEBUG("converted %d", byteValue);
    iSecret.secret[j] = byteValue;
  }
}