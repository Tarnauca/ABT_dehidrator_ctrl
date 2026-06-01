#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Small helper for cooperative periodic tasks.
 *
 * `PeriodicTask` keeps scheduling state for one task and reports when the task
 * is due. It is intentionally simple so the firmware can run without dynamic
 * allocation, threads, or blocking delays.
 */
class PeriodicTask {
 public:
  /**
   * @brief Creates a periodic task with a fixed interval.
   *
   * The task is due on the first `shouldRun()` call so startup code can emit
   * initial state without waiting for the first interval.
   *
   * @param intervalMs Period between task executions in milliseconds.
   */
  explicit constexpr PeriodicTask(uint32_t intervalMs)
      : intervalMs_(intervalMs), lastRunMs_(0), hasRun_(false) {}

  /**
   * @brief Returns whether the task should run at the provided time.
   *
   * When this returns true, the internal last-run timestamp is updated to
   * `nowMs`. Unsigned subtraction is used so normal `millis()` wraparound is
   * handled by the usual Arduino timing idiom.
   *
   * @param nowMs Current monotonic time in milliseconds.
   * @return true when the task is due and its timestamp was updated.
   */
  bool shouldRun(uint32_t nowMs) {
    if (!hasRun_) {
      hasRun_ = true;
      lastRunMs_ = nowMs;
      return true;
    }

    if (static_cast<uint32_t>(nowMs - lastRunMs_) >= intervalMs_) {
      lastRunMs_ = nowMs;
      return true;
    }

    return false;
  }

  /**
   * @brief Resets the schedule baseline without running the task.
   *
   * @param nowMs New baseline time in milliseconds.
   */
  void reset(uint32_t nowMs) {
    hasRun_ = true;
    lastRunMs_ = nowMs;
  }

  /**
   * @brief Returns the configured interval.
   *
   * @return Task interval in milliseconds.
   */
  constexpr uint32_t intervalMs() const { return intervalMs_; }

 private:
  uint32_t intervalMs_;
  uint32_t lastRunMs_;
  bool hasRun_;
};

}  // namespace dehydrator
