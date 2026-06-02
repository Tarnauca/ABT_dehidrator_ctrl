#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Interface for reading raw ADC counts from analog inputs.
 *
 * Hardware implementations wrap Arduino `analogRead()` or a future ADC driver.
 * Pure sensor readers depend on this interface so conversion logic can be
 * tested without connected hardware.
 */
class AnalogInput {
 public:
  /**
   * @brief Virtual destructor for safe polymorphic use.
   */
  virtual ~AnalogInput() = default;

  /**
   * @brief Reads one raw ADC count from an analog channel.
   *
   * @param channel Logical analog channel or board-specific analog pin value.
   * @return Raw ADC count from the hardware adapter.
   */
  virtual uint16_t read(uint8_t channel) = 0;
};

}  // namespace dehydrator
