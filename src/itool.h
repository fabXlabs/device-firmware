#pragma once
#include "result.h"
#include "backend.h"


enum class ToolType
{
    UNLOCK,
    KEEP
};

enum class IdleState
{
    IDLE_LOW,
    IDLE_HIGH
};


class ITool 
{
public:
    ITool(char* iName, ToolType iType, bool iRequires2FA, int iTime, IdleState iIdleState);
    virtual Result unlock() = 0;
    virtual bool requires2FA() = 0;

private:
    char*  mName = "Test";
    ToolType mToolType;
    bool mRequires2FA = false;
    int mTime = 0;
    IdleState mIdleState;
    int mPin;
};