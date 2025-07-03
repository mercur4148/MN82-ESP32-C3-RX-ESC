void onDataChanged(String key)
{
  // LOG_I("Data changed: %s = %s \n", key.c_str(), conf(key).c_str());
  // if (key == "display_style") conf.setDisplayType((ConfigAssistDisplayType)conf("display_style").toInt());
}

bool configassist_start()
{
  setCpuFrequencyMhz(80);

  conf.setDisplayType(ConfigAssistDisplayType::AccordionToggleClosed);
  Config_server.on("/", HTTP_GET, []() {
    Config_server.sendHeader("Location", "/cfg");
    Config_server.send(302, "text/plain", "");
  });

  conf.setRemotUpdateCallback(onDataChanged);
  conf.setupConfigPortal(Config_server, true);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  configassist_running = 1;

  return configassist_running;
}

bool configassist_stop()
{
  Config_server.stop();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  setCpuFrequencyMhz(40);

  configassist_running = 0;

  return configassist_running;
}
