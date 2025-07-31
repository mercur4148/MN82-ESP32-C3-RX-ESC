void blinkers()
{
  // works as intended
  // relies on servo width to control cornering lamps
  // relies on both inputs to determine if it's a 4-way-flashing mode

  static bool first_run = 1;
  static bool delay_flash = 1;
  static bool is_4_way = 0;
  static bool need_sync = 0;
  static bool was_left = 0;
  static bool was_right = 0;
  static bool led_state = 1;                      // cathode driver, so 1 is OFF

  static uint16_t BLINK_ON_TIME = 313;             // based on ASIC signals
  static uint16_t BLINK_OFF_TIME = BLINK_ON_TIME;  // based on ASIC signals

  BLINK_ON_TIME = conf("BLINK_TIME").toInt();

  static uint16_t servo_width = 0;
  servo_width = pulseIn(SERVO, HIGH /*uS timeout*/);
  // maybe I'll migrate to ISR later
  // throttled_print("SERVO", servo_width, 25);

  if (configassist_running)
  {
    static bool indicator_state = 1;

    if (!indicator_state && chrono_blinkers_indicator.hasPassed(950, 1))
    {
      digitalWrite(CORN_LEFT, 0);
      digitalWrite(CORN_RIGHT, 0);
      indicator_state = 1;
    }
    else if (indicator_state && chrono_blinkers_indicator.hasPassed(50, 1))
    {
      digitalWrite(CORN_LEFT, 1);
      digitalWrite(CORN_RIGHT, 1);
      indicator_state = 0;
    }
  }
  else if (web_debug)
  {
    // ─── 2‑flash pattern: 50‑ms ON → 50‑ms OFF → 50‑ms ON → 850‑ms OFF ───
    static const uint16_t phaseTime[4]  = {50, 50, 50, 850};
    static const uint8_t  phaseLevel[4] = {0, 1, 0, 1};
    static uint8_t phase = 0;

    if (chrono_blinkers_indicator.hasPassed(phaseTime[phase]))
    {
      phase = (phase + 1) & 0x03;          // 0→1→2→3→0  (faster than % 4)
      digitalWrite(CORN_LEFT, phaseLevel[phase]);
      digitalWrite(CORN_RIGHT, phaseLevel[phase]);
      chrono_blinkers_indicator.restart();
    }
  }
  else
  {
    // ---------------------
    // check for 4-way blinking
    if ((digitalRead(nCL) == 0) && (digitalRead(nCR) == 0))
    {
      // RX wants both cornering lamps lit
      chrono_4way.restart();
      is_4_way = 1;

      if (need_sync)
      {
        // if left or right blinker was cornering, synchronize the 4-way to existing phase
        if (was_left)
        {
          led_state = digitalRead(CORN_LEFT);
        }
        else if (was_right)
        {
          led_state = digitalRead(CORN_RIGHT);
        }
        else led_state = 0;

        digitalWrite(CORN_LEFT, led_state);
        digitalWrite(CORN_RIGHT, led_state);

        need_sync = 0;
      }
    }

    //  else if ((digitalRead(nCL) == 1) && (digitalRead(nCR) == 1))
    //  {
    //    // RX wants both cornering lamps off
    //    // nothing here, we're hijacking control over lamps
    //  }

    if (is_4_way)
    {
      if (chrono_blinkers.hasPassed(BLINK_ON_TIME, 1))
      {
        // blink both lamps simultaneously

        led_state = !led_state;
        digitalWrite(CORN_LEFT, led_state);
        digitalWrite(CORN_RIGHT, led_state);
      }

      if (chrono_4way.hasPassed(/*BLINK_ON_TIME*/ 313 + 32))
      {
        // if RX doesn't send cornering light pulses (313 ms LOW + 313 ms HIGH), consider a timeout and turn 4-way OFF
        // here it's like "313 ms LOW + 32 ms timeout"
        is_4_way = 0;
        first_run = 1;
        delay_flash = 1;

        if (!was_left)
        {
          // turn left lamp OFF only if it's not in cornering state
          // e.g. if you're switching OFF the 4-way but the steering is in the left - continue blinking with respect to the existing phase
          digitalWrite(CORN_LEFT, 1);
          first_run = 0;
          delay_flash = 0;
        }

        if (!was_right)
        {
          // turn right lamp OFF only if it's not in cornering state
          digitalWrite(CORN_RIGHT, 1);
          first_run = 0;
          delay_flash = 0;
        }
      }
    }
    // ----------------------
    else if (!is_4_way)
    {
      // if it's not 4-way blinking, then sniff servo values
      if (servo_width <= 800)
      {
        was_left = was_right = 0;
        is_4_way = 1;
        // may happen on low battery, when servo is turned off by ASIC
        // check the real behaviour
      }
      if (conf("USE_BLINKERS").toInt() == 1)
      {
        //----- RIGHT CORNERING LAMP
        if (servo_width <= conf("BLINK_RIGHT_DELAYED").toInt())
        {
          was_left = 0;
          was_right = 1;

          if (first_run)
          {
            chrono_delayed_blinkers.restart();
            chrono_blinkers.add(BLINK_ON_TIME * 2);
            first_run = 0;
          }

          if (servo_width < conf("BLINK_RIGHT_IMMEDIATELY").toInt() || chrono_delayed_blinkers.hasPassed(conf("BLINK_DELAY_MS").toInt()))
          {
            chrono_delayed_blinkers.add(conf("BLINK_DELAY_MS").toInt() + BLINK_ON_TIME);
            chrono_delayed_blinkers.stop();

            if (chrono_blinkers.hasPassed(BLINK_ON_TIME, 1))
            {
              // blink the right cornering signal

              led_state = !led_state;
              digitalWrite(CORN_RIGHT, led_state);
              need_sync = 1;
            }
          }
        }
        //----- LEFT CORNERING LAMP
        else if (servo_width >= conf("BLINK_LEFT_DELAYED").toInt())
        {
          was_left = 1;
          was_right = 0;

          if (first_run)
          {
            chrono_delayed_blinkers.restart();
            chrono_blinkers.add(BLINK_ON_TIME * 2);
            first_run = 0;
          }

          if (servo_width > conf("BLINK_LEFT_IMMEDIATELY").toInt() || chrono_delayed_blinkers.hasPassed(conf("BLINK_DELAY_MS").toInt()))
          {
            chrono_delayed_blinkers.add(conf("BLINK_DELAY_MS").toInt() + BLINK_ON_TIME);
            chrono_delayed_blinkers.stop();

            if (chrono_blinkers.hasPassed(BLINK_ON_TIME, 1))
            {
              // blink the left cornering signal

              led_state = !led_state;
              digitalWrite(CORN_LEFT, led_state);
              need_sync = 1;
            }
          }
        }
        // ---------------------
        // turn the cornering lamps OFF when your steering is in the middle
        else if ((servo_width < conf("BLINK_LEFT_DELAYED").toInt()) && (servo_width > conf("BLINK_RIGHT_DELAYED").toInt()))
        {
          first_run = 1;
          delay_flash = 1;
          need_sync = 0;

          if (was_left || was_right)
          {
            digitalWrite(CORN_LEFT, 1);
            digitalWrite(CORN_RIGHT, 1);
          }
          was_left = was_right = 0;
        }
      }
    }
  }
}
