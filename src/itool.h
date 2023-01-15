#pragma once
#include "result.h"


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
    ITool(String iName, ToolType iType, bool iRequires2FA, int iTime, IdleState iIdleState, int iPint, String iToolId);
    Result unlock();

public:
    String mName;
    ToolType mToolType;
    bool mRequires2FA = false;
    int mTime = 0;
    IdleState mIdleState;
    int mPin;
    String mToolId;
};

inline ITool::ITool(String iName, ToolType iType, bool iRequires2FA, int iTime, IdleState iIdleState, int iPint, String iToolId)
    :mName(iName)
    ,mToolType(iType)
    ,mRequires2FA(iRequires2FA)
    ,mTime(iTime)
    ,mIdleState(iIdleState)
    ,mPin(iPint)
    ,mToolId(iToolId)
{

}