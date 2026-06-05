#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Available actions on one saved or vacant user-profile slot.
 */
enum class UserProfileAction {
  /** Start the saved profile. */
  Start,
  /** Open the profile editor. */
  Edit,
  /** Delete the saved profile from the slot. */
  Delete,
  /** Return to the previous menu level. */
  Back,
};

/**
 * @brief Result of one user-profile detail screen action.
 */
struct UserProfileActionResult {
  /** True when the selected action changed. */
  bool selectionChanged = false;
  /** True when the user selected `Pornire`. */
  bool startRequested = false;
  /** True when the user selected `Editeaza`. */
  bool editRequested = false;
  /** True when the user selected `Sterge`. */
  bool deleteRequested = false;
  /** True when the user selected `Inapoi`. */
  bool exitRequested = false;
};

/**
 * @brief Action selector for one user-profile detail screen.
 */
class UserProfileActionController {
 public:
  /**
   * @brief Configures whether the selected slot currently contains a profile.
   *
   * Occupied slots expose `Pornire / Editeaza / Sterge / Inapoi`. Vacant slots
   * expose `Editeaza / Inapoi`.
   *
   * @param occupied True when the selected slot contains a saved profile.
   */
  void setOccupied(bool occupied) {
    occupied_ = occupied;
    if (selectedIndex_ >= actionCount()) {
      selectedIndex_ = 0U;
    }
  }

  /**
   * @brief Resets selection to the first available action.
   */
  void reset() { selectedIndex_ = 0U; }

  /**
   * @brief Returns the selected action.
   */
  UserProfileAction currentAction() const {
    if (occupied_) {
      static constexpr UserProfileAction occupiedActions[4] = {
          UserProfileAction::Start, UserProfileAction::Edit,
          UserProfileAction::Delete, UserProfileAction::Back};
      return occupiedActions[selectedIndex_];
    }

    static constexpr UserProfileAction vacantActions[2] = {
        UserProfileAction::Edit, UserProfileAction::Back};
    return vacantActions[selectedIndex_];
  }

  /**
   * @brief Handles one encoder step across available actions.
   *
   * @param delta Positive for clockwise, negative for counter-clockwise.
   * @return Result indicating whether selection changed.
   */
  UserProfileActionResult onRotate(int8_t delta) {
    if (delta == 0) {
      return {};
    }

    const uint8_t previous = selectedIndex_;
    if (delta > 0 && selectedIndex_ + 1U < actionCount()) {
      selectedIndex_++;
    } else if (delta < 0 && selectedIndex_ > 0U) {
      selectedIndex_--;
    }

    UserProfileActionResult result;
    result.selectionChanged = previous != selectedIndex_;
    return result;
  }

  /**
   * @brief Confirms the currently selected action.
   */
  UserProfileActionResult onShortPress() const {
    UserProfileActionResult result;
    switch (currentAction()) {
      case UserProfileAction::Start:
        result.startRequested = true;
        break;
      case UserProfileAction::Edit:
        result.editRequested = true;
        break;
      case UserProfileAction::Delete:
        result.deleteRequested = true;
        break;
      case UserProfileAction::Back:
        result.exitRequested = true;
        break;
    }
    return result;
  }

 private:
  uint8_t actionCount() const { return occupied_ ? 4U : 2U; }

  bool occupied_ = false;
  uint8_t selectedIndex_ = 0U;
};

}  // namespace dehydrator
