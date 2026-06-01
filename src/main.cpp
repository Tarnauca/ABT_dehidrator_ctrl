#include <Arduino.h>

#include "dehydrator/app/PeriodicTask.h"
#include "dehydrator/logging/LogDispatcher.h"
#include "dehydrator/logging/LogFormatter.h"
#include "dehydrator/logging/LogSink.h"

constexpr unsigned long BAUD_RATE = 115200;
constexpr uint32_t LED_TASK_INTERVAL_MS = 1000;
constexpr uint32_t STATE_LOG_INTERVAL_MS = 5000;
constexpr size_t LOG_SINK_CAPACITY = 2;
constexpr size_t LOG_LINE_SIZE = 96;

constexpr char LOG_TRUNCATED_EVENT[] = "WARN code=log_truncated source=event";
constexpr char LOG_TRUNCATED_STATE[] = "WARN code=log_truncated source=state";

dehydrator::PeriodicTask ledTask(LED_TASK_INTERVAL_MS);
dehydrator::PeriodicTask stateLogTask(STATE_LOG_INTERVAL_MS);

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
dehydrator::LogSink* logSinks[LOG_SINK_CAPACITY] = {};
dehydrator::LogDispatcher logger(logSinks, LOG_SINK_CAPACITY);

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
  char line[LOG_LINE_SIZE] = {};
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
  digitalWrite(LED_BUILTIN, ledOn ? HIGH : LOW);
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

  char line[LOG_LINE_SIZE] = {};
  if (dehydrator::LogFormatter::formatSchedulerState(line, sizeof(line), nowMs,
                                                     ledOn)) {
    writeLogLine(line);
    return;
  }

  writeLogLine(LOG_TRUNCATED_STATE);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(BAUD_RATE);
  Serial1.begin(BAUD_RATE);
  unsigned long serial_start = millis();
  while (!Serial && millis() - serial_start < 2000) {
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
