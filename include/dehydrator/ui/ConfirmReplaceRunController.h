#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Result of one confirm-replace-run UI action.
 */
struct ConfirmReplaceRunResult {
  /** True when the selected answer changed. */
  bool selectionChanged = false;
  /** True when the user confirmed replacing the active run. */
  bool confirmed = false;
  /** True when the user cancelled and wants to return. */
  bool cancelled = false;
};

/**
 * @brief Minimal yes/no controller for replacing an active preset run.
 */
class ConfirmReplaceRunController {
 public:
  /**
   * @brief Resets the confirmation choice to the safe default `Nu`.
   */
  void reset() { confirmSelected_ = false; }

  /**
   * @brief Handles one encoder step between `Nu` and `Da`.
   *
   * @param delta Positive for clockwise, negative for counter-clockwise.
   * @return Result indicating whether the selection changed.
   */
  ConfirmReplaceRunResult onRotate(int8_t delta) {
    if (delta == 0) {
      return {};
    }

    const bool previous = confirmSelected_;
    if (delta > 0) {
      confirmSelected_ = true;
    } else {
      confirmSelected_ = false;
    }

    ConfirmReplaceRunResult result;
    result.selectionChanged = previous != confirmSelected_;
    return result;
  }

  /**
   * @brief Handles a short press on the current answer.
   *
   * @return Result with either `confirmed` or `cancelled`.
   */
  ConfirmReplaceRunResult onShortPress() const {
    ConfirmReplaceRunResult result;
    result.confirmed = confirmSelected_;
    result.cancelled = !confirmSelected_;
    return result;
  }

  /**
   * @brief Returns whether `Da` is currently selected.
   *
   * @return true when the confirm action is selected.
   */
  bool confirmSelected() const { return confirmSelected_; }

 private:
  bool confirmSelected_ = false;
};

}  // namespace dehydrator
