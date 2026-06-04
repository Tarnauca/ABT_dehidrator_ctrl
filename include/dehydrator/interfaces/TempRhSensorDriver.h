#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Raw temperature/RH sample from a secondary environment sensor driver.
 */
struct TempRhRawSample {
  /** Temperature reported by the driver, in centi-Celsius. */
  int16_t tempCentiC = 0;
  /** Relative humidity reported by the driver, in centi-percent RH. */
  uint16_t rhCentiPercent = 0;
  /** True when the driver produced a fresh valid sample. */
  bool valid = false;
};

/**
 * @brief Interface for secondary temperature/RH hardware drivers.
 *
 * Concrete implementations may wrap a DHT22/AM2302 library or a future sensor
 * family. Higher-level telemetry code consumes this interface so tests do not
 * depend on a chosen bus or vendor library.
 */
class TempRhSensorDriver {
 public:
  /**
   * @brief Virtual destructor for safe polymorphic use.
   */
  virtual ~TempRhSensorDriver() = default;

  /**
   * @brief Reads one temperature/RH sample.
   *
   * @return Raw sample with validity flag.
   */
  virtual TempRhRawSample readSample() = 0;
};

}  // namespace dehydrator
