#include "webserver.h"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

#include "content/result.h"
#include "settings.h"

WebServer::WebServer(Xy6020 *xy_obj, Settings &config)
    : mWebServer(80), mXy(xy_obj), mConfig(config) {}

void WebServer::init(bool admin_mode) {
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  mWebServer.on("/style.css", std::bind(&WebServer::handleStyleCss, this));
  if (admin_mode) {
    mWebServer.on("/", std::bind(&WebServer::handleSettings, this));
  } else {
    mWebServer.on("/", std::bind(&WebServer::handleRoot, this));
  }
  mWebServer.on("/settings.html", std::bind(&WebServer::handleSettings, this));
  mWebServer.on("/logic.js", std::bind(&WebServer::handleLogicJs, this));
  mWebServer.on("/segment-display.js",
                std::bind(&WebServer::handleSegmentDisplayJs, this));
  mWebServer.onNotFound(std::bind(&WebServer::handleNotFound, this));
  mWebServer.on("/control", HTTP_GET,
                std::bind(&WebServer::handleControlGet, this));
  mWebServer.on("/control", HTTP_POST,
                std::bind(&WebServer::handleControlPost, this));
  mWebServer.on("/config", HTTP_GET,
                std::bind(&WebServer::handleSettingsGet, this));
  mWebServer.on("/config", HTTP_POST,
                std::bind(&WebServer::handleSettingsPost, this));
  mWebServer.begin();
  Serial.println("HTTP server started");
}

void WebServer::task() { mWebServer.handleClient(); }

void WebServer::handleNotFound() {
  Serial.println("send not found");

  mWebServer.send(404, "text/plain", "Not found");
}

void WebServer::handleRoot() { mWebServer.send(200, "text/html", html__index); }

void WebServer::handleSettings() {
  mWebServer.send(200, "text/html", html__settings);
}

void WebServer::handleStyleCss() {
  mWebServer.send(200, "text/css", css__style);
}

void WebServer::handleLogicJs() {
  mWebServer.send(200, "text/javascript", js__logic);
}

void WebServer::handleSegmentDisplayJs() {
  mWebServer.send(200, "text/javascript", js__segmentdisplay);
}

void WebServer::handleSettingsGet() {
  String str;
  auto &cfg = mConfig.data();

  str += "{\"ssid\" : \"" + String(cfg.wifi_ssid) + "\"," +
         "\"use-static-ip\": " + cfg.use_static_ip + ",\"static-ip\":\"" +
         IPAddress(cfg.static_ip).toString() + "\",\"subnet\": \"" +
         IPAddress(cfg.subnet).toString() + "\",\"gateway\": \"" +
         IPAddress(cfg.gateway).toString() + "\",\"mqtt-server\": \"" +
         IPAddress(cfg.mqtt_broker).toString() + "\",\"mqtt-port\": \"" +
         cfg.mqtt_port + "\",\"mqtt-user\": \"" + cfg.mqtt_user +
         "\", \"mqtt-id\": \"" + cfg.mqtt_id +
         "\",\"zero-feed-in\": " + cfg.zero_feed_in + ",\"smi-topic\": \"" +
         cfg.smi_topic + "\",\"sm-name\": \"" + cfg.sm_name +
         "\", \"enable-input-limits\":" + cfg.enable_input_limits +
         ", \"switch-off-voltage\": \"" + cfg.switch_off_voltage +
         "\", \"switch-on-voltage\": \"" + cfg.switch_on_voltage + "\"}";

  mWebServer.sendHeader("Access-Control-Allow-Origin", "*");
  mWebServer.send(200, "application/json", str);
}

void WebServer::handleSettingsPost() {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, mWebServer.arg("plain"));
  Serial.println(mWebServer.arg("plain"));

  auto &cfg = mConfig.data();
  if (err == DeserializationError::Ok) {
    if (!doc["ssid"].isUnbound()) {
      String str = doc["ssid"];
      strncpy(cfg.wifi_ssid, str.c_str(), 128);
    }
    if (!doc["wifi-password"].isUnbound()) {
      String str = doc["wifi-password"];
      if (str.length()) {
        strncpy(cfg.wifi_password, str.c_str(), 128);
      }
    }

    cfg.use_static_ip = doc["use-static-ip"] | cfg.use_static_ip;

    if (!doc["static-ip"].isUnbound()) {
      String str = doc["static-ip"];
      IPAddress addr;
      if (addr.fromString(str)) {
        cfg.static_ip = addr;
      }
    }

    if (!doc["subnet"].isUnbound()) {
      String str = doc["subnet"];
      IPAddress addr;
      if (addr.fromString(str)) {
        cfg.subnet = addr;
      }
    }

    if (!doc["gateway"].isUnbound()) {
      String str = doc["gateway"];
      IPAddress addr;
      if (addr.fromString(str)) {
        cfg.gateway = addr;
      }
    }

    if (!doc["mqtt-server"].isUnbound()) {
      String str = doc["mqtt-server"];
      IPAddress addr;
      if (addr.fromString(str)) {
        cfg.mqtt_broker = addr;
      }
    }

    cfg.mqtt_port = doc["mqtt-port"] | cfg.mqtt_port;

    if (!doc["mqtt-user"].isUnbound()) {
      String str = doc["mqtt-user"];
      strncpy(cfg.mqtt_user, str.c_str(), 128);
    }

    if (!doc["mqtt-pass"].isUnbound()) {
      String str = doc["mqtt-pass"];
      if (str.length()) {
        strncpy(cfg.mqtt_password, str.c_str(), 128);
      }
    }

    if (!doc["mqtt-id"].isUnbound()) {
      String str = doc["mqtt-id"];
      strncpy(cfg.mqtt_id, str.c_str(), 128);
    }

    cfg.zero_feed_in = doc["zero-feed-in"] | cfg.zero_feed_in;

    if (!doc["smi-topic"].isUnbound()) {
      String str = doc["smi-topic"];
      strncpy(cfg.smi_topic, str.c_str(), 128);
    }
    if (!doc["sm-name"].isUnbound()) {
      String str = doc["sm-name"];
      strncpy(cfg.sm_name, str.c_str(), 128);
    }

    cfg.enable_input_limits =
        doc["enable-input-limits"] | cfg.enable_input_limits;

    if (!doc["switch-off-voltage"].isUnbound()) {
      String str = doc["switch-off-voltage"];
      cfg.switch_off_voltage = str.toFloat();
    }
    if (!doc["switch-on-voltage"].isUnbound()) {
      String str = doc["switch-on-voltage"];
      cfg.switch_on_voltage = str.toFloat();
    }

    mConfig.store();
  }

  mWebServer.sendHeader("Access-Control-Allow-Origin", "*");
  mWebServer.send(200, "text/html", "OK");
}

void WebServer::handleControlGet() {
  char str[256];
  sprintf(str,
          "{"
          "\"voltage\": %0.2f,"
          "\"current\": %0.2f,"
          "\"power\": %0.1f,"
          "\"output\": %d,"
          "\"tvoltage\": %0.2f,"
          "\"tcurrent\": %0.2f,"
          "\"tpower\": %0.1f,"
          "\"ivoltage\": %0.1f,"
          "\"connected\": %d"
          "}",
          mXy->actualVoltage(), mXy->actualCurrent(), mXy->actualPower(),
          mXy->outputEnabled(), mXy->targetVoltage(), mXy->maxCurrent(),
          mXy->maxPower(), mXy->inputVoltage(), mXy->isConnected());
  mWebServer.sendHeader("Access-Control-Allow-Origin", "*");
  mWebServer.send(200, "application/json", str);
}

void WebServer::handleControlPost() {
  Serial.println("Control set request received.");
  mWebServer.sendHeader("Access-Control-Allow-Origin", "*");
  for (int a = 0; a < mWebServer.args(); ++a) {
    auto param = mWebServer.argName(a);
    auto val_str = mWebServer.arg(a);
    float value = val_str.toFloat();
    Serial.printf_P("%s=%0.2f\n", param.c_str(), value);
    bool ret = false;
    if (param == "voltage") {
      ret = mXy->setTargetVoltage(value);
    } else if (param == "current") {
      ret = mXy->setMaxCurrent(value);
    } else if (param == "max-power") {
      mXy->setMaxPower(value);
      mConfig.store();
      ret = true;
    } else if (param == "output") {
      if (val_str.length()) {
        mXy->setOutputEnabled(value > 0.01);
        ret = true;
      }
    } else if (param == "reset") {
      ESP.reset();
    }
    if (!ret) {
      mWebServer.send(200, "text/html", "FAIL");
      return;
    }
  }
  mWebServer.send(200, "text/html", "OK");
}