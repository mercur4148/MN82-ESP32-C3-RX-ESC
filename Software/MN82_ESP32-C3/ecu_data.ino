void read_odo()
{
  if (!odometer.confExists())
  {
    odometer["odo_m"] = odo_meters;
    odometer["odo_cm"] = odo_centimeters;
    odometer["odo_pulses"] = odo_pulses;
    odometer["trip_m"] = trip_meters;
    odometer["trip_cm"] = trip_centimeters;
    odometer["trip_pulses"] = trip_pulses;
    odometer.saveConfigFile();
  }
  else
  {
    odo_meters = odometer("odo_m").toInt();
    odo_centimeters = odometer("odo_cm").toInt();
    odo_pulses = odometer("odo_pulses").toInt();
    trip_meters = odometer("trip_m").toInt();
    trip_centimeters = odometer("trip_cm").toInt();
    trip_pulses = odometer("trip_pulses").toInt();

    conf["odometer_readings"] = (((odo_meters * 100) + odo_centimeters) / 100.0);
    conf["trip_readings"] = (((trip_meters * 100) + trip_centimeters) / 100.0);
    conf["odometer_pulses"] = odo_pulses;
    conf["trip_pulses"] = trip_pulses;
    conf["battery_readings"] = get_battery_voltage();
  }
}

void write_odo()
{
  odometer["odo_m"] = odo_meters;
  odometer["odo_cm"] = odo_centimeters;
  odometer["odo_pulses"] = odo_pulses;
  odometer["trip_m"] = trip_meters;
  odometer["trip_cm"] = trip_centimeters;
  odometer["trip_pulses"] = trip_pulses;
  odometer.saveConfigFile();

  conf["odometer_readings"] = (((odo_meters * 100) + odo_centimeters) / 100.0);
  conf["trip_readings"] = (((trip_meters * 100) + trip_centimeters) / 100.0);
  conf["odometer_pulses"] = odo_pulses;
  conf["trip_pulses"] = trip_pulses;
  conf["battery_readings"] = get_battery_voltage();

  //  Serial.println("ODOMETER:");
  //  Serial.print(odo_meters);
  //  Serial.print(".");
  //  if (odo_centimeters < 10) Serial.print("0");
  //  Serial.println(odo_centimeters);
  //  Serial.println("TRIP:");
  //  Serial.print(trip_meters);
  //  Serial.print(".");
  //  if (trip_centimeters < 10) Serial.print("0");
  //  Serial.println(trip_centimeters);
  //  Serial.print("odo_pulses ");
  //  Serial.println(odo_pulses);
  //  Serial.print("trip_pulses ");
  //  Serial.println(trip_pulses);
  //  Serial.println("------------");
}

void save_odo()
{
  if (chrono_time_in_idle.hasPassed(ODO_WRITE_INTERVAL))
  {
    if (update_odo)
    {
      update_odo = 0;
      write_odo();
      chrono_time_in_idle.restart(512);
      chrono_time_in_idle.stop();
    }
  }
}
