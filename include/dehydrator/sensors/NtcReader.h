#pragma once

#include <stdint.h>

#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/domain/NtcSensorModel.h"
#include "dehydrator/interfaces/AnalogInput.h"

namespace dehydrator {

/**
 * @brief Primary thermistor reader that connects raw ADC access to the conversion model.
 *
 * The reader does not own hardware. It receives an `AnalogInput` interface so
 * Arduino-specific ADC access stays in a hardware adapter and tests can provide
 * deterministic fake ADC counts.
 */
class NtcReader {
 public:
  /**
   * @brief Creates a thermistor reader for one analog channel.
   *
   * @param analog Raw ADC input provider.
   * @param channel Analog channel or board-specific analog pin value.
   * @param calibration Thermistor conversion and calibration constants.
   */
  NtcReader(AnalogInput& analog, uint8_t channel,
            const config::CalibrationConfig& calibration)
      : analog_(analog), channel_(channel), calibration_(calibration) {}

  /**
   * @brief Reads and converts one thermistor sample.
   *
   * @return Converted thermistor reading with validity flag.
   */
  NtcReading read() {
    return NtcSensorModel::convert(calibration_, analog_.read(channel_));
  }

 private:
  AnalogInput& analog_;
  uint8_t channel_;
  const config::CalibrationConfig& calibration_;
};

}  // namespace dehydrator
