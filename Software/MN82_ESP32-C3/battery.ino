uint32_t get_battery_voltage()
{
  // works as intended
  // needs DIVIDER_RATIO adjustment
  static uint16_t divider_voltage = 0;
  static uint16_t filtered_div = 0;
  static uint32_t battery_voltage = 0;
  
  divider_voltage = analogReadMilliVolts(BAT_DIV);
  
  (uint16_t)average.add(divider_voltage);
  filtered_div = (int16_t)average.get_avg();

  battery_voltage = ((filtered_div * DIVIDER_RATIO) / 1000);
  // throttled_print("BAT MV", battery_voltage, 25);

  return battery_voltage;
}
