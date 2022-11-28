#pragma once
#include "result.h"
#include "ntp.h"
#include "backend.h"
#include "display.h"
#include "xwifi.h"
#include <LinkedList.h>
#include "itool.h"

class FabXDevice {


enum class States{
    INIT,
    IDLE,
    TOOL_SELECT,
    TOOL_UNLOCK,
    TOOL_KEEP,
};

public:
    FabXDevice();
    void loop();
    Result addBackend(Backend& iBackend);
    Result addNTP(NTP& iNTP);
    Result addDisplay(X5Display& iDisplay);
    Result addWifi(XWiFi& iWifi);

private:

    Result displayTime();
    Result displayName();
    Result displayWifi();
    Result displayControls();


    // LinkedList<ITool> mTools;
    NTP* mNTP = nullptr;
    Backend* mBackend = nullptr;
    X5Display* mDisplay = nullptr;
    XWiFi* mWifi = nullptr;
    States mCurrentState = States::INIT;
};

inline FabXDevice::FabXDevice()
{

}

inline void FabXDevice::loop()
{
    //handle state logic
    switch (mCurrentState){
        case States::INIT:
            X_DEBUG("INIT");
            mWifi->begin();
            mDisplay->begin();

            wl_status_t oStatus;
            while(oStatus != WL_CONNECTED){
                mWifi->getStatus(oStatus);
                break;
            }
            //setup secrets
            mBackend->begin();
            mCurrentState = States::IDLE;
            break;
        case States::IDLE:
            //display idle screen
            //ask authsources for auths;
            mWifi->getStatus(oStatus);
            mBackend->loop(oStatus);
            mWifi->loop();
            sleep(0.1);
            break;
        case States::TOOL_SELECT:
            break;
        case States::TOOL_UNLOCK:
            break;
        case States::TOOL_KEEP:
            break;
        default:
            break;
    }

}

inline Result FabXDevice::addBackend(Backend& iBackend)
{
    mBackend = &iBackend;
    return Result::OK;
}

inline Result FabXDevice::addNTP(NTP& iNTP)
{
    mNTP = &iNTP;
    return Result::OK;
}

inline Result FabXDevice::addDisplay(X5Display& iDisplay)
{
    mDisplay = &iDisplay;
    return Result::OK;
}

inline Result FabXDevice::addWifi(XWiFi& iWifi)
{
    mWifi = &iWifi;
    return Result::OK;
}