# Urinary Bag Full Detection System

An ESP32-based prototype that monitors the estimated fill level of a urinary drainage bag using a load cell and HX711 amplifier. It provides local LED/buzzer alerts and a Blynk IoT dashboard with a push notification at the critical level.

## Features

- Measures the load of the drainage bag using a load cell and HX711.
- Estimates fluid volume using the approximation **1 g ≈ 1 mL**.
- Displays estimated volume and fill percentage in the Blynk dashboard.
- Provides status messages for normal, half-full, near-full, and critical levels.
- Activates an LED and buzzer at warning/critical levels.
- Sends a Blynk push notification when the critical threshold is reached.
- Supports remote tare and alarm reset controls, plus Serial Monitor commands.

## Hardware

- ESP32 development board
- Load cell suitable for the intended bag/load range
- HX711 load-cell amplifier module
- Buzzer
- LED and suitable current-limiting resistor
- Jumper wires and stable mechanical mounting for the load cell
- Wi-Fi connection for Blynk IoT features

## Wiring

| Component | ESP32 pin |
|---|---:|
| HX711 DOUT / DT | GPIO 16 |
| HX711 SCK / CLK | GPIO 4 |
| Buzzer signal | GPIO 18 |
| LED signal | GPIO 19 |
| HX711 VCC and GND | Use module-compatible power and common ground |

Connect the load cell to the HX711 terminals according to the labels and wiring diagram supplied with your load cell. Load-cell wire colors are not universal. Check the voltage requirements of your particular HX711 module and ESP32 board before wiring.

## Software requirements

- Arduino IDE
- ESP32 board support package
- `Blynk` library compatible with ESP32
- `HX711` library
- A Blynk IoT account and device template

## Blynk setup

Create a Blynk template/device and replace the placeholder template ID, template name, authentication token, Wi-Fi SSID, and password in the sketch.

Configure these datastreams (virtual pins):

| Virtual pin | Purpose | Suggested data type |
|---|---|---|
| `V0` | Estimated volume in mL | Double |
| `V1` | Current status / sensor error | String |
| `V2` | Fill percentage | Double |
| `V3` | Remote tare button | Integer; button mode recommended |
| `V4` | Remote alarm reset/mute | Integer; button mode recommended |
| `V5` | Critical alarm indicator | Integer |

Create a Blynk **Event** with the event code `bag_full` and enable push notifications for that event. The event code must match the code used by `Blynk.logEvent()` in the sketch.

## Fill thresholds

The sketch uses a nominal capacity of 2,000 mL and these thresholds:

| Level | Threshold | Behavior |
|---|---:|---|
| Half full | 1,000 mL (50%) | Dashboard status update |
| Near full | 1,500 mL (75%) | Warning status, LED on, short buzzer tone |
| Critical | 1,800 mL (90%) | Critical status, LED on, continuous buzzer, one push notification per event |

Adjust these constants to match your prototype's intended bag capacity and validated thresholds. These are example settings, not clinical recommendations.

## Calibration

The example `calibrationFactor` value is only a starting point and will usually need to be calibrated for the actual load cell, mounting arrangement, and HX711 module.

1. Upload the sketch and open Serial Monitor at **115200 baud**.
2. Ensure the bag/support is in the intended empty reference state.
3. Use the tare function to establish the zero reference.
4. Calibrate the scale using a known weight and the procedure recommended by your installed HX711 library.
5. Update `calibrationFactor` and verify readings across the expected load range.
6. Check the volume estimate against measured amounts. The `1 g ≈ 1 mL` approximation assumes a fluid density close to water and does not account for bag mass unless the setup is tared appropriately.

## Serial commands

Send one character through the Serial Monitor:

- `t` or `T`: tare the scale and clear the notification latch.
- `r` or `R`: reset/mute the local alarm.

## Important behavior notes

- The sensor and dashboard are polled every 1.5 seconds.
- The critical push notification is sent once while the measured volume remains in the critical event; the notification latch is cleared when the estimated volume falls below the half-full threshold or when tare is used.
- The alarm reset command mutes the local buzzer/LED temporarily, but if the measured volume remains critical, the next sensor update can activate the critical alarm again.
- A tare operation resets the scale reference; use it only when the bag/support is in the intended empty reference condition. Taring a bag while it contains fluid can make subsequent estimates inaccurate.
- Wi-Fi/Blynk connectivity is required for remote dashboard updates and push notifications. Local alarm behavior should be tested independently.
