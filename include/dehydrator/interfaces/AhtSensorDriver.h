#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Raw AHT-like temperature/RH sample from a hardware driver.
 */
struct AhtRawSample {
  /** Temperature reported by the driver, in centi-Celsius. */
  int16_t tempCentiC = 0;
  /** Relative humidity reported by the driver, in centi-percent RH. */
  uint16_t rhCentiPercent = 0;
  /** True when the driver produced a fresh valid sample. */
  bool valid = false;
};

/**
 * @brief Interface for AHT-like temperature/RH hardware drivers.
 *
 * Concrete implementations may wrap an AHT21/AHT20 Arduino library later. The
 * application consumes this interface so secondary telemetry handling can be
 * tested without I2C hardware or a chosen library dependency.
 */
class AhtSensorDriver {
 public:
  /**
   * @brief Virtual destructor for safe polymorphic use.
   */
  virtual ~AhtSensorDriver() = default;

  /**
   * @brief Reads one temperature/RH sample.
   *
   * @return Raw sample with validity flag.
   */
  virtual AhtRawSample readSample() = 0;
};

}  // namespace dehydrator
