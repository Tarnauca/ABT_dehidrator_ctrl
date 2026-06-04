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
  /** Sensor sampling interval in milliseconds. */
  uint32_t sensorSampleIntervalMs;
  /** Secondary temp/RH sensor sampling interval in milliseconds. */
  uint32_t tempRhSampleIntervalMs;
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
  /** Lowest plausible primary thermistor reading accepted by the fault detector. */
  int16_t ntcMinValidTempC;
  /** Highest plausible primary thermistor reading accepted by the fault detector. */
  int16_t ntcMaxValidTempC;
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
 * @brief Voltage-divider orientation for primary thermistor resistance calculation.
 */
enum class DividerOrientation {
  /** Fixed resistor to VCC, thermistor to ground. ADC ratio rises with temperature. */
  FixedHighNtcLow,
  /** Thermistor to VCC, fixed resistor to ground. ADC ratio falls with temperature. */
  NtcHighFixedLow,
};

/**
 * @brief Calibration constants for the primary thermistor ADC conversion.
 */
struct CalibrationConfig {
  /** Fixed divider resistor in milliohms. Must match the real circuit. */
  int32_t ntcFixedResistorMilliOhms;
  /** Thermistor nominal resistance at the nominal temperature, in milliohms. */
  int32_t ntcNominalMilliOhms;
  /** Thermistor Beta coefficient in kelvin. */
  int32_t ntcBetaK;
  /** Thermistor nominal temperature in centi-Celsius. */
  int16_t ntcNominalTempCentiC;
  /** Maximum ADC count for the configured ADC resolution. */
  uint16_t adcMaxCount;
  /** Divider orientation used by the real thermistor wiring. */
  DividerOrientation ntcDividerOrientation;
  /** Offset applied after raw thermistor conversion, in centi-Celsius. */
  int16_t ntcOffsetCentiC;
  /** Scale factor applied after offset, in parts per million. */
  int32_t ntcScalePpm;
  /** Lowest plausible converted primary temperature, in Celsius. */
  int16_t ntcMinValidTempC;
  /** Highest plausible converted primary temperature, in Celsius. */
  int16_t ntcMaxValidTempC;
  /** Secondary temp/RH sensor temperature offset, in centi-Celsius. */
  int16_t tempRhTempOffsetCentiC;
  /** Secondary temp/RH sensor humidity offset, in centi-percent RH. */
  int16_t tempRhRhOffsetCentiPercent;
  /** Lowest plausible secondary sensor temperature, in Celsius. */
  int16_t tempRhMinValidTempC;
  /** Highest plausible secondary sensor temperature, in Celsius. */
  int16_t tempRhMaxValidTempC;
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
    2000UL,
    1000UL,
    20UL,
    2000UL,
};

/** Default logging configuration for fixed-buffer mirrored logging. */
constexpr LoggingConfig LOGGING = {
    2U,
    160U,
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

/** Default primary thermistor conversion and calibration constants. */
constexpr CalibrationConfig CALIBRATION = {
    100000000,
    100000000,
    3950,
    2500,
    1023U,
    DividerOrientation::FixedHighNtcLow,
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
