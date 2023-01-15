#pragma once

enum class States{
    INIT,
    CONFIGURE,
    IDLE,
    REQUEST_AUTH_TOOLS,
    TOOL_SELECT,
    TOOL_UNLOCK,
    TOOL_KEEP,
};

enum class BackendStates{
    INIT,
    PROVISIONING,
    CONFIGURED,
};
