#include "settings.h"

#include <EEPROM.h>
#include <ESP8266WiFi.h>

#ifndef EEPROM_SIZE
#define EEPROM_SIZE 4096
#endif

Settings::Settings() { EEPROM.begin(EEPROM_SIZE); }

void Settings::load() {
  EEPROM.get(0, mData);
  if (mData.version != CONFIG_VERSION) {
    Serial.println("\nNo config detected -> Set Defaults");
    setDefaults();
  }
}

void Settings::store() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(0, mData);
  EEPROM.end();
  EEPROM.begin(EEPROM_SIZE);
}

void Settings::setDefaults() {
  mData.version = CONFIG_VERSION;
  sprintf(mData.wifi_ssid, "MY-SSID");
  mData.wifi_password[0] = '\0';

  mData.use_static_ip = false;
  mData.static_ip = 0;
  mData.subnet = 0xffffff00;
  mData.gateway = 0;

  mData.mqtt_broker = 0;
  mData.mqtt_port = 1883;
  mData.mqtt_user[0] = '\0';
  mData.mqtt_password[0] = '\0';

  uint8_t mac[6];
  WiFi.macAddress(mac);
  sprintf(mData.mqtt_id, "xy6020_%02X%02X%02X%02X%02X%02X", mac[0], mac[1],
          mac[2], mac[3], mac[4], mac[5]);

  mData.zero_feed_in = false;
  sprintf(mData.smi_topic, "tele/tasmota_smi_device/SENSOR");
  sprintf(mData.sm_name, "SmartMeter_1st_FLOOR");

  mData.enable_input_limits = false;
  mData.switch_off_voltage = 0;
  mData.switch_on_voltage = 0.1;

  mData.max_power = 1200;
}

String Settings::ipToString(uint32_t ip) {
  char str[16];
  sprintf(str, "%d.%d.%d.%d", 0xff & (ip >> 24), 0xff & (ip >> 16),
          0xff & (ip >> 8), 0xff & (ip >> 0));
  return String(str);
}
