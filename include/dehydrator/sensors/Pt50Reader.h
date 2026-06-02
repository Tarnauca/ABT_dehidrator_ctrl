#pragma once

#include <stdint.h>

#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/domain/Pt50SensorModel.h"
#include "dehydrator/interfaces/AnalogInput.h"

namespace dehydrator {

/**
 * @brief PT50 reader that connects raw ADC access to the PT50 conversion model.
 *
 * The reader does not own hardware. It receives an `AnalogInput` interface so
 * Arduino-specific ADC access stays in a hardware adapter and tests can provide
 * deterministic fake ADC counts.
 */
class Pt50Reader {
 public:
  /**
   * @brief Creates a PT50 reader for one analog channel.
   *
   * @param analog Raw ADC input provider.
   * @param channel Analog channel or board-specific analog pin value.
   * @param calibration PT50 conversion and calibration constants.
   */
  Pt50Reader(AnalogInput& analog, uint8_t channel,
             const config::CalibrationConfig& calibration)
      : analog_(analog), channel_(channel), calibration_(calibration) {}

  /**
   * @brief Reads and converts one PT50 sample.
   *
   * @return Converted PT50 reading with validity flag.
   */
  Pt50Reading read() {
    return Pt50SensorModel::convert(calibration_, analog_.read(channel_));
  }

 private:
  AnalogInput& analog_;
  uint8_t channel_;
  const config::CalibrationConfig& calibration_;
};

}  // namespace dehydrator
