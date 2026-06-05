#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Selectable entries in the settings submenu.
 */
enum class SettingsMenuItem {
  /** Open the direct-output diagnostic screen. */
  Testare,
  /** Return to the main menu. */
  Back,
};

/**
 * @brief Result of one settings-menu user action.
 */
struct SettingsMenuResult {
  /** True when the highlighted entry changed. */
  bool selectionChanged = false;
  /** True when the direct-output test screen should open. */
  bool openTest = false;
  /** True when the submenu should close back to the main menu. */
  bool exitToMainMenu = false;
};

/**
 * @brief Small two-entry submenu for product settings.
 */
class SettingsMenuController {
 public:
  /** Number of visible settings-menu items. */
  static constexpr uint8_t ITEM_COUNT = 2U;

  /**
   * @brief Resets selection to the first actionable entry.
   */
  void reset() { selectedIndex_ = 0U; }

  /**
   * @brief Returns the selected settings-menu item.
   */
  SettingsMenuItem currentItem() const {
    return selectedIndex_ == 0U ? SettingsMenuItem::Testare
                                : SettingsMenuItem::Back;
  }

  /**
   * @brief Returns the selected row index.
   */
  uint8_t selectedIndex() const { return selectedIndex_; }

  /**
   * @brief Returns the visible label at one index.
   *
   * @param index Zero-based visible entry index.
   * @return Romanian label or empty string when out of range.
   */
  static const char* itemLabel(uint8_t index) {
    static constexpr const char* kLabels[ITEM_COUNT] = {"Testare", "Inapoi"};
    return index < ITEM_COUNT ? kLabels[index] : "";
  }

  /**
   * @brief Copies visible labels into one caller-owned fixed array.
   *
   * @param labels Output array with at least `ITEM_COUNT` entries.
   */
  static void fillVisibleItems(const char* (&labels)[ITEM_COUNT]) {
    for (uint8_t index = 0U; index < ITEM_COUNT; index++) {
      labels[index] = itemLabel(index);
    }
  }

  /**
   * @brief Handles one encoder step in the submenu.
   *
   * @param delta Positive for clockwise, negative for counter-clockwise.
   * @return Result describing whether selection changed.
   */
  SettingsMenuResult onRotate(int8_t delta) {
    if (delta == 0) {
      return {};
    }

    const uint8_t previous = selectedIndex_;
    if (delta > 0 && selectedIndex_ + 1U < ITEM_COUNT) {
      selectedIndex_++;
    } else if (delta < 0 && selectedIndex_ > 0U) {
      selectedIndex_--;
    }

    SettingsMenuResult result;
    result.selectionChanged = previous != selectedIndex_;
    return result;
  }

  /**
   * @brief Confirms the current submenu entry.
   *
   * Selecting `Inapoi` also resets next entry to the first item.
   *
   * @return Result describing the requested action.
   */
  SettingsMenuResult onShortPress() {
    SettingsMenuResult result;
    if (currentItem() == SettingsMenuItem::Testare) {
      result.openTest = true;
    } else {
      reset();
      result.exitToMainMenu = true;
    }
    return result;
  }

 private:
  uint8_t selectedIndex_ = 0U;
};

}  // namespace dehydrator
