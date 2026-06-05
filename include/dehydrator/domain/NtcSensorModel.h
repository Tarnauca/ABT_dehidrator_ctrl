#pragma once

#include <math.h>
#include <stdint.h>

#include "dehydrator/config/RuntimeConfig.h"

namespace dehydrator {

/**
 * @brief Converted primary thermistor reading from one raw ADC sample.
 */
struct NtcReading {
  /** Raw ADC count used for conversion. */
  uint16_t adcCount = 0;
  /** Calculated thermistor resistance in milliohms. */
  int32_t resistanceMilliOhms = 0;
  /** Calibrated temperature in deci-Celsius. */
  int16_t tempDeciC = 0;
  /** True when ADC, resistance, and temperature are plausible. */
  bool valid = false;
};

/**
 * @brief Pure thermistor ADC conversion and calibration model.
 *
 * The model assumes a ratiometric voltage divider supplied by the same VCC used
 * as ADC reference. It therefore works from ADC count ratios and configured
 * resistor values instead of absolute voltage.
 */
class NtcSensorModel {
 public:
  /**
   * @brief Converts one raw ADC sample into a thermistor temperature reading.
   *
   * @param config Calibration and divider constants.
   * @param adcCount Raw ADC count.
   * @return Converted reading with validity flag.
   */
  static NtcReading convert(const config::CalibrationConfig& config,
                            uint16_t adcCount) {
    NtcReading reading;
    reading.adcCount = adcCount;

    if (adcCount == 0U || adcCount >= config.adcMaxCount ||
        config.ntcFixedResistorMilliOhms <= 0 ||
        config.ntcNominalMilliOhms <= 0 || config.ntcBetaK <= 0 ||
        config.ntcNominalTempCentiC <= -27315 || config.ntcScalePpm <= 0) {
      return reading;
    }

    reading.resistanceMilliOhms = resistanceFromAdc(config, adcCount);
    if (reading.resistanceMilliOhms <= 0) {
      return reading;
    }

    const int32_t rawCentiC =
        resistanceToCentiC(config, reading.resistanceMilliOhms);
    const int32_t calibratedCentiC =
        (static_cast<int64_t>(rawCentiC + config.ntcOffsetCentiC) *
         config.ntcScalePpm) /
        1000000L;
    const int32_t calibratedTempDeciC = roundCentiCToDeciC(calibratedCentiC);
    reading.valid =
        calibratedTempDeciC >=
            static_cast<int32_t>(config.ntcMinValidTempC) * 10L &&
        calibratedTempDeciC <=
            static_cast<int32_t>(config.ntcMaxValidTempC) * 10L &&
        calibratedTempDeciC >= INT16_MIN &&
        calibratedTempDeciC <= INT16_MAX;

    if (reading.valid) {
      reading.tempDeciC = static_cast<int16_t>(calibratedTempDeciC);
    }
    return reading;
  }

 private:
  static int32_t resistanceFromAdc(const config::CalibrationConfig& config,
                                   uint16_t adcCount) {
    const int64_t fixed = config.ntcFixedResistorMilliOhms;
    const int64_t adc = adcCount;
    const int64_t adcMax = config.adcMaxCount;

    if (config.ntcDividerOrientation ==
        config::DividerOrientation::FixedHighNtcLow) {
      return static_cast<int32_t>((fixed * adc) / (adcMax - adc));
    }

    return static_cast<int32_t>((fixed * (adcMax - adc)) / adc);
  }

  static int32_t resistanceToCentiC(const config::CalibrationConfig& config,
                                    int32_t resistanceMilliOhms) {
    const float resistance = static_cast<float>(resistanceMilliOhms);
    const float nominalResistance =
        static_cast<float>(config.ntcNominalMilliOhms);
    const float nominalTempC =
        static_cast<float>(config.ntcNominalTempCentiC) / 100.0f;
    const float nominalTempK = nominalTempC + 273.15f;
    const float beta = static_cast<float>(config.ntcBetaK);
    const float tempK = 1.0f /
                        ((1.0f / nominalTempK) +
                         (logf(resistance / nominalResistance) / beta));
    const float tempC = tempK - 273.15f;
    return static_cast<int32_t>(tempC * 100.0f + (tempC >= 0.0f ? 0.5f : -0.5f));
  }

  static int32_t roundCentiCToDeciC(int32_t centiC) {
    if (centiC >= 0) {
      return (centiC + 5) / 10;
    }

    return (centiC - 5) / 10;
  }
};

}  // namespace dehydrator
