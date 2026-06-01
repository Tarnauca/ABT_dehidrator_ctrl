#pragma once

namespace dehydrator {

/**
 * @brief Destination for complete structured log lines.
 *
 * Implementations may write to USB serial, secondary telemetry serial, or test
 * capture buffers. The line passed to `writeLine()` must be a complete
 * null-terminated log record without a trailing newline requirement.
 */
class LogSink {
 public:
  /**
   * @brief Virtual destructor for safe polymorphic use.
   */
  virtual ~LogSink() = default;

  /**
   * @brief Writes one complete log line.
   *
   * @param line Null-terminated structured log line.
   */
  virtual void writeLine(const char* line) = 0;
};

}  // namespace dehydrator
