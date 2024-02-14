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
    String getMac();
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
    WiFi.setAutoReconnect(true);
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

    //reconnect if disconnected
}

inline void XWiFi::getStatus(wl_status_t& oStatus)
{
    oStatus = mStatus;
}

inline String XWiFi::getMac()
{
    byte mac[6];
    WiFi.macAddress(mac);
    char macBuffer[13];
    sprintf(macBuffer, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macBuffer);

}