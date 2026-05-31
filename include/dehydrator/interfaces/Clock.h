#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Monotonic millisecond clock interface for application logic.
 *
 * Production code should adapt this to Arduino `millis()`. Tests can provide
 * a fake clock so timing behavior is deterministic and does not require real
 * waiting.
 */
class Clock {
 public:
  /**
   * @brief Virtual destructor for safe polymorphic use.
   */
  virtual ~Clock() = default;

  /**
   * @brief Returns monotonic elapsed milliseconds.
   *
   * @return Current monotonic time in milliseconds.
   */
  virtual uint32_t millis() const = 0;
};

}  // namespace dehydrator
