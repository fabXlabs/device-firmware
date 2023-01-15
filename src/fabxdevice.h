#pragma once
#include "result.h"
#include "ntp.h"
#include "backend.h"
#include "display.h"
#include "xwifi.h"
#include "itool.h"
#include "states.h"


class FabXDevice {

public:
    FabXDevice();
    void loop();
    Result addBackend(Backend& iBackend);
    Result addNTP(NTP& iNTP);
    Result addDisplay(X5Display& iDisplay);
    Result addWifi(XWiFi& iWifi);

private:

    NTP* mNTP = nullptr;
    Backend* mBackend = nullptr;
    X5Display* mDisplay = nullptr;
    XWiFi* mWifi = nullptr;
    States mCurrentState = States::INIT;
    wl_status_t mCurrentWifiState;
    int mSelectedTool = 0;
};

inline FabXDevice::FabXDevice()
{

}

inline void FabXDevice::loop()
{
    //handle state logic
    switch (mCurrentState){

        case States::INIT:
        {
            X_DEBUG("INIT");
            mDisplay->begin();
            mDisplay->drawBootScreen();
            mWifi->begin(); //connect Wifi
            mBackend->mMac = mWifi->getMac();
            int timeout = 1000;
            do{ //wait for wifi to connect
                mWifi->loop();
                mWifi->getStatus(mCurrentWifiState);
                mDisplay->clear();
                mDisplay->drawBootScreen();
                mDisplay->drawWifiStatus(mCurrentWifiState);
                mDisplay->pushCanvas();
                timeout -= 1;
            }while(mCurrentWifiState != WL_CONNECTED && timeout > 0); //insert timeout, if timed out use cached config from sd
            if(mCurrentWifiState == WL_CONNECTED) 
            {
                mCurrentState = States::CONFIGURE;
            }
            break;
        }
        case States::CONFIGURE:
            X_DEBUG("CONFIGURE");
            mDisplay->clear();
            mDisplay->drawConfigScreen();
            mBackend->begin(); //start backend websocket
            mBackend->loop(mCurrentWifiState, mCurrentState);

            do{ //wait for configuration response, after WS connect, config gets requested, response handler configures the tools
                mWifi->loop();
                mWifi->getStatus(mCurrentWifiState);
                mBackend->loop(mCurrentWifiState, mCurrentState);
                mDisplay->clear();
                mDisplay->drawConfigScreen();
                mDisplay->drawWifiStatus(mCurrentWifiState);
                mDisplay->pushCanvas();

            }
            while(mBackend->getState() == BackendStates::INIT); //insert timeout
            if(mBackend->getState() == BackendStates::PROVISIONING)
            {
                mBackend->setupSecret(true);
                String qrCode = mBackend->mMac;
                qrCode += "\n";
                qrCode += mBackend->mSecret;
                mDisplay->clear();
                mDisplay->drawQr(qrCode);
                mDisplay->pushCanvas();
                while(!M5.BtnB.wasPressed())
                {   
                    X_DEBUG("Waiting for Button");
                    M5.update();
                    delay(10);
                }
                ESP.restart();
            }
            mCurrentState = States::IDLE;
            break;

        case States::IDLE:
        {
            mDisplay->clear();
            String name = String(mBackend->mName);
            mDisplay->drawName(name);
            wl_status_t oStatus;
            mWifi->loop();
            mWifi->getStatus(oStatus);
            mBackend->loop(oStatus, mCurrentState);
            mDisplay->drawWifiStatus(oStatus);
            mDisplay->pushCanvas();
            if (oStatus != mCurrentWifiState)
            {
                mCurrentState = States::INIT;
                return;
            }
            if(M5.BtnB.wasPressed())
            mCurrentState = States::TOOL_SELECT;
        }
            //wait for authsources
            break;
        case States::REQUEST_AUTH_TOOLS:
            //request authorized tools with authsource
            //depending on amount of tool unlock instantly or show tool selection
            break;
        case States::TOOL_SELECT:
            //tool selection menue with scrolling menue
            {
                mDisplay->clear();
                wl_status_t oStatus;
                mWifi->loop();
                mWifi->getStatus(oStatus);
                mBackend->loop(oStatus, mCurrentState);
                mCurrentState = States::TOOL_SELECT;
                if(M5.BtnC.wasPressed())
                {
                    if(mSelectedTool == 0) mSelectedTool = mBackend->mTools.size()-1;
                    else mSelectedTool -= 1;
                }
                if(M5.BtnA.wasPressed())
                {
                    if(mSelectedTool == mBackend->mTools.size()-1) mSelectedTool = 0;
                    else mSelectedTool += 1;
                }
                if(M5.BtnB.wasPressed())
                {
                    mCurrentState = States::IDLE;
                }
                mDisplay->drawWifiStatus(oStatus);
                mDisplay->drawControls(((mBackend->mTools.size()>5)&&(mSelectedTool>4)),((mBackend->mTools.size()>5)&&(mSelectedTool<mBackend->mTools.size()-1)));
                mDisplay->drawToolList(mBackend->mTools, mSelectedTool);
                mDisplay->pushCanvas();
                delay(100);
            }
            break;
        case States::TOOL_UNLOCK:
            //unlock once go back to idle
            break;
        case States::TOOL_KEEP:
            //unlock and check reader periodically
            //if card is not present anymore lock tool again
            //go back to idle
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

