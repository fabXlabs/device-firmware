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