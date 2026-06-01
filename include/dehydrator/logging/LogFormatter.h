#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

namespace dehydrator {

/**
 * @brief Utility for formatting structured log lines into fixed buffers.
 *
 * The formatter intentionally avoids dynamic allocation and Arduino `String`.
 * Returned values indicate whether the destination buffer was large enough for
 * the complete line.
 */
class LogFormatter {
 public:
  /**
   * @brief Formats a structured event record.
   *
   * @param buffer Destination character buffer.
   * @param bufferSize Size of `buffer` in bytes.
   * @param type Stable event type token. Null is emitted as `null`.
   * @param detail Stable detail token or short value. Null is emitted as
   * `null`.
   * @return true if the full line fit in the destination buffer.
   */
  static bool formatEvent(char* buffer, size_t bufferSize, const char* type,
                          const char* detail) {
    return format(buffer, bufferSize, "EVENT type=%s detail=%s", safeToken(type),
                  safeToken(detail));
  }

  /**
   * @brief Formats the scheduler shell periodic state record.
   *
   * @param buffer Destination character buffer.
   * @param bufferSize Size of `buffer` in bytes.
   * @param uptimeMs Current firmware uptime in milliseconds.
   * @param ledOn Whether the built-in LED is currently on.
   * @return true if the full line fit in the destination buffer.
   */
  static bool formatSchedulerState(char* buffer, size_t bufferSize,
                                   uint32_t uptimeMs, bool ledOn) {
    return format(buffer, bufferSize, "STATE app=scheduler_shell uptime_ms=%lu led=%s",
                  static_cast<unsigned long>(uptimeMs), ledOn ? "on" : "off");
  }

 private:
  /**
   * @brief Converts a nullable token pointer to a printable token.
   *
   * @param token Candidate token pointer.
   * @return `token` when non-null, otherwise the stable token `null`.
   */
  static constexpr const char* safeToken(const char* token) {
    return token == nullptr ? "null" : token;
  }

  template <typename... Args>
  static bool format(char* buffer, size_t bufferSize, const char* pattern,
                     Args... args) {
    if (buffer == nullptr || bufferSize == 0U) {
      return false;
    }

    const int written = snprintf(buffer, bufferSize, pattern, args...);
    return written >= 0 && static_cast<size_t>(written) < bufferSize;
  }
};

}  // namespace dehydrator
