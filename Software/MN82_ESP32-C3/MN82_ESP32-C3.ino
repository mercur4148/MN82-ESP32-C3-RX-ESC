// The CAR:       25.12.24
// HW scan:       28.12.24
// PCB routing:   18.01.25
// SW:            11.03.25, 13.03.25, 14, 16, 19, 25, 28, 29
//                03.04.25, 04, 05, 06, 12, 19
//                10.05.25, 11, 12
#include <Arduino.h>
#include <Smooth.h>
// can get from Library Manager
#include "Button2.h"
// can get from Library Manager
#include <Chrono.h>
// can get from Library Manager
//#include <Preferences.h>
//Preferences ecu_data;

// settings
// lights
const uint8_t STOP_LIGHT_BRIGHTNESS = 168;      // inverted logic
// battery
const uint16_t DIVIDER_RATIO = 6545;
// motor
const uint16_t PWM_FREQ = 20000;
const uint8_t PWM_RESOLUTION = 10;
const uint16_t PWM_MAX_VALUE = (1 << PWM_RESOLUTION) - 1;
const uint16_t IDLE_TIME_THRESHOLD = 64;
const uint8_t PULSES_PER_METER = 12;        // I've got this amount of pulses for approx. 1 meter run, see below
const uint8_t CM_PER_METER_OVERSHOOT = 3;   // actually it was 103 cm for 12 pulles, so the overshoot is 3 cm
const uint16_t ODO_WRITE_INTERVAL = 8000;
// settings END

// pins definitions
const uint8_t MOT_A = 0;
const uint8_t MOT_B = 1;
const uint8_t SERVO = 2;
const uint8_t PH_A = 3;
const uint8_t BAT_DIV = 4;
const uint8_t CORN_LEFT = 5;
const uint8_t CORN_RIGHT = 6;
const uint8_t PH_B = 7;
const uint8_t HALL = 8;
const uint8_t BTN = 9;
const uint8_t STOP_ESP = 10;
const uint8_t nCL = 20;
const uint8_t nCR = 21;
// pins definitions END

// global variables
// thanks to ChatGPT
// motor-related
volatile uint32_t startTime_A = 0, startTime_B = 0, startTime_4_way = 0;
volatile uint16_t phaseA_width = 0, phaseB_width = 0, phaseA_prev = 0, phaseB_prev = 0;
volatile bool phaseA_active = 0, phaseB_active = 0;
bool braking = false;
// hall-related
uint32_t lastPulseTime = 0;
const uint32_t DEBOUNCE_TIME = 32000;     // Debounce time in microseconds

uint32_t odo_meters = 0;
uint32_t odo_centimeters = 0;
uint8_t odo_pulses = 0;

uint32_t trip_meters = 0;
uint32_t trip_centimeters = 0;
uint8_t trip_pulses = 0;

bool update_odo = 0;
// web
bool web_debug = 0;
// global variables END

Chrono chrono_4way;
Chrono chrono_blinkers;
Chrono chrono_delayed_blinkers;
Chrono chrono_time_in_idle;
//Chrono chrono_webserial;
Chrono stop_indicator_chrono;
Chrono blinkers_indicator_chrono;

Smooth average(256);
Button2 button;

// --- web debug
//#include <WiFi.h>
//#include <AsyncTCP.h>
//#include <DNSServer.h>
//#include <ESPAsyncWebServer.h>
//#include <WString.h>
//#include <MycilaWebSerial.h>
//AsyncWebServer server(80);
//WebSerial webSerial;
//
//const char* ssid = "MN82";
//const char* password = "";
//IPAddress apIP(192, 168, 4, 1);

#include "dictionary.h"
#include <ConfigAssist.h>
// can get from Library Manager
WebServer Config_server(80);
ConfigAssist conf("/MN82_config.ini", VARIABLES_DEF_YAML);
ConfigAssist odometer("/odometer.ini");
bool configassist_running = 0;

void IRAM_ATTR hallISR()
{
  uint32_t now = micros();

  if (now - lastPulseTime > DEBOUNCE_TIME)
  {
    if (digitalRead(HALL) == LOW)
    {
      trip_pulses++;
      odo_pulses++;
      lastPulseTime = now;

      // TRIP_meter
      if (trip_pulses >= PULSES_PER_METER)
      {
        trip_pulses = 0;
        trip_centimeters = trip_centimeters + CM_PER_METER_OVERSHOOT;
        if (trip_centimeters > 100)
        {
          trip_meters++;
          trip_centimeters = trip_centimeters - 100;
        }
        trip_meters++;
      }
      // TRIP_meter END

      // ODO_meter
      if (odo_pulses >= PULSES_PER_METER)
      {
        odo_pulses = 0;
        odo_centimeters = odo_centimeters + CM_PER_METER_OVERSHOOT;
        if (odo_centimeters > 100)
        {
          odo_meters++;
          odo_centimeters = odo_centimeters - 100;
        }
        odo_meters++;
      }
      // ODO_meter END
    }
  }
}

void IRAM_ATTR phaseA_ISR()
{
  bool state = digitalRead(PH_A);
  if (state == HIGH)
  {
    startTime_A = micros();
    phaseA_active = true;
  }
  else
  {
    unsigned long pulse = micros() - startTime_A;
    if (pulse > 16)
    {
      // Ignore glitches
      phaseA_width = constrain(pulse, 0, 2000);
    }
    phaseA_active = false;
  }
}

void IRAM_ATTR phaseB_ISR()
{
  bool state = digitalRead(PH_B);
  if (state == HIGH)
  {
    startTime_B = micros();
    phaseB_active = true;
  }
  else
  {
    unsigned long pulse = micros() - startTime_B;
    if (pulse > 16)
    {
      // Ignore glitches
      phaseB_width = constrain(pulse, 0, 2000);
    }
    phaseB_active = false;
  }
}
// ChatGPT END

void onDataChanged(String key)
{
  // LOG_I("Data changed: %s = %s \n", key.c_str(), conf(key).c_str());
  // if (key == "display_style") conf.setDisplayType((ConfigAssistDisplayType)conf("display_style").toInt());
}

void setup()
{
  //  Serial.begin(115200);
  //  Serial.println("MN82_ESP32-C3");
  //  Serial.print(__DATE__);
  //  Serial.print(" ");
  //  Serial.println(__TIME__);

  setCpuFrequencyMhz(40);

  // pins setup
  pinMode(MOT_A, OUTPUT);
  pinMode(MOT_B, OUTPUT);
  motor_brake();
  ledcAttach(MOT_A, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(MOT_B, PWM_FREQ, PWM_RESOLUTION);
  motor_release();

  pinMode(CORN_LEFT, OUTPUT);
  digitalWrite(CORN_LEFT, 1);
  pinMode(CORN_RIGHT, OUTPUT);
  digitalWrite(CORN_RIGHT, 1);
  // blinkers OFF

  // stop light OFF
  //  pinMode(STOP_ESP, OUTPUT);
  //  digitalWrite(STOP_ESP, 1);
  ledcAttach(STOP_ESP, 1024, 8);
  ledcWrite(STOP_ESP, 255);

  pinMode(SERVO, INPUT);
  pinMode(PH_A, INPUT);
  pinMode(PH_B, INPUT);
  attachInterrupt(digitalPinToInterrupt(PH_A), phaseA_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PH_B), phaseB_ISR, CHANGE);
  pinMode(nCL, INPUT_PULLUP);
  pinMode(nCR, INPUT_PULLUP);

  pinMode(HALL, INPUT);
  attachInterrupt(digitalPinToInterrupt(HALL), hallISR, FALLING);
  // pinMode(BTN, INPUT_PULLUP);

  button.begin(BTN);
  button.setLongClickTime(1500);
  button.setDoubleClickTime(750);

  button.setReleasedHandler(released);
  button.setClickHandler(click);
  button.setDoubleClickHandler(doubleClick);
  button.setTripleClickHandler(tripleClick);
  button.setLongClickDetectedHandler(longClickDetected);
  button.setLongClickHandler(longClick);
  button.setLongClickDetectedRetriggerable(false);

  pinMode(BAT_DIV, INPUT);
  // pins setup END

  analogReadResolution(10);
  analogSetAttenuation(ADC_6db);

  read_odo();
}

void loop()
{
  button.loop();

  motor_driver();
  blinkers();
  get_battery_voltage();

  if (configassist_running)
  {
    Config_server.handleClient();
  }

  if (conf("DISABLE_MOTOR").toInt())
  {
    static bool indicator_state = 1;

    if (!indicator_state && stop_indicator_chrono.hasPassed(500, 1))
    {
      ledcWrite(STOP_ESP, 120);
      indicator_state = 1;
    }
    else if (indicator_state && stop_indicator_chrono.hasPassed(500, 1))
    {
      ledcWrite(STOP_ESP, 255);
      indicator_state = 0;
    }
  }

  save_odo();

  // save data

  /*
    if (web_debug == 1)
    {
      if (Serial.available())
      {
        String input = Serial.readStringUntil('\n');
        webSerial.print(input);
      }

      if (chrono_webserial.hasPassed(1000))
      {
        chrono_webserial.restart();

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
        webSerial.println("------------");
      }
    }
  */
}

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
    if (update_odo == 1)
    {
      update_odo = 0;
      write_odo();
      chrono_time_in_idle.restart(512);
      chrono_time_in_idle.stop();
    }
  }
}
