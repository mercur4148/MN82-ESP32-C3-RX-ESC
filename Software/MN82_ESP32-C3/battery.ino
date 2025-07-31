float get_battery_voltage()
{
  // works as intended
  // still needs DIVIDER_RATIO adjustment
  static uint16_t divider_voltage = 0;
  static uint16_t filtered_div = 0;

  divider_voltage = analogReadMilliVolts(BAT_DIV);

  (uint16_t)average.add(divider_voltage);
  filtered_div = (int16_t)average.get_avg();

  battery_voltage = filtered_div * DIVIDER_RATIO / 1000;

  if      (battery_voltage >= 8.4) battery_percentage = 100;
  else if (battery_voltage >= 8.2) battery_percentage = 90;
  else if (battery_voltage >= 8.0) battery_percentage = 80;
  else if (battery_voltage >= 7.8) battery_percentage = 70;
  else if (battery_voltage >= 7.6) battery_percentage = 60;
  else if (battery_voltage >= 7.4) battery_percentage = 50;
  else if (battery_voltage >= 7.2) battery_percentage = 30;
  else if (battery_voltage >= 7.0) battery_percentage = 15;
  else if (battery_voltage >= 6.8) battery_percentage = 5;
  else                             battery_percentage = 0;

  // throttled_print("BAT MV", battery_voltage, 25);

  return battery_voltage;
}
