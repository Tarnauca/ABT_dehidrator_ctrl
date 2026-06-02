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

/**
 * @brief Safety thresholds for hard-fault detection.
 */
struct SafetyConfig {
  /** Lowest plausible PT50 reading accepted by the fault detector. */
  int16_t pt50MinValidTempC;
  /** Highest plausible PT50 reading accepted by the fault detector. */
  int16_t pt50MaxValidTempC;
  /** Temperature at or above which an over-temperature hard fault occurs. */
  int16_t hardFaultTempC;
  /** Minimum required rise while heater is commanded ON. */
  int16_t noRiseMinIncreaseC;
  /** Accumulated heater ON time allowed before no-rise fault. */
  uint16_t noRiseWindowSeconds;
  /** Grace time after heater command OFF before stuck-heater monitoring starts. */
  uint16_t stuckHeaterGraceSeconds;
  /** Temperature rise while heater is OFF that indicates suspected stuck ON. */
  int16_t stuckHeaterRiseC;
  /** Monitoring window for stuck-heater temperature rise. */
  uint16_t stuckHeaterWindowSeconds;
  /** Continuous active button time before stuck-input hard fault. */
  uint16_t buttonStuckSeconds;
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

/** Default safety thresholds agreed during requirements discovery. */
constexpr SafetyConfig SAFETY = {
    -20,
    120,
    80,
    2,
    5U * 60U,
    2U * 60U,
    3,
    5U * 60U,
    30U,
};

}  // namespace config
}  // namespace dehydrator
