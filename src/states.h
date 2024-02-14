#pragma once
#include <Arduino.h>


enum class States{
    INIT,
    CONFIGURE,
    IDLE,
    REQUEST_AUTH_TOOLS,
    REQUEST_SECOND_FACTOR,
    TOOL_SELECT,
    TOOL_UNLOCK,
    TOOL_KEEP,
    TOOL_LOCK,
};

enum class BackendStates{
    UNINIT,
    INIT,
    PROVISIONING,
    IDLE,
    ERROR,
    WAITING,
    UNLOCK_PENDING,
};


typedef struct {
    long commandId;
    String toolId;
} unlockStruct;

enum class WebsocketStates{
    AVAILABLE,
    UNAVAILABLE,
};
