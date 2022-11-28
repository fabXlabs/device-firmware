#pragma once

#include <Arduino.h>
#include "trace.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiClientSecure.h>

class XWiFi
{
public:
    XWiFi(const char *iSsid, const char *iPassphrase);
    void begin();
    void loop();
    void getStatus(wl_status_t& oStatus);
private:
    wl_status_t mStatus = WL_NO_SHIELD;
    bool mDisplayRequest = false;
    const char *mSsid;
    const char *mPassphrase;
};

inline XWiFi::XWiFi(const char *iSsid, const char *iPassphrase)
: mSsid(iSsid)
, mPassphrase(iPassphrase)
{

}

inline void XWiFi::begin()
{
    WiFi.begin(mSsid, mPassphrase);
    esp_err_t wifi_success = esp_wifi_set_ps(WIFI_PS_NONE);
    if (wifi_success == ESP_OK)
    {
        X_INFO("WiFi set to no powersaving");
    }
    else
    {
        X_INFO("Error when trying to set WiFi to no powersaving");
    }
}

inline void XWiFi::loop()
{
    wl_status_t status_new = WiFi.status();
    if (status_new != mStatus)
    {
        mStatus = status_new;
        X_DEBUG("Wifi State changed");
        mDisplayRequest = true;
    }
}

inline void XWiFi::getStatus(wl_status_t& oStatus)
{
    oStatus = mStatus;
}
