#include <Arduino.h>

#include "dehydrator/app/PeriodicTask.h"

constexpr unsigned long BAUD_RATE = 115200;
constexpr uint32_t LED_TASK_INTERVAL_MS = 1000;
constexpr uint32_t STATE_LOG_INTERVAL_MS = 5000;

dehydrator::PeriodicTask ledTask(LED_TASK_INTERVAL_MS);
dehydrator::PeriodicTask stateLogTask(STATE_LOG_INTERVAL_MS);

bool ledOn = false;

void logEvent(const __FlashStringHelper* type, const __FlashStringHelper* detail) {
  Serial.print(F("EVENT type="));
  Serial.print(type);
  Serial.print(F(" detail="));
  Serial.println(detail);
}

void updateLedTask(uint32_t nowMs) {
  if (!ledTask.shouldRun(nowMs)) {
    return;
  }

  ledOn = !ledOn;
  digitalWrite(LED_BUILTIN, ledOn ? HIGH : LOW);
  logEvent(F("led"), ledOn ? F("on") : F("off"));
}

void updateStateLogTask(uint32_t nowMs) {
  if (!stateLogTask.shouldRun(nowMs)) {
    return;
  }

  Serial.print(F("STATE app=scheduler_shell uptime_ms="));
  Serial.print(nowMs);
  Serial.print(F(" led="));
  Serial.println(ledOn ? F("on") : F("off"));
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(BAUD_RATE);
  unsigned long serial_start = millis();
  while (!Serial && millis() - serial_start < 2000) {
    // Wait briefly for native USB serial boards, then continue anyway.
  }

  Serial.println(F("Hello world!"));
  Serial.println(F("EVENT type=boot detail=scheduler_shell"));
}

void loop() {
  const uint32_t nowMs = millis();

  updateLedTask(nowMs);
  updateStateLogTask(nowMs);
}
