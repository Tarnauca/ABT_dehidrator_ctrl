#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Three-way decision for save-before-leave/save-before-start prompts.
 */
enum class SavePromptChoice {
  /** Save the current profile first. */
  Yes,
  /** Continue without saving. */
  No,
  /** Cancel the pending action and stay in the editor. */
  Cancel,
};

/**
 * @brief Result of one three-way save prompt UI action.
 */
struct SavePromptResult {
  /** True when the current choice changed. */
  bool selectionChanged = false;
  /** Choice confirmed by the user. */
  bool confirmed = false;
  /** Confirmed choice when `confirmed` is true. */
  SavePromptChoice choice = SavePromptChoice::No;
};

/**
 * @brief Simple `Da / Nu / Renunta` controller for unsaved manual edits.
 */
class SavePromptController {
 public:
  /**
   * @brief Resets the selector to the middle `Nu` answer.
   */
  void reset() { selectedIndex_ = 1U; }

  /**
   * @brief Handles one encoder step across the three choices.
   *
   * @param delta Positive for clockwise, negative for counter-clockwise.
   * @return Result indicating whether selection changed.
   */
  SavePromptResult onRotate(int8_t delta) {
    if (delta == 0) {
      return {};
    }

    const uint8_t previous = selectedIndex_;
    if (delta > 0 && selectedIndex_ < 2U) {
      selectedIndex_++;
    } else if (delta < 0 && selectedIndex_ > 0U) {
      selectedIndex_--;
    }

    SavePromptResult result;
    result.selectionChanged = previous != selectedIndex_;
    result.choice = currentChoice();
    return result;
  }

  /**
   * @brief Confirms the current choice.
   */
  SavePromptResult onShortPress() const {
    SavePromptResult result;
    result.confirmed = true;
    result.choice = currentChoice();
    return result;
  }

  /**
   * @brief Returns the currently highlighted choice.
   */
  SavePromptChoice currentChoice() const {
    if (selectedIndex_ == 0U) {
      return SavePromptChoice::Yes;
    }
    if (selectedIndex_ == 1U) {
      return SavePromptChoice::No;
    }
    return SavePromptChoice::Cancel;
  }

 private:
  uint8_t selectedIndex_ = 1U;
};

}  // namespace dehydrator
