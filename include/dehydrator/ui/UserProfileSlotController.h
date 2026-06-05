#pragma once

#include <stdint.h>

#include "dehydrator/persistence/UserProfileStore.h"

namespace dehydrator {

/**
 * @brief Result of one user-profile slot list action.
 */
struct UserProfileSlotResult {
  /** True when the selected slot changed. */
  bool selectionChanged = false;
  /** True when the current slot was confirmed. */
  bool slotSelected = false;
  /** True when the synthetic back entry was confirmed. */
  bool exitRequested = false;
};

/**
 * @brief Controller for the 10-slot user-profile list plus `Inapoi`.
 */
class UserProfileSlotController {
 public:
  /** Index of the synthetic `Inapoi` entry. */
  static constexpr uint8_t BACK_INDEX = UserProfileStore::SLOT_COUNT;

  /**
   * @brief Resets selection to the first slot.
   */
  void reset() { selectedIndex_ = 0U; }

  /**
   * @brief Sets the selected slot index when it is within range.
   *
   * @param selectedIndex Zero-based slot index.
   */
  void setSelectedSlot(uint8_t selectedIndex) {
    if (selectedIndex < UserProfileStore::SLOT_COUNT) {
      selectedIndex_ = selectedIndex;
    }
  }

  /**
   * @brief Returns the current selection index.
   */
  uint8_t selectedIndex() const { return selectedIndex_; }

  /**
   * @brief Returns whether the current entry is the synthetic back item.
   */
  bool currentIsBack() const { return selectedIndex_ == BACK_INDEX; }

  /**
   * @brief Returns the current slot index when one real slot is selected.
   */
  uint8_t currentSlot() const {
    return selectedIndex_ < UserProfileStore::SLOT_COUNT ? selectedIndex_ : 0U;
  }

  /**
   * @brief Handles one encoder step within the slot list.
   *
   * @param delta Positive for clockwise, negative for counter-clockwise.
   * @return Result indicating whether selection changed.
   */
  UserProfileSlotResult onRotate(int8_t delta) {
    if (delta == 0) {
      return {};
    }

    const uint8_t previous = selectedIndex_;
    if (delta > 0 && selectedIndex_ < BACK_INDEX) {
      selectedIndex_++;
    } else if (delta < 0 && selectedIndex_ > 0U) {
      selectedIndex_--;
    }

    UserProfileSlotResult result;
    result.selectionChanged = previous != selectedIndex_;
    return result;
  }

  /**
   * @brief Confirms the currently selected slot or `Inapoi`.
   */
  UserProfileSlotResult onShortPress() const {
    UserProfileSlotResult result;
    result.exitRequested = currentIsBack();
    result.slotSelected = !result.exitRequested;
    return result;
  }

 private:
  uint8_t selectedIndex_ = 0U;
};

}  // namespace dehydrator
