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
  /** LCD status refresh interval in milliseconds. */
  uint32_t lcdRefreshIntervalMs;
  /** Encoder/button sampling interval in milliseconds. */
  uint32_t inputScanIntervalMs;
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

/**
 * @brief Voltage-divider orientation for PT50 resistance calculation.
 */
enum class DividerOrientation {
  /** Fixed resistor to VCC, PT50 to ground. ADC ratio rises with temperature. */
  FixedHighPt50Low,
  /** PT50 to VCC, fixed resistor to ground. ADC ratio falls with temperature. */
  Pt50HighFixedLow,
};

/**
 * @brief Calibration constants for PT50 ADC conversion.
 */
struct CalibrationConfig {
  /** Fixed divider resistor in milliohms. Must match the real circuit. */
  int32_t pt50FixedResistorMilliOhms;
  /** PT50 nominal resistance at 0 C in milliohms. */
  int32_t pt50NominalMilliOhms;
  /** PT50 temperature coefficient in parts per million per C. */
  int32_t pt50AlphaPpmPerC;
  /** Maximum ADC count for the configured ADC resolution. */
  uint16_t adcMaxCount;
  /** Divider orientation used by the real PT50 wiring. */
  DividerOrientation pt50DividerOrientation;
  /** Offset applied after raw PT50 conversion, in centi-Celsius. */
  int16_t pt50OffsetCentiC;
  /** Scale factor applied after offset, in parts per million. */
  int32_t pt50ScalePpm;
  /** Lowest plausible converted PT50 temperature, in Celsius. */
  int16_t pt50MinValidTempC;
  /** Highest plausible converted PT50 temperature, in Celsius. */
  int16_t pt50MaxValidTempC;
  /** AHT temperature offset, in centi-Celsius. */
  int16_t ahtTempOffsetCentiC;
  /** AHT relative humidity offset, in centi-percent RH. */
  int16_t ahtRhOffsetCentiPercent;
  /** Lowest plausible AHT temperature, in Celsius. */
  int16_t ahtMinValidTempC;
  /** Highest plausible AHT temperature, in Celsius. */
  int16_t ahtMaxValidTempC;
};

/** Default serial configuration for USB debug and secondary telemetry. */
constexpr SerialConfig SERIAL_PORTS = {
    115200UL,
};

/** Default scheduler timings for the current firmware shell. */
constexpr SchedulerConfig SCHEDULER = {
    1000UL,
    5000UL,
    1000UL,
    20UL,
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

/** Default PT50 conversion and calibration constants. */
constexpr CalibrationConfig CALIBRATION = {
    100000,
    50000,
    3850,
    1023U,
    DividerOrientation::FixedHighPt50Low,
    0,
    1000000,
    -20,
    120,
    0,
    0,
    -40,
    85,
};

}  // namespace config
}  // namespace dehydrator
