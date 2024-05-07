#include <Arduino.h>

#define ACCESS_POINT_NAME "XY6020_Config"

#define EEPROM_SIZE 4096
#define ESP_MRD_USE_EEPROM true

#define MRD_TIMES 5
#define MRD_TIMEOUT 5
#define MRD_ADDRESS 4000

#define XY_RX_PIN 4
#define XY_TX_PIN 5

//#define USE_BUTTON_CTRL

#include <ESP_MultiResetDetector.h>

#include "buttonctrl.h"
#include "mqttclient.h"
#include "settings.h"
#include "webserver.h"
#include "xy6020.h"

// Global declarations
unsigned long ts;
Settings settings;
auto &cfg = settings.data();

Xy6020 xy(cfg, XY_RX_PIN, XY_TX_PIN);
WebServer webserver(&xy, settings);
MqttClient mqtt(xy, cfg);
MultiResetDetector mrd(MRD_TIMEOUT, MRD_ADDRESS);
#ifdef USE_BUTTON_CTRL
ButtonCtrl buttons(xy);
#endif

void setup() {
  Serial.begin(115200);
  settings.load();

  bool admin_mode = false;
  auto mr_detected = mrd.detectMultiReset();
  if (String(cfg.wifi_ssid) == "MY-SSID" || mr_detected) {
    Serial.println("\n\nADMIN MODE!\n");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ACCESS_POINT_NAME, NULL);
    admin_mode = true;
  } else {
    WiFi.begin(cfg.wifi_ssid, cfg.wifi_password);
    ts = millis();
    while (WiFi.status() != WL_CONNECTED) {
      mrd.loop();
      if (millis() > ts + 1000) {
        ts = millis();
        Serial.print(".");
      }
      yield();
    }
  }

  webserver.init(admin_mode);
  mqtt.init(admin_mode);
#ifdef USE_BUTTON_CTRL
  buttons.init();
#endif
  ts = millis();
}

void loop() {
  webserver.task();
  mqtt.task();
  mrd.loop();
  xy.task();
#ifdef USE_BUTTON_CTRL
  buttons.task();
#endif
}
