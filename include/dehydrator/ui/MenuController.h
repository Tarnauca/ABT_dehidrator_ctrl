#pragma once

#include <stddef.h>
#include <stdint.h>

namespace dehydrator {

/**
 * @brief High-level screen selection for the LCD bring-up UI.
 */
enum class UiScreen {
  /** Show the status screen with temperatures and output states. */
  Status,
  /** Show the simple navigation menu. */
  Menu,
};

/**
 * @brief UI action produced by a button or encoder event.
 */
enum class UiAction {
  /** No externally-visible UI transition occurred. */
  None,
  /** Menu screen was opened from status. */
  OpenMenu,
  /** Menu screen was closed back to status. */
  CloseMenu,
  /** Menu selection changed. */
  MoveSelection,
  /** Current menu item was selected. */
  SelectItem,
};

/**
 * @brief Result returned after handling one UI input event.
 */
struct UiResult {
  /** High-level action for logging or follow-up behavior. */
  UiAction action = UiAction::None;
  /** Selected item label when `action` is `SelectItem`. */
  const char* selectedItem = nullptr;

  /**
   * @brief Creates a UI result with optional action and item payload.
   *
   * @param actionValue Action reported by the controller.
   * @param selectedItemValue Selected item label, when relevant.
   */
  constexpr UiResult(UiAction actionValue = UiAction::None,
                     const char* selectedItemValue = nullptr)
      : action(actionValue), selectedItem(selectedItemValue) {}
};

/**
 * @brief Minimal encoder-driven menu controller for UI bring-up.
 *
 * This controller intentionally avoids business logic. It only owns the active
 * screen and current menu selection so we can verify encoder/menu behavior on
 * real hardware before wiring the full dehydrator workflows.
 */
class MenuController {
 public:
  /**
   * @brief Number of built-in bring-up menu items.
   */
  static constexpr size_t ITEM_COUNT = 5U;

  /**
   * @brief Creates a menu controller starting on the status screen.
   */
  constexpr MenuController() = default;

  /**
   * @brief Opens the menu or selects the current menu item.
   *
   * @return UI result describing the transition or selection.
   */
  UiResult onShortPress() {
    if (screen_ == UiScreen::Status) {
      screen_ = UiScreen::Menu;
      return {UiAction::OpenMenu, nullptr};
    }

    return {UiAction::SelectItem, currentItem()};
  }

  /**
   * @brief Closes the menu when active.
   *
   * @return UI result describing whether the screen changed.
   */
  UiResult onLongPress() {
    if (screen_ == UiScreen::Menu) {
      screen_ = UiScreen::Status;
      return {UiAction::CloseMenu, nullptr};
    }

    return {};
  }

  /**
   * @brief Applies one encoder step when the menu is active.
   *
   * @param delta Positive for clockwise, negative for counter-clockwise.
   * @return UI result describing whether selection changed.
   */
  UiResult onRotate(int8_t delta) {
    if (screen_ != UiScreen::Menu || delta == 0) {
      return {};
    }

    const int8_t direction = delta > 0 ? 1 : -1;
    size_t newIndex = selectedIndex_;
    if (direction > 0) {
      newIndex = (selectedIndex_ + 1U) % ITEM_COUNT;
    } else if (selectedIndex_ == 0U) {
      newIndex = ITEM_COUNT - 1U;
    } else {
      newIndex = selectedIndex_ - 1U;
    }

    if (newIndex == selectedIndex_) {
      return {};
    }

    selectedIndex_ = newIndex;
    return {UiAction::MoveSelection, nullptr};
  }

  /**
   * @brief Returns the currently active screen.
   *
   * @return `UiScreen::Status` or `UiScreen::Menu`.
   */
  UiScreen screen() const { return screen_; }

  /**
   * @brief Returns the current selected menu item label.
   *
   * @return Stable Romanian item label.
   */
  const char* currentItem() const { return items()[selectedIndex_]; }

  /**
   * @brief Returns the current selected menu item token for logs.
   *
   * @return Stable ASCII token without spaces.
   */
  const char* currentToken() const { return tokens()[selectedIndex_]; }

  /**
   * @brief Returns the selected menu index.
   *
   * @return Zero-based selected item index.
   */
  size_t selectedIndex() const { return selectedIndex_; }

  /**
   * @brief Returns the built-in menu item labels.
   *
   * @return Pointer to the fixed menu item array.
   */
  static const char* const* items() {
    static const char* const kItems[ITEM_COUNT] = {
        "Pornire preset",
        "Mod manual",
        "Setari",
        "Reluare program",
        "Oprire",
    };
    return kItems;
  }

  /**
   * @brief Returns stable log tokens for the built-in menu items.
   *
   * @return Pointer to the fixed token array.
   */
  static const char* const* tokens() {
    static const char* const kTokens[ITEM_COUNT] = {
        "pornire_preset",
        "mod_manual",
        "setari",
        "reluare_program",
        "oprire",
    };
    return kTokens;
  }

 private:
  UiScreen screen_ = UiScreen::Status;
  size_t selectedIndex_ = 0U;
};

}  // namespace dehydrator
