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
   * @brief Formats the periodic bring-up state record with primary thermistor fields.
   *
   * @param buffer Destination character buffer.
   * @param bufferSize Size of `buffer` in bytes.
   * @param uptimeMs Current firmware uptime in milliseconds.
   * @param ledOn Whether the built-in LED is currently on.
   * @param ntcValid Whether the primary thermistor reading is valid.
   * @param ntcTempDeciC Latest primary thermistor temperature in deci-Celsius.
   * @param ntcAdcCount Latest raw primary thermistor ADC count.
   * @return true if the full line fit in the destination buffer.
   */
  static bool formatBringupState(char* buffer, size_t bufferSize,
                                 uint32_t uptimeMs, bool ledOn,
                                 bool ntcValid, int16_t ntcTempDeciC,
                                 uint16_t ntcAdcCount, bool tempRhValid,
                                 int16_t tempRhTempDeciC, uint8_t rhPercent,
                                 const char* runStateToken = nullptr,
                                 const char* presetToken = nullptr,
                                 bool heaterOn = false, bool fanOn = false) {
    const char* stateToken = safeToken(runStateToken);
    const char* activePreset = safeToken(presetToken);
    char ntcValue[12] = {};
    char tempRhValue[12] = {};
    if (!ntcValid && !tempRhValid) {
      return format(buffer, bufferSize,
                    "STATE app=bringup uptime_ms=%lu led=%s run=%s preset=%s h=%s f=%s ntc=null adc=%u env_t=null rh=null",
                    static_cast<unsigned long>(uptimeMs),
                    ledOn ? "on" : "off",
                    stateToken,
                    activePreset,
                    heaterOn ? "on" : "off",
                    fanOn ? "on" : "off",
                    static_cast<unsigned int>(ntcAdcCount));
    }

    if (!ntcValid) {
      formatDeciTemperature(tempRhTempDeciC, tempRhValue, sizeof(tempRhValue));
      return format(buffer, bufferSize,
                    "STATE app=bringup uptime_ms=%lu led=%s run=%s preset=%s h=%s f=%s ntc=null adc=%u env_t=%s rh=%u",
                    static_cast<unsigned long>(uptimeMs), ledOn ? "on" : "off",
                    stateToken,
                    activePreset,
                    heaterOn ? "on" : "off",
                    fanOn ? "on" : "off",
                    static_cast<unsigned int>(ntcAdcCount),
                    tempRhValue,
                    tempRhValid ? static_cast<unsigned int>(rhPercent) : 0U);
    }

    if (!tempRhValid) {
      formatDeciTemperature(ntcTempDeciC, ntcValue, sizeof(ntcValue));
      return format(buffer, bufferSize,
                    "STATE app=bringup uptime_ms=%lu led=%s run=%s preset=%s h=%s f=%s ntc=%s adc=%u env_t=null rh=null",
                    static_cast<unsigned long>(uptimeMs), ledOn ? "on" : "off",
                    stateToken,
                    activePreset,
                    heaterOn ? "on" : "off",
                    fanOn ? "on" : "off",
                    ntcValue,
                    static_cast<unsigned int>(ntcAdcCount));
    }

    formatDeciTemperature(ntcTempDeciC, ntcValue, sizeof(ntcValue));
    formatDeciTemperature(tempRhTempDeciC, tempRhValue, sizeof(tempRhValue));
    return format(buffer, bufferSize,
                  "STATE app=bringup uptime_ms=%lu led=%s run=%s preset=%s h=%s f=%s ntc=%s adc=%u env_t=%s rh=%u",
                  static_cast<unsigned long>(uptimeMs), ledOn ? "on" : "off",
                  stateToken,
                  activePreset,
                  heaterOn ? "on" : "off",
                  fanOn ? "on" : "off",
                  ntcValue,
                  static_cast<unsigned int>(ntcAdcCount),
                  tempRhValue,
                  static_cast<unsigned int>(rhPercent));
  }

 private:
  static void formatDeciTemperature(int16_t tempDeciC, char* buffer,
                                    size_t bufferSize) {
    const bool negative = tempDeciC < 0;
    const int16_t magnitude =
        static_cast<int16_t>(negative ? -tempDeciC : tempDeciC);
    snprintf(buffer, bufferSize, "%s%d.%d", negative ? "-" : "",
             static_cast<int>(magnitude / 10),
             static_cast<int>(magnitude % 10));
  }

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
