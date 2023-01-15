#define DEFINE_ONCE

#define DEBUG

#include <Arduino.h>
#include <M5Unified.h>
#include "config.h"
#include "trace.h"
#include "xwifi.h"
#include "display.h"
#include "backend.h"
#include "ntp.h"
#include "fabxdevice.h"


static const char *ssid = WIFI_SSID;
static const char *password = WIFI_PSK;

static const char *timezone_info = TZ_INFO;
static const char *ntp_server = NTP_SERVER;

static const char *backend_host = BACKEND_HOST;
static const int backend_port = BACKEND_PORT;
static const char *backend_url = BACKEND_URL;

static SerialLogger sSerial;

static XWiFi sWifi(ssid, password);
static X5Display sDisplay;
static NTP sNTP(timezone_info, ntp_server, sWifi);
Backend sBackend(backend_host, backend_port, backend_url);

static FabXDevice sFabXDevice;

void setup()
{

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
    
    m5::M5Unified::config_t config = M5.config();
    config.output_power= false;
    M5.begin(config);
    
#endif
    sDisplay.begin();
    Trace::add_logger(sSerial);

    sFabXDevice.addBackend(sBackend);
    sFabXDevice.addDisplay(sDisplay);
    sFabXDevice.addNTP(sNTP);
    sFabXDevice.addWifi(sWifi);
    X_INFO("Trace started!");
}

void loop() //darf nicht zu lang sein weil m5 crap
{
    sFabXDevice.loop();
    M5.update();
}
