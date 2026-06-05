#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

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
 * @brief Runtime visibility flags for the main menu.
 */
struct MainMenuContext {
  /** Show `Oprire program` when one active/resumable run exists. */
  bool showStopProgram = false;
  /** Show `Reluare program` when one resumable run exists. */
  bool showResumeProgram = false;
};

/**
 * @brief Minimal encoder-driven main menu controller with dynamic entries.
 */
class MenuController {
 public:
  /** Maximum number of visible top-level menu items including `Inapoi`. */
  static constexpr size_t MAX_ITEM_COUNT = 7U;

  /**
   * @brief Creates a menu controller starting on the status screen.
   */
  constexpr MenuController() = default;

  /**
   * @brief Updates dynamic menu visibility flags.
   *
   * @param context Runtime context that decides which top-level entries appear.
   */
  void setContext(const MainMenuContext& context) {
    context_ = context;
    clampSelection();
  }

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

    if (strcmp(currentToken(), "inapoi") == 0) {
      screen_ = UiScreen::Status;
      return {UiAction::CloseMenu, nullptr};
    }

    return {UiAction::SelectItem, currentItem()};
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

    const size_t count = itemCount();
    size_t newIndex = selectedIndex_;
    if (delta > 0) {
      if (selectedIndex_ + 1U >= count) {
        return {};
      }
      newIndex = selectedIndex_ + 1U;
    } else {
      if (selectedIndex_ == 0U) {
        return {};
      }
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
   */
  UiScreen screen() const { return screen_; }

  /**
   * @brief Returns the current selected menu item label.
   */
  const char* currentItem() const { return itemAt(selectedIndex_)->label; }

  /**
   * @brief Returns the current selected menu item token for logs.
   */
  const char* currentToken() const { return itemAt(selectedIndex_)->token; }

  /**
   * @brief Returns the selected menu index.
   */
  size_t selectedIndex() const { return selectedIndex_; }

  /**
   * @brief Returns the current visible item count.
   */
  size_t itemCount() const {
    size_t count = 0U;
    for (size_t index = 0U; index < FULL_ITEM_COUNT; index++) {
      if (isVisible(allItems()[index])) {
        count++;
      }
    }
    return count;
  }

  /**
   * @brief Copies the current visible labels into one caller-owned array.
   *
   * @param labels Output label array of at least `MAX_ITEM_COUNT`.
   */
  void fillVisibleItems(const char* (&labels)[MAX_ITEM_COUNT]) const {
    size_t writeIndex = 0U;
    for (size_t index = 0U; index < FULL_ITEM_COUNT && writeIndex < MAX_ITEM_COUNT;
         index++) {
      if (isVisible(allItems()[index])) {
        labels[writeIndex++] = allItems()[index].label;
      }
    }
  }

  /**
   * @brief Forces the controller into the menu-screen mode.
   */
  void enterMenu() { screen_ = UiScreen::Menu; }

  /**
   * @brief Forces the controller back to the status-screen mode.
   */
  void returnToStatus() { screen_ = UiScreen::Status; }

 private:
  struct MenuItem {
    const char* label;
    const char* token;
    bool dynamicStop;
    bool dynamicResume;
  };

  static constexpr size_t FULL_ITEM_COUNT = 7U;

  static const MenuItem* allItems() {
    static const MenuItem kItems[FULL_ITEM_COUNT] = {
        {"Oprire program", "oprire_program", true, false},
        {"Reluare program", "reluare_program", false, true},
        {"Programe presetate", "programe_presetate", false, false},
        {"Programe utilizator", "programe_utilizator", false, false},
        {"Program manual", "program_manual", false, false},
        {"Setari", "setari", false, false},
        {"Inapoi", "inapoi", false, false},
    };
    return kItems;
  }

  bool isVisible(const MenuItem& item) const {
    if (item.dynamicStop) {
      return context_.showStopProgram;
    }
    if (item.dynamicResume) {
      return context_.showResumeProgram;
    }
    return true;
  }

  const MenuItem* itemAt(size_t visibleIndex) const {
    size_t currentVisible = 0U;
    for (size_t index = 0U; index < FULL_ITEM_COUNT; index++) {
      if (!isVisible(allItems()[index])) {
        continue;
      }
      if (currentVisible == visibleIndex) {
        return &allItems()[index];
      }
      currentVisible++;
    }
    return &allItems()[FULL_ITEM_COUNT - 1U];
  }

  void clampSelection() {
    const size_t count = itemCount();
    if (count == 0U) {
      selectedIndex_ = 0U;
      return;
    }
    if (selectedIndex_ >= count) {
      selectedIndex_ = count - 1U;
    }
  }

  UiScreen screen_ = UiScreen::Status;
  size_t selectedIndex_ = 0U;
  MainMenuContext context_;
};

}  // namespace dehydrator
