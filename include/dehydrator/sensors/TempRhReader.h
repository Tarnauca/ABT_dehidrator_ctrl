#pragma once

#include <stdint.h>

#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/interfaces/TempRhSensorDriver.h"

namespace dehydrator {

/**
 * @brief Calibrated secondary temperature/RH reading.
 */
struct TempRhReading {
  /** Calibrated integer temperature in Celsius. */
  int16_t tempC = 0;
  /** Calibrated integer relative humidity in percent. */
  uint8_t rhPercent = 0;
  /** True when both temperature and RH are plausible. */
  bool valid = false;
};

/**
 * @brief Secondary temperature/RH reader for telemetry and UI display.
 *
 * This sensor is not a safety-control source in the current scope. Invalid
 * readings should be handled by higher layers as warnings, not hard faults.
 */
class TempRhReader {
 public:
  /**
   * @brief Creates a secondary temp/RH reader.
   *
   * @param driver Hardware/library driver interface.
   * @param calibration Calibration and plausible-range configuration.
   */
  TempRhReader(TempRhSensorDriver& driver,
               const config::CalibrationConfig& calibration)
      : driver_(driver), calibration_(calibration) {}

  /**
   * @brief Reads and calibrates one secondary temp/RH sample.
   *
   * @return Calibrated reading with validity flag.
   */
  TempRhReading read() {
    const TempRhRawSample raw = driver_.readSample();
    TempRhReading reading;

    if (!raw.valid) {
      return reading;
    }

    const int32_t calibratedTempCentiC =
        static_cast<int32_t>(raw.tempCentiC) +
        calibration_.tempRhTempOffsetCentiC;
    const int32_t calibratedRhCentiPercent =
        static_cast<int32_t>(raw.rhCentiPercent) +
        calibration_.tempRhRhOffsetCentiPercent;

    const int32_t tempC = roundCentiToInteger(calibratedTempCentiC);
    const int32_t rhPercent = roundCentiToInteger(calibratedRhCentiPercent);

    if (tempC < calibration_.tempRhMinValidTempC ||
        tempC > calibration_.tempRhMaxValidTempC || rhPercent < 0 ||
        rhPercent > 100) {
      return reading;
    }

    reading.tempC = static_cast<int16_t>(tempC);
    reading.rhPercent = static_cast<uint8_t>(rhPercent);
    reading.valid = true;
    return reading;
  }

 private:
  static int32_t roundCentiToInteger(int32_t centiValue) {
    if (centiValue >= 0) {
      return (centiValue + 50) / 100;
    }

    return (centiValue - 50) / 100;
  }

  TempRhSensorDriver& driver_;
  const config::CalibrationConfig& calibration_;
};

}  // namespace dehydrator
