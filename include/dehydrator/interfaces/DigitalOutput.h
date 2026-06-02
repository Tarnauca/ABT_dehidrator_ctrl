#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Interface for writing logical MCU pin levels.
 *
 * Hardware implementations wrap Arduino `pinMode()`/`digitalWrite()` or a
 * future GPIO driver. Output adapters depend on this interface so polarity and
 * safety sequencing can be tested without connected hardware.
 */
class DigitalOutput {
 public:
  /**
   * @brief Virtual destructor for safe polymorphic use.
   */
  virtual ~DigitalOutput() = default;

  /**
   * @brief Preloads one pin level and configures it as a digital output.
   *
   * Implementations should write the output latch before enabling output mode
   * where the platform supports it. This avoids short active-low relay pulses
   * during startup.
   *
   * @param pin Board-specific digital pin number.
   * @param initialHigh Initial logical MCU level to drive.
   */
  virtual void configureOutput(uint8_t pin, bool initialHigh) = 0;

  /**
   * @brief Writes one logical MCU level to a pin.
   *
   * @param pin Board-specific digital pin number.
   * @param high True for high level, false for low level.
   */
  virtual void write(uint8_t pin, bool high) = 0;
};

}  // namespace dehydrator
