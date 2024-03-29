#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <ESP8266WebServer.h>

#include "settings.h"
#include "xy6020.h"

class WebServer {
  ESP8266WebServer mWebServer;
  Xy6020 *mXy;
  Settings &mConfig;

 protected:
  void handleNotFound();
  void handleRoot();
  void handleSettings();
  void handleStyleCss();
  void handleLogicJs();
  void handleSegmentDisplayJs();
  void handleControlGet();
  void handleControlPost();
  void handleSettingsGet();
  void handleSettingsPost();

 public:
  WebServer(Xy6020 *xy_obj, Settings &config);
  void init(bool admin_mode = false);
  void task();
};

#endif  // WEBSERVER_H