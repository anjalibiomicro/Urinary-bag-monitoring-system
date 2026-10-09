```cpp
#define BLYNK_TEMPLATE_ID   "TMPLxxxxxx"
#define BLYNK_TEMPLATE_NAME "Urinary Bag Detector"
#define BLYNK_AUTH_TOKEN    "AuthTokenHere"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "HX711.h"

char ssid[] = "WIFI_SSID";
char pass[] = "WIFI_PASSWORD";

const byte HX711_DOUT_PIN = 16;
const byte HX711_SCK_PIN  = 4;
const byte BUZZER_PIN     = 18;
const byte LED_PIN        = 19;

const float BAG_MAX_CAPACITY_ML = 2000.0;
const float LEVEL_HALF_ML       = 1000.0;
const float LEVEL_NEAR_FULL_ML  = 1500.0;
const float LEVEL_CRITICAL_ML   = 1800.0;

// Replace with the calibration factor determined using known weights.
float calibrationFactor = -7050.0;

HX711 scale;
BlynkTimer timer;

bool alarmLatched = false;
bool notificationSent = false;

void readSensorAndProcessLevels() {
  if (!scale.is_ready()) {
    Serial.println(F("Sensor read error: HX711 not responding."));
    Blynk.virtualWrite(V1, "Sensor Error!");
    return;
  }

  float measuredGrams = scale.get_units(10);
  if (measuredGrams < 0) measuredGrams = 0;

  // Approximate fluid volume using 1 g = 1 mL.
  float estimatedVolumeMl = measuredGrams;
  float fillPercentage = (estimatedVolumeMl / BAG_MAX_CAPACITY_ML) * 100.0;

  if (fillPercentage > 100.0) fillPercentage = 100.0;

  Blynk.virtualWrite(V0, estimatedVolumeMl);
  Blynk.virtualWrite(V2, fillPercentage);

  Serial.print(F("Volume: "));
  Serial.print(estimatedVolumeMl, 1);
  Serial.print(F(" mL | Percentage: "));
  Serial.print(fillPercentage, 1);
  Serial.println(F("%"));

  if (estimatedVolumeMl >= LEVEL_CRITICAL_ML) {
    alarmLatched = true;
    Blynk.virtualWrite(V1, "CRITICAL: BAG FULL");
    Blynk.virtualWrite(V5, 255);

    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 2200);

    // Send one push notification per full-bag event.
    if (!notificationSent) {
      Blynk.logEvent("bag_full", "URGENT: Urinary drainage bag is full! Immediate emptying required.");
      notificationSent = true;
    }
  }
  else if (estimatedVolumeMl >= LEVEL_NEAR_FULL_ML) {
    Blynk.virtualWrite(V1, "Warning: Near Full");
    Blynk.virtualWrite(V5, 0);

    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 1800, 150);
  }
  else if (estimatedVolumeMl >= LEVEL_HALF_ML) {
    Blynk.virtualWrite(V1, "Status: Half Full");
    Blynk.virtualWrite(V5, 0);

    if (!alarmLatched) {
      digitalWrite(LED_PIN, LOW);
      noTone(BUZZER_PIN);
    }
  }
  else {
    Blynk.virtualWrite(V1, "Status: Normal");
    Blynk.virtualWrite(V5, 0);

    if (!alarmLatched) {
      digitalWrite(LED_PIN, LOW);
      noTone(BUZZER_PIN);
    }

    notificationSent = false;
  }
}

// Blynk remote tare button on virtual pin V3.
BLYNK_WRITE(V3) {
  int tareButton = param.asInt();

  if (tareButton == 1) {
    scale.tare(20);
    alarmLatched = false;
    notificationSent = false;
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
    Blynk.virtualWrite(V5, 0);

    Serial.println(F("Blynk Command: Tare completed remotely."));
  }
}

// Blynk remote alarm reset button on virtual pin V4.
BLYNK_WRITE(V4) {
  int resetButton = param.asInt();

  if (resetButton == 1) {
    alarmLatched = false;
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);

    Serial.println(F("Blynk Command: Alarm muted temporarily."));
  }
}

void checkSerialCommands() {
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == 't' || cmd == 'T') {
      scale.tare(20);
      alarmLatched = false;
      notificationSent = false;
      digitalWrite(LED_PIN, LOW);
      noTone(BUZZER_PIN);

      Serial.println(F("Local Serial: Scale tared."));
    }
    else if (cmd == 'r' || cmd == 'R') {
      alarmLatched = false;
      digitalWrite(LED_PIN, LOW);
      noTone(BUZZER_PIN);

      Serial.println(F("Local Serial: Alarm reset."));
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);

  if (!scale.is_ready()) {
    Serial.println(F("ERROR: HX711 module not detected. Check circuit."));

    while (!scale.is_ready()) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      delay(200);
    }
  }

  scale.set_scale(calibrationFactor);
  delay(500);
  scale.tare(20);

  Serial.println(F("HX711 initialized and tared."));

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Update sensor readings and dashboard every 1.5 seconds.
  timer.setInterval(1500L, readSensorAndProcessLevels);
}

void loop() {
  Blynk.run();
  timer.run();
  checkSerialCommands();
}
```