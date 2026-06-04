#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "dehydrator/interfaces/AnalogInput.h"

namespace dehydrator {

/**
 * @brief Arduino `analogRead()` adapter for the project analog input interface.
 *
 * The adapter is intentionally thin: calibration, plausibility checks, and PT50
 * conversion remain in testable domain/sensor code.
 */
class ArduinoAnalogInput final : public AnalogInput {
 public:
  /**
   * @brief Reads one raw ADC sample from an Arduino analog channel.
   *
   * @param channel Analog channel or analog pin value accepted by Arduino.
   * @return Raw ADC count from `analogRead()`.
   */
  uint16_t read(uint8_t channel) override {
    return static_cast<uint16_t>(analogRead(channel));
  }
};

}  // namespace dehydrator
