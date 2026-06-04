#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/presets/PresetCatalog.h"
#include "dehydrator/ui/LcdStatusView.h"

namespace dehydrator {

/**
 * @brief Snapshot rendered by the preset selection screen.
 */
struct LcdPresetSnapshot {
  /** Fixed array of preset definitions. */
  const PresetDefinition* presets = nullptr;
  /** Number of presets available in `presets`. */
  size_t presetCount = 0U;
  /** Currently selected preset index. */
  size_t selectedIndex = 0U;
  /** Whether the heartbeat custom symbol should be visible. */
  bool heartbeatOn = false;
};

/**
 * @brief Renders the 4x20 Romanian preset selection screen.
 */
class LcdPresetView {
 public:
  /**
   * @brief Creates a preset view for the provided character display.
   *
   * @param display LCD character display interface.
   */
  explicit LcdPresetView(CharacterDisplay& display) : display_(display) {}

  /**
   * @brief Renders the current preset selection.
   *
   * @param snapshot Current preset list and selected index.
   */
  void render(const LcdPresetSnapshot& snapshot) {
    char line[LcdStatusView::COLUMNS + 1U] = {};

    fillLine(line);
    writeToken(line, "Pornire preset", 0U);
    writeLine(0U, line, false);

    fillLine(line);
    line[0] = '>';
    writeToken(line, currentLabel(snapshot), 1U);
    writeLine(1U, line, false);

    fillLine(line);
    writeToken(line, currentDescription(snapshot), 0U);
    writeLine(2U, line, false);

    fillLine(line);
    writeToken(line, "Apas=OK Tine=Inap", 0U);
    writeLine(3U, line, snapshot.heartbeatOn);
  }

 private:
  static const PresetDefinition* currentPreset(const LcdPresetSnapshot& snapshot) {
    if (snapshot.presets == nullptr || snapshot.presetCount == 0U ||
        snapshot.selectedIndex >= snapshot.presetCount) {
      return nullptr;
    }

    return &snapshot.presets[snapshot.selectedIndex];
  }

  static const char* currentLabel(const LcdPresetSnapshot& snapshot) {
    const PresetDefinition* preset = currentPreset(snapshot);
    return preset != nullptr ? preset->label : "";
  }

  static const char* currentDescription(const LcdPresetSnapshot& snapshot) {
    const PresetDefinition* preset = currentPreset(snapshot);
    if (preset == nullptr) {
      return "";
    }

    return preset->profile.mode == ProfileMode::Fixed ? "Mod fix" : "Mod fluctuat";
  }

  static void fillLine(char* line) {
    for (uint8_t index = 0U; index < LcdStatusView::COLUMNS; index++) {
      line[index] = ' ';
    }
    line[LcdStatusView::COLUMNS] = '\0';
  }

  static void writeToken(char* line, const char* token, uint8_t column) {
    if (token == nullptr) {
      return;
    }

    uint8_t writeColumn = column;
    for (size_t index = 0U;
         token[index] != '\0' && writeColumn < LcdStatusView::COLUMNS; index++) {
      line[writeColumn] = token[index];
      writeColumn++;
    }
  }

  void writeLine(uint8_t row, const char* line, bool heartbeatOn) {
    display_.setCursor(0U, row);
    for (uint8_t column = 0U; column < LcdStatusView::COLUMNS; column++) {
      if (row == LcdStatusView::ROWS - 1U &&
          column == LcdStatusView::COLUMNS - 1U && heartbeatOn) {
        display_.writeCustom(LcdStatusView::HEARTBEAT_CHAR);
      } else {
        display_.writeChar(line[column]);
      }
    }
  }

  CharacterDisplay& display_;
};

}  // namespace dehydrator
