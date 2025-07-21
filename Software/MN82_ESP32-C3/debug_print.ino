bool start_webserial()
{
  setCpuFrequencyMhz(160);

  WiFi.softAP("MN82_debug");
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  webSerial.onMessage([](const std::string & msg) {
    Serial.println(msg.c_str());
  });
  webSerial.begin(&Webserial_server);
  webSerial.setBuffer(255);

  Webserial_server.onNotFound([](AsyncWebServerRequest * request) {
    request->redirect("/webserial");
  });
  Webserial_server.begin();

  web_debug = 1;
  return web_debug;
}

bool stop_webserial()
{
  webSerial.println("Mischief managed");

  Webserial_server.end();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  setCpuFrequencyMhz(40);

  web_debug = 0;

  return web_debug;
}

void webprint()
{
  webSerial.print("O: ");
  webSerial.print(odo_meters);
  webSerial.print(".");
  if (odo_centimeters < 10) webSerial.print("0");
  webSerial.print(odo_centimeters);
  webSerial.print(" O_P: ");
  webSerial.println(odo_pulses);

  webSerial.print("T: ");
  webSerial.print(trip_meters);
  webSerial.print(".");
  if (trip_centimeters < 10) webSerial.print("0");
  webSerial.print(trip_centimeters);
  webSerial.print(" T_P: ");
  webSerial.println(trip_pulses);

  webSerial.print("B: ");
  webSerial.print(battery_voltage);
  webSerial.print("V & ");
  webSerial.print(battery_percentage);
  webSerial.println("%");
  webSerial.println("---");
}

void webprint_legacy()
{
  webSerial.println("ODOMETER:");
  webSerial.print(odo_meters);
  webSerial.print(".");
  if (odo_centimeters < 10) webSerial.print("0");
  webSerial.println(odo_centimeters);
  webSerial.println("TRIP:");
  webSerial.print(trip_meters);
  webSerial.print(".");
  if (trip_centimeters < 10) webSerial.print("0");
  webSerial.println(trip_centimeters);
  webSerial.print("odo_pulses ");
  webSerial.println(odo_pulses);
  webSerial.print("trip_pulses ");
  webSerial.println(trip_pulses);
  webSerial.print("BATTERY: ");
  webSerial.print(battery_voltage);
  webSerial.print("V ");
  webSerial.print(battery_percentage);
  webSerial.println("%");
  webSerial.println("------------");
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
