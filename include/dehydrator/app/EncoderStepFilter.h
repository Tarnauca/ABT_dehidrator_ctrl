#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Debounces raw quadrature counts into stable detent steps.
 *
 * The `Encoder` library exposes every raw state transition. Mechanical rotary
 * encoders can briefly bounce backwards during a single turn, which shows up as
 * occasional opposite-direction counts. This helper accumulates raw deltas and
 * only emits one logical step after a full, consistent detent-sized movement.
 */
class EncoderStepFilter {
 public:
  /**
   * @brief Creates a filter with the provided counts-per-step threshold.
   *
   * @param countsPerStep Raw encoder counts required for one logical step.
   */
  explicit constexpr EncoderStepFilter(int8_t countsPerStep = 4)
      : countsPerStep_(countsPerStep) {}

  /**
   * @brief Seeds the raw encoder position without emitting a step.
   *
   * @param rawPosition Current raw encoder position.
   */
  void reset(long rawPosition) {
    lastRawPosition_ = rawPosition;
    accumulatedDelta_ = 0;
    initialized_ = true;
  }

  /**
   * @brief Consumes a new raw encoder position.
   *
   * @param rawPosition Latest raw position from the encoder library.
   * @return `1` for one clockwise step, `-1` for one counter-clockwise step,
   * or `0` when no full filtered step is ready.
   */
  int8_t update(long rawPosition) {
    if (!initialized_) {
      reset(rawPosition);
      return 0;
    }

    const long rawDelta = rawPosition - lastRawPosition_;
    lastRawPosition_ = rawPosition;
    if (rawDelta == 0) {
      return 0;
    }

    const int8_t stepDirection = rawDelta > 0 ? 1 : -1;
    if (accumulatedDelta_ != 0 &&
        ((accumulatedDelta_ > 0) != (stepDirection > 0))) {
      accumulatedDelta_ = 0;
    }

    if (rawDelta > 0) {
      accumulatedDelta_ += saturateMagnitude(rawDelta);
    } else {
      accumulatedDelta_ -= saturateMagnitude(-rawDelta);
    }

    if (accumulatedDelta_ >= countsPerStep_) {
      accumulatedDelta_ = 0;
      return 1;
    }

    if (accumulatedDelta_ <= -countsPerStep_) {
      accumulatedDelta_ = 0;
      return -1;
    }

    return 0;
  }

 private:
  int8_t saturateMagnitude(long magnitude) const {
    if (magnitude > countsPerStep_) {
      return countsPerStep_;
    }

    return static_cast<int8_t>(magnitude);
  }

  const int8_t countsPerStep_;
  long lastRawPosition_ = 0L;
  int8_t accumulatedDelta_ = 0;
  bool initialized_ = false;
};

}  // namespace dehydrator
