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

  /**
   * @brief Formats the periodic bring-up state record with PT50 fields.
   *
   * @param buffer Destination character buffer.
   * @param bufferSize Size of `buffer` in bytes.
   * @param uptimeMs Current firmware uptime in milliseconds.
   * @param ledOn Whether the built-in LED is currently on.
   * @param pt50Valid Whether the PT50 reading is valid.
   * @param pt50TempC Latest PT50 temperature in Celsius.
   * @param pt50AdcCount Latest raw PT50 ADC count.
   * @return true if the full line fit in the destination buffer.
   */
  static bool formatBringupState(char* buffer, size_t bufferSize,
                                 uint32_t uptimeMs, bool ledOn,
                                 bool pt50Valid, int16_t pt50TempC,
                                 uint16_t pt50AdcCount, bool ahtValid,
                                 int16_t ahtTempC, uint8_t rhPercent) {
    if (!pt50Valid && !ahtValid) {
      return format(buffer, bufferSize,
                    "STATE app=bringup uptime_ms=%lu led=%s pt50=null adc=%u aht_t=null rh=null",
                    static_cast<unsigned long>(uptimeMs),
                    ledOn ? "on" : "off",
                    static_cast<unsigned int>(pt50AdcCount));
    }

    if (!pt50Valid) {
      return format(buffer, bufferSize,
                    "STATE app=bringup uptime_ms=%lu led=%s pt50=null adc=%u aht_t=%d rh=%u",
                    static_cast<unsigned long>(uptimeMs), ledOn ? "on" : "off",
                    static_cast<unsigned int>(pt50AdcCount),
                    ahtValid ? static_cast<int>(ahtTempC) : 0,
                    ahtValid ? static_cast<unsigned int>(rhPercent) : 0U);
    }

    if (!ahtValid) {
      return format(buffer, bufferSize,
                    "STATE app=bringup uptime_ms=%lu led=%s pt50=%d adc=%u aht_t=null rh=null",
                    static_cast<unsigned long>(uptimeMs), ledOn ? "on" : "off",
                    static_cast<int>(pt50TempC),
                    static_cast<unsigned int>(pt50AdcCount));
    }

    return format(buffer, bufferSize,
                  "STATE app=bringup uptime_ms=%lu led=%s pt50=%d adc=%u aht_t=%d rh=%u",
                  static_cast<unsigned long>(uptimeMs), ledOn ? "on" : "off",
                  static_cast<int>(pt50TempC),
                  static_cast<unsigned int>(pt50AdcCount),
                  static_cast<int>(ahtTempC),
                  static_cast<unsigned int>(rhPercent));
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
