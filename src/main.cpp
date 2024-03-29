#define DEFINE_ONCE

#define DEBUG

#include "FS.h"
#include "SPIFFS.h"
#include <Adafruit_MCP23008.h>
#include <Arduino.h>
#include <M5Unified.h>

#include "backend.h"
#include "cardreader.h"
#include "config.h"
#include "display.h"
#include "fabxdevice.h"
#include "ntp.h"
#include "trace.h"
#include "xwifi.h"

static const char *firmware_version = "1.0.3";
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
Backend sBackend(backend_host, backend_port, backend_url, firmware_version);
static CardReader sCardReader;

static Adafruit_MCP23008 sGpioOutput, sGpioInput;

static FabXDevice sFabXDevice;

void i2cscan() {
  Serial.println(" Scanning I2C Addresses");
  uint8_t cnt = 0;
  for (uint8_t i = 0; i < 128; i++) {
    Wire.beginTransmission(i);
    uint8_t ec = Wire.endTransmission(true);
    if (ec == 0) {
      if (i < 16)
        Serial.print('0');
      Serial.print(i, HEX);
      cnt++;
    } else
      Serial.print("..");
    Serial.print(' ');
    if ((i & 0x0f) == 0x0f)
      Serial.println();
  }
  Serial.print("Scan Completed, ");
  Serial.print(cnt);
  Serial.println(" I2C Devices found.");
}

void setup() {

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)

  m5::M5Unified::config_t config = M5.config();
  config.output_power = false;
  M5.begin(config);

#endif
  sDisplay.begin();
  Trace::add_logger(sSerial);
  sDisplay.begin();
  Wire.begin((int)21, (int)22);
  SPI.begin();
  i2cscan();
  sGpioOutput.begin(0);
  sGpioInput.begin(1);
  if (!SPIFFS.begin(true)) {
    X_ERROR("SPIFFS Mount Failed");
  }
  sFabXDevice.addBackend(sBackend);
  sFabXDevice.addDisplay(sDisplay);
  sFabXDevice.addNTP(sNTP);
  sFabXDevice.addWifi(sWifi);
  sFabXDevice.addReader(sCardReader);
  sFabXDevice.addOutputExpander(sGpioOutput);
  sFabXDevice.addInputExpander(sGpioInput);

  X_INFO("Trace started!");
  X_DEBUG("Firmware Version %s", firmware_version);
}

void loop() {
  sFabXDevice.loop();
  M5.update();
  ;
}
