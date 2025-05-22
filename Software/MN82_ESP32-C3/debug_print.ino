void start_webserver()
{
  /*
    WiFi.setHostname("ssid");
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssid);
    Serial.print("1. Connect to Wi-Fi ");
    Serial.println(ssid);
    Serial.println("2. Open \"192.168.4.1/webserial\" in browser");
    webSerial.onMessage([](const std::string & msg) {
      Serial.println(msg.c_str());
    });
    server.onNotFound([](AsyncWebServerRequest * request) {
      request->redirect("/webserial");
    });
    webSerial.begin(&server);
    server.begin();
  */

  web_debug = 1;
}

void stop_webserver()
{
  /*
    server.end();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("Mischief managed");

    // IDK why, but server is broken after restarting
    // WebSerial.println("something") doesn't show in remote console
    // remote to ESP messages still work fine \_(O_o)_/
  */

  web_debug = 0;
}

void throttled_print(String label, uint16_t value, uint16_t throttle_rate)
{
  // works as intended
  // allows to push big chunks of data over Serial without freezing the PC's terminal
  static uint8_t throttled_value = 0;

  if (throttled_value > throttle_rate)
  {
    throttled_value = 0;

    Serial.print(label);
    Serial.print(": ");
    Serial.print(value);
    Serial.println();
  }
  throttled_value++;
}
