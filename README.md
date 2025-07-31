This is a rework of a stock RX + ESC based on ESP32-C3 SuperMini module. It uses some parts from the original ESC board so get your hot air soldering station and soldering iron ready. See reddit for more details:
https://www.reddit.com/r/mn82/comments/1jf5e2q/custommade_rxesc_stock_rework/
https://www.reddit.com/r/mn82/comments/1ks0tar/mn82_with_a_brushed_motor_but_custommade_esc/

# Features implemented:
1. No more 500 Hz whine from the motor - it's driven by nice 20 kHz waveforms.
2. Blinkers are redesigned to start blinking at predefined steering values, so slight turns can be done without blinking. However, when steering is near the edges, lamps do start blinking; also, staying in leftish or rightish side for too long starts blinking too. See and adjust for yourself.
3. Added braking when swiftly changing throttle from REVERSE to FORWARD. Stock ESC has braking logic only for fast transition from FORWARD to REVERSE.
4. Since it's ESP32-C3, there's a Wi-Fi Access Point can be created to adjust some settings, see saved data and make OTA updates (no need to disassemble the car to make some updates - if you've added pushbutton first; otherwise, use ESP32's BOOT button).
5. If one installs a magnet into the drivetrain and a Hall sensor next to it, the odometer can be implemented. The sensor can be connected to GPIO8 (please provide an appropriate pullup and 100 nF filtering by yourself). In my case, I've placed 2 pcs 2*1 mm magnet into one of the gears in the gearbox next to the sensor; there are 12 pulses per 103 cm of run. See the code for implementation.
6. A button with an active LOW signal can be soldered to GPIO9 of ESP (I use the TTP223 touch module).
    - 1 long click clears the "trip meter";
    - 3 fast clicks toggle Wi-Fi Access Point (connect to "MN82 settings" and open 192.168.4.1 in browser) - blinkers will blink shortly to indicate the ON state. Please turn it off when everything's done by either clicking the button 3 times once again or rebooting the board;
    - 5 fast clicks create WebSerial debug Wi-Fi Access point. Search for "MN82_debug" and open 192.168.4.1 in browser. It's useful for those wanting to adjust an odometer or see battery voltage;
    - 7 fast clicks toggle blinkers functionality (4-way blinkers aren't affected);
    - 9 fast clicks toggle motor output (it's a good idea to disable the motor when tinkering with a car which stands on a table) - STOP lamps blink to indicate MOTOR_OFF state.
7. Added a power MOSFET so the switch on the bottom of the car doesn't have to carry large currents.
8. Got rid of LM78M05 and made space for AP63205 step-down chip.
9. Added NXW-02 connector for external voltmeter to place somewhere in the car.

# PCB ordering
The PCB is 8-layer stackup. I've provided Gerbers, NC drill and a reference file to order from JLCPCB. If done right, you'll get 5 PCBs for $2 excluding freight.

# PCB assembly
The Bill of Materials contains nearly all the data needed to assemble the board. I've marked parts that can be scavenged from the original PCB. Use the Assembly Drawings both with Schematics.

# Software
I've used Arduino IDE 1.8.13 with an esp32 core 3.2.0.
The libraries needed are placed at the start of the sketch with a places where to get them. Note that "ConfigAssist" library should be retrieved from github as there are some vital changes not in release yet: https://github.com/gemi254/ConfigAssist-ESP32-ESP8266

---
Designed in Ukraine while listening to russian drone attacks and ballistic missiles incoming.

Last update 31 July 2025
