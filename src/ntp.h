#pragma once

#include <Arduino.h>
#include <time.h>
#include "WiFiType.h"
#include "trace.h"
#include "xwifi.h"

#define CONFIG_LWIP_SNTP_UPDATE_DELAY 15000

class NTP
{
public:
    NTP(const char *iTimezone, const char *iNTPServer, XWiFi &iWifi);
    void loop();
    bool get_redraw_request();

private:
    bool mUpdatePending = false;
    const char *mTimezone;
    const char *mServer;
    bool mSynced = false;
    unsigned long mLastCheckTime = 0;
    int mHour = -1;
    int mMin = -1;
    XWiFi *mWifi;
};

inline NTP::NTP(const char *iTimezone, const char *iNTPServer, XWiFi &iWifi)
    : mTimezone(iTimezone), mServer(iNTPServer), mWifi(&iWifi)
{
}

void NTP::loop()
{
    struct tm local;
    wl_status_t wifi_status;
    mWifi->getStatus(wifi_status);
    if (wifi_status == WL_CONNECTED && !mSynced)
    {
        X_INFO("attempting sync");
        configTzTime(mTimezone, mServer);
        mSynced = getLocalTime(&local, 5000);
        X_INFO("sync done: %d", mSynced);
        mUpdatePending = true;
    }

    unsigned long current_time = millis();

    if (mSynced && (current_time - mLastCheckTime > 1000))
    {
        mLastCheckTime = current_time;

        time_t now;
        time(&now);
        localtime_r(&now, &local);
        if (local.tm_hour != mHour)
        {
            mHour = local.tm_hour;
            mUpdatePending = true;
        }
        if (local.tm_min != mMin)
        {
            mMin = local.tm_min;
            mUpdatePending = true;
        }
    }
}

inline bool NTP::get_redraw_request()
{
    return mUpdatePending;
}