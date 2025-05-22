void released(Button2& btn)
{
  //  Serial.print("released: ");
  //  Serial.println(btn.wasPressedFor());
}

void longClickDetected(Button2& btn)
{
  // Serial.println("long click detected");
  trip_pulses = trip_meters = trip_centimeters = 0;
  chrono_time_in_idle.restart(ODO_WRITE_INTERVAL + 512);
  update_odo = 1;
}

void longClick(Button2& btn)
{
  //  Serial.println("long click");
}

void click(Button2& btn)
{
  //  Serial.println("click");
}

void doubleClick(Button2& btn)
{
  //  Serial.println("double click");
}

void tripleClick(Button2& btn)
{
  if (btn.getNumberOfClicks() == 3)
  {
    if (!configassist_running)
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
    }
    else if (configassist_running)
    {
      Config_server.stop();
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      setCpuFrequencyMhz(40);
      configassist_running = 0;
    }
  }

  else if (btn.getNumberOfClicks() == 5)
  {
    //    if (web_debug == 0) start_webserver();
    //    else stop_webserver();
  }

  else if (btn.getNumberOfClicks() == 7)
  {
    if (!conf("USE_BLINKERS").toInt()) conf["USE_BLINKERS"] = int(1);
    else if (conf("USE_BLINKERS").toInt()) conf["USE_BLINKERS"] = int(0);
    conf.saveConfigFile();
  }

  else if (btn.getNumberOfClicks() == 9)
  {
    if (!conf("DISABLE_MOTOR").toInt()) conf["DISABLE_MOTOR"] = int(1);
    else if (conf("DISABLE_MOTOR").toInt()) conf["DISABLE_MOTOR"] = int(0);
    conf.saveConfigFile();
  }
}
