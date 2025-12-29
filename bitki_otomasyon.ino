#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

const int MOISTURE_PIN = A5;
const int PUMP_PIN      = 6;
const int LED_PIN       = 5;

const int DRY_THRESHOLD    = 900;
const int PUMP_TIME_SEC    = 2;
const int RECHECK_TIME_SEC = 10;

void printTwoDigits(int v) {
  if (v < 10) Serial.print('0');
  Serial.print(v);
}

void printStatus(uint16_t raw, bool isDry, bool lightOn, bool pumpOn, DateTime now) {
  Serial.print("Saat: ");
  printTwoDigits(now.hour());
  Serial.print(":");
  printTwoDigits(now.minute());
  Serial.print(":");
  printTwoDigits(now.second());

  Serial.print(" | Nem ham: ");
  Serial.print(raw);

  Serial.print(" | Durum: ");
  Serial.print(isDry ? "KURU" : "NEMLI");

  Serial.print(" | Isik: ");
  Serial.print(lightOn ? "ACIK" : "KAPALI");

  Serial.print(" | Pompa: ");
  Serial.print(pumpOn ? "ACIK" : "KAPALI");

  Serial.println();
}

void setup() {
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(LED_PIN, HIGH);

  Serial.begin(9600);
  while (!Serial) {
  }

  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("RTC bulunamadi, takili mi bak.");
    while (1);
  }

  Serial.println("Bitki sistemi basladi.");
}

void loop() {
  bool lightOn = true;

  DateTime now = rtc.now();
  uint16_t raw = analogRead(MOISTURE_PIN);
  bool isDry = (raw > DRY_THRESHOLD);
  bool pumpOn = false;

  printStatus(raw, isDry, lightOn, pumpOn, now);

  if (isDry) {
    Serial.println("Toprak KURU algilandi, sulama dongusu basliyor.");

    digitalWrite(PUMP_PIN, HIGH);
    pumpOn = true;
    for (int s = 0; s < PUMP_TIME_SEC; s++) {
      delay(1000);
      now = rtc.now();
      raw = analogRead(MOISTURE_PIN);
      isDry = (raw > DRY_THRESHOLD);
      printStatus(raw, isDry, lightOn, pumpOn, now);
    }
    digitalWrite(PUMP_PIN, LOW);
    pumpOn = false;
    Serial.println("Pompa KAPANDI, 10 saniye gozlem...");

    for (int s = 0; s < RECHECK_TIME_SEC; s++) {
      delay(1000);
      now = rtc.now();
      raw = analogRead(MOISTURE_PIN);
      isDry = (raw > DRY_THRESHOLD);
      printStatus(raw, isDry, lightOn, pumpOn, now);
    }

    return;
  }

  delay(1000);
}
