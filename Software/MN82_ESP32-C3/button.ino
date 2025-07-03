void released(Button2& btn)
{
  //  Serial.print("released: ");
  //  Serial.println(btn.wasPressedFor());
}

void longClickDetected(Button2& btn)
{
  // reset trip meters
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
  // change display modes
}

void doubleClick(Button2& btn)
{
  //  Serial.println("double click");
}

void tripleClick(Button2& btn)
{
  if (btn.getNumberOfClicks() == 3)
  {
    if (!configassist_running && !web_debug) configassist_start();
    else if (configassist_running && !web_debug) configassist_stop();
  }

  else if (btn.getNumberOfClicks() == 5)
  {
    if (!web_debug && !configassist_running) start_webserial();
    else if (web_debug && !configassist_running) stop_webserial();
  }

  else if (btn.getNumberOfClicks() == 6)
  {
    // toggle fog lamps usage
    // not possible in HW 1
  }

  else if (btn.getNumberOfClicks() == 7)
  {
    // toggle blinkers usage
    if (!conf("USE_BLINKERS").toInt()) conf["USE_BLINKERS"] = int(1);
    else if (conf("USE_BLINKERS").toInt()) conf["USE_BLINKERS"] = int(0);
    conf.saveConfigFile();
  }

  else if (btn.getNumberOfClicks() == 9)
  {
    // disable motor
    if (!conf("DISABLE_MOTOR").toInt()) conf["DISABLE_MOTOR"] = int(1);
    else if (conf("DISABLE_MOTOR").toInt()) conf["DISABLE_MOTOR"] = int(0);
    conf.saveConfigFile();
  }
}
