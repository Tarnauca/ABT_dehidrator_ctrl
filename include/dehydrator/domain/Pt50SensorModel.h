#pragma once

#include <stdint.h>

#include "dehydrator/config/RuntimeConfig.h"

namespace dehydrator {

/**
 * @brief Converted PT50 reading from one raw ADC sample.
 */
struct Pt50Reading {
  /** Raw ADC count used for conversion. */
  uint16_t adcCount = 0;
  /** Calculated PT50 resistance in milliohms. */
  int32_t resistanceMilliOhms = 0;
  /** Calibrated integer temperature in Celsius. */
  int16_t tempC = 0;
  /** True when ADC, resistance, and temperature are plausible. */
  bool valid = false;
};

/**
 * @brief Pure PT50 ADC conversion and calibration model.
 *
 * The model assumes a ratiometric voltage divider supplied by the same VCC used
 * as ADC reference. It therefore works from ADC count ratios and configured
 * resistor values instead of absolute voltage.
 */
class Pt50SensorModel {
 public:
  /**
   * @brief Converts one raw ADC sample into a PT50 temperature reading.
   *
   * @param config Calibration and divider constants.
   * @param adcCount Raw ADC count.
   * @return Converted reading with validity flag.
   */
  static Pt50Reading convert(const config::CalibrationConfig& config,
                             uint16_t adcCount) {
    Pt50Reading reading;
    reading.adcCount = adcCount;

    if (adcCount == 0U || adcCount >= config.adcMaxCount ||
        config.pt50FixedResistorMilliOhms <= 0 ||
        config.pt50NominalMilliOhms <= 0 || config.pt50AlphaPpmPerC <= 0 ||
        config.pt50ScalePpm <= 0) {
      return reading;
    }

    reading.resistanceMilliOhms = resistanceFromAdc(config, adcCount);
    if (reading.resistanceMilliOhms <= 0) {
      return reading;
    }

    const int32_t rawCentiC =
        resistanceToCentiC(config, reading.resistanceMilliOhms);
    const int32_t calibratedCentiC =
        (static_cast<int64_t>(rawCentiC + config.pt50OffsetCentiC) *
         config.pt50ScalePpm) /
        1000000L;
    const int32_t calibratedTempC = roundCentiCToCelsius(calibratedCentiC);
    reading.valid = calibratedTempC >= config.pt50MinValidTempC &&
                    calibratedTempC <= config.pt50MaxValidTempC &&
                    calibratedTempC >= INT16_MIN &&
                    calibratedTempC <= INT16_MAX;

    if (reading.valid) {
      reading.tempC = static_cast<int16_t>(calibratedTempC);
    }
    return reading;
  }

 private:
  static int32_t resistanceFromAdc(const config::CalibrationConfig& config,
                                   uint16_t adcCount) {
    const int64_t fixed = config.pt50FixedResistorMilliOhms;
    const int64_t adc = adcCount;
    const int64_t adcMax = config.adcMaxCount;

    if (config.pt50DividerOrientation ==
        config::DividerOrientation::FixedHighPt50Low) {
      return static_cast<int32_t>((fixed * adc) / (adcMax - adc));
    }

    return static_cast<int32_t>((fixed * (adcMax - adc)) / adc);
  }

  static int32_t resistanceToCentiC(const config::CalibrationConfig& config,
                                    int32_t resistanceMilliOhms) {
    const int64_t delta = static_cast<int64_t>(resistanceMilliOhms) -
                          config.pt50NominalMilliOhms;
    const int64_t numerator = delta * 100000000LL;
    const int64_t denominator =
        static_cast<int64_t>(config.pt50NominalMilliOhms) *
        config.pt50AlphaPpmPerC;
    return static_cast<int32_t>(numerator / denominator);
  }

  static int32_t roundCentiCToCelsius(int32_t centiC) {
    if (centiC >= 0) {
      return (centiC + 50) / 100;
    }

    return (centiC - 50) / 100;
  }
};

}  // namespace dehydrator
