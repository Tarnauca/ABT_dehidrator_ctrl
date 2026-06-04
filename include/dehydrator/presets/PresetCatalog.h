#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dehydrator/domain/ProfileEngine.h"

namespace dehydrator {

/**
 * @brief Built-in preset definition shown in the run configuration UI.
 */
struct PresetDefinition {
  /** Stable ASCII token for logs and tests. */
  const char* token = nullptr;
  /** Romanian display label for the LCD. */
  const char* label = nullptr;
  /** Drying profile used when the preset is started. */
  ProfileConfig profile;

  /**
   * @brief Creates a preset definition.
   *
   * @param tokenValue Stable ASCII token.
   * @param labelValue Romanian display label.
   * @param profileValue Profile configuration associated with the preset.
   */
  constexpr PresetDefinition(const char* tokenValue = nullptr,
                             const char* labelValue = nullptr,
                             const ProfileConfig& profileValue = ProfileConfig())
      : token(tokenValue), label(labelValue), profile(profileValue) {}
};

/**
 * @brief Fixed built-in preset catalog for the initial run-configuration flow.
 *
 * The values here are starter presets chosen to make the UI and run path
 * usable before a product manual or calibrated user-specific values arrive.
 * They are intentionally centralized so they can be revised later without
 * touching the UI flow.
 */
class PresetCatalog {
 public:
  /**
   * @brief Number of built-in starter presets.
   */
  static constexpr size_t PRESET_COUNT = 4U;

  /**
   * @brief Returns the built-in starter presets.
   *
   * @return Pointer to the fixed preset array.
   */
  static const PresetDefinition* items() {
    static const PresetDefinition kItems[PRESET_COUNT] = {
        PresetDefinition("mere",
                         "Mere",
                         ProfileConfig{
                             ProfileMode::Fluctuating, 57, 50, 65,
                             static_cast<uint16_t>(10U * 60U), 20U, 20U}),
        PresetDefinition("ierburi",
                         "Ierburi",
                         ProfileConfig{ProfileMode::Fixed, 40, 0, 0,
                                       static_cast<uint16_t>(6U * 60U), 0U,
                                       0U}),
        PresetDefinition("jerky",
                         "Jerky",
                         ProfileConfig{ProfileMode::Fixed, 65, 0, 0,
                                       static_cast<uint16_t>(8U * 60U), 0U,
                                       0U}),
        PresetDefinition("iaurt",
                         "Iaurt",
                         ProfileConfig{ProfileMode::Fixed, 42, 0, 0,
                                       static_cast<uint16_t>(8U * 60U), 0U,
                                       0U}),
    };
    return kItems;
  }
};

}  // namespace dehydrator
