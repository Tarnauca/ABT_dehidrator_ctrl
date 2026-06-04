#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dehydrator/domain/ProfileEngine.h"
#include "dehydrator/presets/PresetCatalog.h"

namespace dehydrator {

/**
 * @brief Result of one preset selection UI action.
 */
struct PresetUiResult {
  /** True when the selected preset changed. */
  bool selectionChanged = false;
  /** True when the selected preset was confirmed. */
  bool presetSelected = false;
  /** True when the screen should close back to the menu. */
  bool exitToMenu = false;
};

/**
 * @brief Minimal preset selection controller for the initial run setup flow.
 */
class PresetSelectController {
 public:
  /**
   * @brief Returns the currently selected preset index.
   *
   * @return Zero-based preset index.
   */
  size_t selectedIndex() const { return selectedIndex_; }

  /**
   * @brief Returns the currently selected preset.
   *
   * @return Pointer to the preset definition.
   */
  const PresetDefinition* currentPreset() const {
    return &PresetCatalog::items()[selectedIndex_];
  }

  /**
   * @brief Handles one encoder step.
   *
   * @param delta Positive for clockwise, negative for counter-clockwise.
   * @return Result indicating whether selection changed.
   */
  PresetUiResult onRotate(int8_t delta) {
    if (delta == 0) {
      return {};
    }

    if (delta > 0) {
      if (selectedIndex_ + 1U >= PresetCatalog::PRESET_COUNT) {
        return {};
      }
      selectedIndex_++;
    } else {
      if (selectedIndex_ == 0U) {
        return {};
      }
      selectedIndex_--;
    }

    PresetUiResult result;
    result.selectionChanged = true;
    return result;
  }

  /**
   * @brief Handles a short press by confirming the preset.
   *
   * @return Result indicating that a preset was selected.
   */
  PresetUiResult onShortPress() {
    PresetUiResult result;
    result.presetSelected = true;
    return result;
  }

  /**
   * @brief Handles a long press by returning to the menu.
   *
   * @return Result requesting a return to the menu.
   */
  PresetUiResult onLongPress() {
    PresetUiResult result;
    result.exitToMenu = true;
    return result;
  }

 private:
  size_t selectedIndex_ = 0U;
};

}  // namespace dehydrator
