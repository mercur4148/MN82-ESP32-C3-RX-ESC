void IRAM_ATTR hallISR()
{
  uint32_t now = micros();

  if (now - lastPulseTime > DEBOUNCE_TIME)
  {
    if (!digitalRead(HALL))
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
  if (state)
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
  if (state)
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

void updateDirection()
{
  static int8_t preprevDir = 0;  // Stores the direction from two cycles ago
  static int8_t prevDir = 0;     // Stores the previous direction
  static int8_t dir = 0;         // Stores the current direction

  // Auto braking (slow mode or ASIC-based function)
  if (phaseA_width == 2000 && phaseB_width == 2000)
  {
    // dir = 99;  // Braking
  }
  else if (phaseA_width > 0 && phaseB_width == 2000)
  {
    dir = 1;  // Forward
  }
  else if (phaseB_width > 0 && phaseA_width == 2000)
  {
    dir = -1; // Reverse
  }
  // else if (phaseA_width == 0 && phaseB_width == 0)
  else if (phaseA_width == phaseB_width && (phaseA_width == 0 || phaseA_width == 2000))
  {
    dir = 0;  // Idle
  }

  if (dir != prevDir /* && dir != 99 */)
  {
    // Print direction changes for debugging
    //      Serial.print("PPD: ");
    //      Serial.print(preprevDir);
    //      Serial.print(" PD: ");
    //      Serial.print(prevDir);
    //      Serial.print(" CD: ");
    //      Serial.println(dir);

    if (dir == 0 && abs(prevDir) == 1)
    {
      chrono_time_in_idle.restart();
      if (braking) braking = 0;
      update_odo = 1;
    }

    else if (abs(dir) == 1 && prevDir == 0)
    {
      chrono_time_in_idle.stop();
      update_odo = 0;
    }

    if ((preprevDir == 1 && prevDir == 0 && dir == -1) || (preprevDir == -1 && prevDir == 0 && dir == 1))
    {
      if (!chrono_time_in_idle.hasPassed(conf("IDLE_TIME_THRESHOLD").toInt()))
      {
        braking = 1;
      }
      else
      {
        braking = 0;
      }
    }
    preprevDir = prevDir;
    prevDir = dir;
  }
}

bool isThrottleIdle()
{
  // not used right now, but works OK
  return (phaseA_width == phaseB_width) && (phaseA_width == 0 || phaseA_width == 2000);
}

void motor_driver()
{
  // works like charm
  uint32_t timeSinceA = micros() - startTime_A;
  uint32_t timeSinceB = micros() - startTime_B;

  // If phase stays LOW for too long, reset to 0
  if (!phaseA_active && timeSinceA > 2100) phaseA_width = 0;
  if (!phaseB_active && timeSinceB > 2100) phaseB_width = 0;

  // If phase stays HIGH for too long, assume 2000
  if (phaseA_active && timeSinceA > 2100) phaseA_width = 2000;
  if (phaseB_active && timeSinceB > 2100) phaseB_width = 2000;

  updateDirection();

  if (!braking)
  {
    uint16_t dutyA = map(phaseA_width, 0, 2000, 0, PWM_MAX_VALUE);
    uint16_t dutyB = map(phaseB_width, 0, 2000, 0, PWM_MAX_VALUE);

    if (!conf("DISABLE_MOTOR").toInt())
    {
      ledcWrite(MOT_A, dutyA);
      ledcWrite(MOT_B, dutyB);
      ledcWrite(STOP_ESP, 255);
    }
    else
    {
      ledcWrite(MOT_A, 0);
      ledcWrite(MOT_B, 0);

      static bool indicator_state = 1;

      if (!indicator_state && chrono_stop_indicator.hasPassed(500, 1))
      {
        ledcWrite(STOP_ESP, 120);
        indicator_state = 1;
      }
      else if (indicator_state && chrono_stop_indicator.hasPassed(500, 1))
      {
        ledcWrite(STOP_ESP, 255);
        indicator_state = 0;
      }
    }
  }

  else if (braking)
  {
    ledcWrite(MOT_A, PWM_MAX_VALUE);
    ledcWrite(MOT_B, PWM_MAX_VALUE);
    ledcWrite(STOP_ESP, STOP_LIGHT_BRIGHTNESS);
  }

  // debug output
  //  static unsigned long lastPrintTime = 0;
  //  if (millis() - lastPrintTime >= 10)
  //  {
  //    // Print every 10ms
  //    lastPrintTime = now;
  //    Serial.println("INCOMING:");
  //    Serial.print("PH_A: ");
  //    Serial.print(phaseA_width);
  //    Serial.print(" us | PH_B: ");
  //    Serial.print(phaseB_width);
  //    Serial.println(" us");
  //    Serial.println("GENERATING:");
  //    Serial.print("MOT_A: ");
  //    Serial.print(dutyA);
  //    Serial.print(" | MOT_B: ");
  //    Serial.print(dutyB);
  //    Serial.print(" of ");
  //    Serial.println(PWM_MAX_VALUE);
  //    Serial.println("-----");
  //  }
}

void motor_brake()
{
  // works as intended
  // motor locked
  digitalWrite(MOT_A, 1);
  digitalWrite(MOT_B, 1);
  ledcWrite(STOP_ESP, STOP_LIGHT_BRIGHTNESS);
}

void motor_release()
{
  // works as intended
  // motor unlocked
  digitalWrite(MOT_A, 0);
  digitalWrite(MOT_B, 0);
  ledcWrite(STOP_ESP, 255);
}
