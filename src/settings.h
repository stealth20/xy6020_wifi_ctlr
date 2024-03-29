#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

#define CONFIG_VERSION 1

struct SettingsData {
  uint8_t version;

  char wifi_ssid[128];
  char wifi_password[128];

  bool use_static_ip;
  uint32_t static_ip;
  uint32_t subnet;
  uint32_t gateway;

  uint32_t mqtt_broker;
  uint16_t mqtt_port;
  char mqtt_user[128];
  char mqtt_password[128];
  char mqtt_id[128];

  bool zero_feed_in;
  char smi_topic[128];
  char sm_name[128];
  bool enable_input_limits;
  float switch_off_voltage;
  float switch_on_voltage;
  float max_power;
};

class Settings {
  SettingsData mData;
  void setDefaults();

public:
  Settings();
  SettingsData &data() { return mData; }
  void load();
  void store();
  String ipToString(uint32_t ip);
};

#endif // SETTINGS_H