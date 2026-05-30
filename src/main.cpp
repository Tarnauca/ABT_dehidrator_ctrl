#include <Arduino.h>

constexpr unsigned long BAUD_RATE = 115200;
constexpr unsigned long BLINK_INTERVAL_MS = 1000;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(BAUD_RATE);
  unsigned long serial_start = millis();
  while (!Serial && millis() - serial_start < 2000) {
    // Wait briefly for native USB serial boards, then continue anyway.
  }

  Serial.println(F("Hello world!"));
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println(F("LED ON"));
  delay(BLINK_INTERVAL_MS);

  digitalWrite(LED_BUILTIN, LOW);
  Serial.println(F("LED OFF"));
  delay(BLINK_INTERVAL_MS);
}
