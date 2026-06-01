#pragma once

#include <stddef.h>

#include "dehydrator/logging/LogSink.h"

namespace dehydrator {

/**
 * @brief Fixed-capacity fan-out dispatcher for structured log lines.
 *
 * The dispatcher stores sink pointers in a caller-provided fixed array. This
 * supports USB debug plus secondary telemetry logging without dynamic
 * allocation.
 */
class LogDispatcher {
 public:
  /**
   * @brief Creates a dispatcher backed by fixed sink storage.
   *
   * @param sinks Storage for sink pointers. Storage must remain valid for the
   * lifetime of the dispatcher.
   * @param capacity Number of entries available in `sinks`.
   */
  constexpr LogDispatcher(LogSink** sinks, size_t capacity)
      : sinks_(sinks), capacity_(capacity), count_(0) {}

  /**
   * @brief Adds a sink if capacity allows and the pointer is valid.
   *
   * @param sink Destination sink to add.
   * @return true if the sink was added.
   */
  bool addSink(LogSink& sink) {
    if (sinks_ == nullptr || count_ >= capacity_) {
      return false;
    }

    sinks_[count_] = &sink;
    ++count_;
    return true;
  }

  /**
   * @brief Writes a line to every configured sink.
   *
   * @param line Null-terminated structured log line. Null lines are ignored.
   */
  void writeLine(const char* line) {
    if (line == nullptr) {
      return;
    }

    for (size_t index = 0; index < count_; ++index) {
      sinks_[index]->writeLine(line);
    }
  }

  /**
   * @brief Returns the number of configured sinks.
   *
   * @return Active sink count.
   */
  size_t sinkCount() const { return count_; }

 private:
  LogSink** sinks_;
  size_t capacity_;
  size_t count_;
};

}  // namespace dehydrator
