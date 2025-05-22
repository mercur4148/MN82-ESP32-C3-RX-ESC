const char* VARIABLES_DEF_YAML PROGMEM = R"~(
Status:
  - host_name:
      label: Wi-Fi AP name
      attribs: disabled
      default: MN82 settings
  - IP_address:
      label: Address to access settings
      attribs: disabled
      default: 192.168.4.1
  - battery_readings:
      label: Battery voltage
      attribs: disabled
      default: 0     
  - odometer_readings:
      label: Total distance traveled in meters
      attribs: disabled
      default: 0
  - odometer_pulses:
      label: 
      attribs: disabled
      default: 0
  - trip_readings:
      label: Trip distance traveled in meters
      attribs: disabled
      default: 0
  - trip_pulses:
      label: 
      attribs: disabled
      default: 0
      
Blinkers settings:
  - USE_BLINKERS:
      label: Enable or disable blinkers (but not the 4-way)
      checked: True
  - BLINK_TIME:
      label: Lenght of half the period
      range: 100, 600, 10
      default: 310  
  - BLINK_LEFT_IMMEDIATELY:
      label: Steering value to turn the LEFT blinker ON instantly
      range: 1600, 2000, 50
      default: 1900
  - BLINK_RIGHT_IMMEDIATELY:
      label: Steering value to turn the RIGHT blinker ON instantly
      range: 1000, 1400, 50
      default: 1100
  - BLINK_LEFT_DELAYED:
      label: Steering value to turn the LEFT blinker ON after a delay
      range: 1600, 2000, 50
      default: 1700
  - BLINK_RIGHT_DELAYED:
      label: Steering value to turn the RIGHT blinker ON after a delay
      range: 1000, 1400, 50
      default: 1300
  - BLINK_DELAY_MS:
      label: Delay for postponed blinking
      range: 1000, 5000, 100
      default: 3000      

Lighting settings:
  - USE_FOG:
      label: Enable or disable fog lamps
      checked: False
  - FOG_BRIGHTNESS:
      label: Fog lamps brightness
      range: 0, 255, 5
      default: 60
      
Maintenance:
  - DISABLE_MOTOR:
      label: Disable motor for safe servicing
      checked: False
  - IDLE_TIME_THRESHOLD:
      label: Time between direction transitions
      range: 5, 255, 5
      default: 65
)~";
