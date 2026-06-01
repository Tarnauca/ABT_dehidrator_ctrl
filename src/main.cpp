#include <Arduino.h>

#include "dehydrator/app/PeriodicTask.h"
#include "dehydrator/config/HardwareConfig.h"
#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/logging/LogDispatcher.h"
#include "dehydrator/logging/LogFormatter.h"
#include "dehydrator/logging/LogSink.h"

constexpr char LOG_TRUNCATED_EVENT[] = "WARN code=log_truncated source=event";
constexpr char LOG_TRUNCATED_STATE[] = "WARN code=log_truncated source=state";

dehydrator::PeriodicTask ledTask(
    dehydrator::config::SCHEDULER.statusLedIntervalMs);
dehydrator::PeriodicTask stateLogTask(
    dehydrator::config::SCHEDULER.stateLogIntervalMs);

bool ledOn = false;

/**
 * @brief Arduino `Stream` adapter for the project log sink interface.
 */
class ArduinoSerialLogSink final : public dehydrator::LogSink {
 public:
  /**
   * @brief Creates a sink that writes lines to an Arduino stream.
   *
   * @param serial Arduino stream used as the destination.
   */
  explicit ArduinoSerialLogSink(Stream& serial) : serial_(serial) {}

  /**
   * @brief Writes a log line followed by the stream newline.
   *
   * @param line Null-terminated structured log line.
   */
  void writeLine(const char* line) override { serial_.println(line); }

 private:
  Stream& serial_;
};

ArduinoSerialLogSink usbLogSink(Serial);
ArduinoSerialLogSink telemetryLogSink(Serial1);
dehydrator::LogSink* logSinks[dehydrator::config::LOGGING.sinkCapacity] = {};
dehydrator::LogDispatcher logger(logSinks,
                                 dehydrator::config::LOGGING.sinkCapacity);

/**
 * @brief Writes one structured log line to all configured sinks.
 *
 * @param line Null-terminated structured log line.
 */
void writeLogLine(const char* line) {
  logger.writeLine(line);
}

/**
 * @brief Formats and writes a structured event line.
 *
 * @param type Stable event type token.
 * @param detail Stable event detail token.
 */
void logEvent(const char* type, const char* detail) {
  char line[dehydrator::config::LOGGING.lineSize] = {};
  if (dehydrator::LogFormatter::formatEvent(line, sizeof(line), type, detail)) {
    writeLogLine(line);
    return;
  }

  writeLogLine(LOG_TRUNCATED_EVENT);
}

/**
 * @brief Cooperative LED task used by the scheduler shell.
 *
 * @param nowMs Current firmware uptime in milliseconds.
 */
void updateLedTask(uint32_t nowMs) {
  if (!ledTask.shouldRun(nowMs)) {
    return;
  }

  ledOn = !ledOn;
  digitalWrite(dehydrator::config::HARDWARE.pins.statusLed, ledOn ? HIGH : LOW);
  logEvent("led", ledOn ? "on" : "off");
}

/**
 * @brief Cooperative periodic state logging task.
 *
 * @param nowMs Current firmware uptime in milliseconds.
 */
void updateStateLogTask(uint32_t nowMs) {
  if (!stateLogTask.shouldRun(nowMs)) {
    return;
  }

  char line[dehydrator::config::LOGGING.lineSize] = {};
  if (dehydrator::LogFormatter::formatSchedulerState(line, sizeof(line), nowMs,
                                                     ledOn)) {
    writeLogLine(line);
    return;
  }

  writeLogLine(LOG_TRUNCATED_STATE);
}

void setup() {
  pinMode(dehydrator::config::HARDWARE.pins.statusLed, OUTPUT);
  digitalWrite(dehydrator::config::HARDWARE.pins.statusLed, LOW);

  Serial.begin(dehydrator::config::SERIAL_PORTS.baudRate);
  Serial1.begin(dehydrator::config::SERIAL_PORTS.baudRate);
  unsigned long serial_start = millis();
  while (!Serial &&
         millis() - serial_start <
             dehydrator::config::SCHEDULER.serialStartupWaitMs) {
    // Wait briefly for native USB serial boards, then continue anyway.
  }

  logger.addSink(usbLogSink);
  logger.addSink(telemetryLogSink);

  writeLogLine("EVENT type=boot detail=scheduler_shell");
}

void loop() {
  const uint32_t nowMs = millis();

  updateLedTask(nowMs);
  updateStateLogTask(nowMs);
}
