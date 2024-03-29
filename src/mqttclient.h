#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "settings.h"
#include "xy6020.h"

class MqttClient {
private:
  bool mConnected;
  bool mAdminMode;
  unsigned long mLastTimeStamp;

  WiFiClient mWiFiClient;
  PubSubClient mClient;
  Xy6020 &mXy;
  SettingsData &mCfg;
  void reconnect();
  void topicCallback(char *topic, byte *payload, unsigned int length);

public:
  MqttClient(Xy6020 &xy, SettingsData &config);
  void init(bool admin_mode = false);
  void task();
};
