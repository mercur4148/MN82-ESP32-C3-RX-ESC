// The CAR:       25.12.24
// HW scan:       28.12.24
// PCB routing:   18.01.25
// SW:            11.03.25, 13.03.25, 14, 16, 19, 25, 28, 29
//                03.04.25, 04, 05, 06, 12, 19
//                10.05.25, 11, 12, 14
// ESP32C3  Dev Module
// USB CDC on Boot: Disabled
// CPU Frequency: 40 MHz
// Partition Scheme: Minimal SPIFFS
#include <Arduino.h>
#include <Smooth.h>
// can get from Library Manager
#include "Button2.h"
// can get from Library Manager
#include <Chrono.h>
// can get from Library Manager

// settings
// lights
const uint8_t STOP_LIGHT_BRIGHTNESS = 168;      // inverted logic
// battery
const float DIVIDER_RATIO = 6.4545;
float battery_voltage = 8.40;
float battery_percentage = 100;
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

Chrono chrono_4way;
Chrono chrono_blinkers;
Chrono chrono_delayed_blinkers;
Chrono chrono_time_in_idle;
Chrono chrono_webserial(Chrono::SECONDS);
Chrono chrono_stop_indicator;
Chrono chrono_blinkers_indicator;

Smooth average(256);
Button2 button;

#include "dictionary.h"
#include <ConfigAssist.h>
// can get from Library Manager
WebServer Config_server(80);
ConfigAssist conf("/MN82_config.ini", VARIABLES_DEF_YAML);
ConfigAssist odometer("/odometer.ini");
bool configassist_running = 0;

// --- web debug
#include <WiFi.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <MycilaWebSerial.h>
AsyncWebServer Webserial_server(80);
WebSerial webSerial;
bool web_debug = 0;

void IRAM_ATTR hallISR();
void IRAM_ATTR phaseA_ISR();
void IRAM_ATTR phaseB_ISR();

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
  ledcAttach(STOP_ESP, 1024, 8);
  ledcWrite(STOP_ESP, 255);     // active LOW

  pinMode(SERVO, INPUT);
  pinMode(PH_A, INPUT);
  pinMode(PH_B, INPUT);
  attachInterrupt(digitalPinToInterrupt(PH_A), phaseA_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PH_B), phaseB_ISR, CHANGE);
  pinMode(nCL, INPUT_PULLUP);
  pinMode(nCR, INPUT_PULLUP);

  pinMode(HALL, INPUT);
  attachInterrupt(digitalPinToInterrupt(HALL), hallISR, FALLING);

  button.begin(BTN, INPUT_PULLUP, 1);
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
  motor_driver();
  blinkers();

  get_battery_voltage();
  button.loop();
  save_odo();

  if (configassist_running) Config_server.handleClient();
  if (web_debug)
  {
    /*
      if (Serial.available())
      {
      String input = Serial.readStringUntil('\n');
      webSerial.println(input);
      }
    */
    if (chrono_webserial.hasPassed(1, 1)) webprint();
  }
}
