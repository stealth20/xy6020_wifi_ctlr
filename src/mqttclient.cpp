#include "mqttclient.h"

#include <ArduinoJson.h>

#define MQTT_MAX_RECEIVE_BUFFER_SIZE 2048

MqttClient::MqttClient(Xy6020 &xy, SettingsData &config)
    : mConnected(true), mAdminMode(false), mClient(mWiFiClient), mXy(xy),
      mCfg(config) {}

void MqttClient::reconnect() {
  if (!mClient.connected()) {
    if (mConnected) {
      mConnected = false;
    }
    Serial.println("MQTT reconnecting...");
    if (!mClient.connect(mCfg.mqtt_id, mCfg.mqtt_user, mCfg.mqtt_password, NULL,
                         0, false, NULL, false)) {
      Serial.print("failed, rc=");
      Serial.print(mClient.state());
      Serial.println(" retrying in 10 seconds");
      yield();
    } else {
      mClient.subscribe(mCfg.smi_topic);
      Serial.println("MQTT connected...");
    }
  }
}

void MqttClient::topicCallback(char *topic, byte *payload,
                               unsigned int length) {
  Serial.print("Received message [");
  Serial.print(topic);
  Serial.println("] ");
  Serial.print("   :");
  String sensor_name = mCfg.sm_name;
  if (String(topic) == mCfg.smi_topic) {
    JsonDocument doc;
    deserializeJson(doc, payload);
    if (doc.containsKey(sensor_name)) {
      auto sensor = doc[sensor_name];
      if (sensor.containsKey("Power")) {
        float power_value = sensor["Power"];
        if (power_value > 0 && mCfg.zero_feed_in) {
          if (power_value > mCfg.max_power) {
            power_value = mCfg.max_power;
          }
          Serial.printf_P("Adjust power value to %0.1f\n", power_value);
          mXy.setMaxPower(power_value);
        }
      }
    }
  }
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void MqttClient::init(bool admin_mode) {
  mAdminMode = admin_mode;
  if (mAdminMode) {
    return;
  }
  mClient.setServer(IPAddress(mCfg.mqtt_broker), mCfg.mqtt_port);

  mClient.setCallback(std::bind(&MqttClient::topicCallback, this,
                                std::placeholders::_1, std::placeholders::_2,
                                std::placeholders::_3));
  mClient.setBufferSize(MQTT_MAX_RECEIVE_BUFFER_SIZE);
  mLastTimeStamp = millis() - 9000;
}

void MqttClient::task() {
  if (mAdminMode) {
    return;
  }
  if (!mClient.connected() && millis() > mLastTimeStamp + 10000) {
    mLastTimeStamp = millis();
    reconnect();
  } else {
    mClient.loop();
  }
}
