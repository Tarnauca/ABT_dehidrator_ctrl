#pragma once

#include <stdint.h>

#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/interfaces/AhtSensorDriver.h"

namespace dehydrator {

/**
 * @brief Calibrated AHT-like temperature/RH reading.
 */
struct AhtReading {
  /** Calibrated integer temperature in Celsius. */
  int16_t tempC = 0;
  /** Calibrated integer relative humidity in percent. */
  uint8_t rhPercent = 0;
  /** True when both temperature and RH are plausible. */
  bool valid = false;
};

/**
 * @brief AHT-like sensor reader for secondary temperature/RH telemetry.
 *
 * The AHT sensor is not a safety-control source in the current scope. Invalid
 * readings should be handled by higher layers as warnings, not hard faults.
 */
class AhtReader {
 public:
  /**
   * @brief Creates an AHT reader.
   *
   * @param driver Hardware/library driver interface.
   * @param calibration Calibration and plausible-range configuration.
   */
  AhtReader(AhtSensorDriver& driver,
            const config::CalibrationConfig& calibration)
      : driver_(driver), calibration_(calibration) {}

  /**
   * @brief Reads and calibrates one AHT sample.
   *
   * @return Calibrated AHT reading with validity flag.
   */
  AhtReading read() {
    const AhtRawSample raw = driver_.readSample();
    AhtReading reading;

    if (!raw.valid) {
      return reading;
    }

    const int32_t calibratedTempCentiC =
        static_cast<int32_t>(raw.tempCentiC) + calibration_.ahtTempOffsetCentiC;
    const int32_t calibratedRhCentiPercent =
        static_cast<int32_t>(raw.rhCentiPercent) +
        calibration_.ahtRhOffsetCentiPercent;

    const int32_t tempC = roundCentiToInteger(calibratedTempCentiC);
    const int32_t rhPercent = roundCentiToInteger(calibratedRhCentiPercent);

    if (tempC < calibration_.ahtMinValidTempC ||
        tempC > calibration_.ahtMaxValidTempC || rhPercent < 0 ||
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

  AhtSensorDriver& driver_;
  const config::CalibrationConfig& calibration_;
};

}  // namespace dehydrator
