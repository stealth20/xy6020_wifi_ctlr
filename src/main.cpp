#include <Arduino.h>

#define ACCESS_POINT_NAME "XY6020_Config"

#define EEPROM_SIZE 4096
#define ESP_MRD_USE_EEPROM true

#define MRD_TIMES 5
#define MRD_TIMEOUT 5
#define MRD_ADDRESS 4000

#define XY_RX_PIN 4
#define XY_TX_PIN 5

#define vplus_BTN 12
#define vminus_BTN 13
#define iplus_BTN 14
#define iminus_BTN 16
#define btn_mV_step 100 //100mV for each click
#define btn_mA_step 100 //100mA for each click
//probably need to do somenting like: if(actualcurrent<(5*btn_mA_step) decrease the current by 50mA or something like that,
//probably need to implement long press of the button.

bool oldState_vplus = HIGH; //variables for btn debounce
bool oldState_vminus= HIGH;
bool oldState_iplus = HIGH;
bool oldState_iminus = HIGH;

#include <ESP_MultiResetDetector.h>

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

void setup() {
  Serial.begin(115200);
  settings.load();

  pinMode(vplus_BTN, INPUT_PULLUP);
  pinMode(vminus_BTN, INPUT_PULLUP);
  pinMode(iplus_BTN, INPUT_PULLUP);
  pinMode(iminus_BTN, INPUT_PULLUP);

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

  ts = millis();
}



void buttonsManager(){
  bool newState_vplus = digitalRead(vplus_BTN); // Get current button state.
  bool newState_vminus = digitalRead(vminus_BTN);
  bool newState_iplus = digitalRead(iplus_BTN);
  bool newState_iminus = digitalRead(iminus_BTN);



  if (newState_vplus == LOW && oldState_vplus == HIGH) {  // Check if state changed from high to low (button press).
    delay(20);// Short delay to debounce button.
    newState_vplus = digitalRead(vplus_BTN);// Check if button is still low after debounce.
    if (newState_vplus == LOW) {
    // code executed when buttton is pressed
    xy.setTargetVoltage( xy.actualVoltage() + (btn_mV_step/1000) );
    // end of code executed when buttton is pressed
    }
  }

  if (newState_vminus == LOW && oldState_vminus == HIGH) {  // Check if state changed from high to low (button press).
    delay(20);// Short delay to debounce button.
    newState_vminus = digitalRead(vminus_BTN);// Check if button is still low after debounce.
    if (newState_vminus == LOW) {
    // code executed when buttton is pressed
    xy.setTargetVoltage( xy.actualVoltage() - (btn_mV_step/1000) );
    // end of code executed when buttton is pressed
    }
  }

  if (newState_iplus == LOW && oldState_iplus == HIGH) {  // Check if state changed from high to low (button press).
    delay(20);// Short delay to debounce button.
    newState_iplus = digitalRead(iplus_BTN);// Check if button is still low after debounce.
    if (newState_iplus == LOW) {
    // code executed when buttton is pressed
    xy.setMaxCurrent( xy.actualCurrent() + (btn_mA_step/1000) );
    // end of code executed when buttton is pressed
    }
  }

  if (newState_iminus == LOW && oldState_iminus == HIGH) {  // Check if state changed from high to low (button press).
    delay(20);// Short delay to debounce button.
    newState_iminus = digitalRead(iminus_BTN);// Check if button is still low after debounce.
    if (newState_iminus == LOW) {
    // code executed when buttton is pressed
    xy.setMaxCurrent( xy.actualCurrent() - (btn_mA_step/1000) );
    // end of code executed when buttton is pressed
    }
  }

  oldState_vplus = newState_vplus; // Set the last button state to the old state.
  oldState_vminus = newState_vminus;
  oldState_iplus = newState_iplus;
  oldState_iminus = newState_iminus;
}



void loop() {
  webserver.task();
  mqtt.task();
  mrd.loop();
  buttonsManager();
  xy.task();
}
