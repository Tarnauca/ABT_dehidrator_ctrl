#pragma once

#include <stddef.h>
#include <stdint.h>

namespace dehydrator {
namespace config {

/**
 * @brief Serial configuration shared by debug and telemetry ports.
 */
struct SerialConfig {
  /** Serial baud rate. */
  uint32_t baudRate;
};

/**
 * @brief Cooperative scheduler timing configuration.
 */
struct SchedulerConfig {
  /** Built-in status LED toggle interval in milliseconds. */
  uint32_t statusLedIntervalMs;
  /** Periodic state log interval in milliseconds. */
  uint32_t stateLogIntervalMs;
  /** Startup wait for USB serial availability in milliseconds. */
  uint32_t serialStartupWaitMs;
};

/**
 * @brief Logging buffer and sink configuration.
 */
struct LoggingConfig {
  /** Maximum number of mirrored log sinks. */
  size_t sinkCapacity;
  /** Fixed log line buffer size in bytes. */
  size_t lineSize;
};

/**
 * @brief Temperature control configuration for relay-based heater control.
 */
struct ControlConfig {
  /** Hysteresis distance below target before heater may turn ON. */
  int16_t hysteresisC;
  /** Minimum commanded heater ON time for normal hysteresis switching. */
  uint16_t minHeaterOnSeconds;
  /** Minimum commanded heater OFF time for normal hysteresis switching. */
  uint16_t minHeaterOffSeconds;
  /** Temperature above which heater is forced OFF by safety policy. */
  int16_t heaterForceOffAboveTempC;
};

/** Default serial configuration for USB debug and secondary telemetry. */
constexpr SerialConfig SERIAL_PORTS = {
    115200UL,
};

/** Default scheduler timings for the current firmware shell. */
constexpr SchedulerConfig SCHEDULER = {
    1000UL,
    5000UL,
    2000UL,
};

/** Default logging configuration for fixed-buffer mirrored logging. */
constexpr LoggingConfig LOGGING = {
    2U,
    96U,
};

/** Default control settings for simple relay hysteresis. */
constexpr ControlConfig CONTROL = {
    1,
    10U,
    10U,
    75,
};

}  // namespace config
}  // namespace dehydrator
