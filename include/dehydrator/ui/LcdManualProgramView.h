#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/ui/LcdStatusView.h"
#include "dehydrator/ui/ManualProgramController.h"

namespace dehydrator {

/**
 * @brief Snapshot rendered by the manual program editor screen.
 */
struct LcdManualProgramSnapshot {
  /** Selected manual program mode. */
  ManualProgramMode mode = ManualProgramMode::Constant;
  /** Currently selected field. */
  ManualProgramField selectedField = ManualProgramField::Mode;
  /** Current zero-based selection index in the dynamic field list. */
  uint8_t selectedIndex = 0U;
  /** Whether the selected field is being edited. */
  bool editing = false;
  /** Current base/reference temperature. */
  int16_t targetTempC = 0;
  /** Current total duration in minutes. */
  uint16_t durationMinutes = 0;
  /** Current boost delta above base temperature. */
  int16_t boostDeltaC = 0;
  /** Current boost duration in minutes. */
  uint16_t boostDurationMinutes = 0;
  /** Current fluctuating upper target. */
  int16_t upperTempC = 0;
  /** Current fluctuating lower target. */
  int16_t lowerTempC = 0;
  /** Current fluctuating upper phase duration. */
  uint16_t upperDurationMinutes = 0;
  /** Current fluctuating lower phase duration. */
  uint16_t lowerDurationMinutes = 0;
};

/**
 * @brief Renders the 4x20 Romanian manual program editor.
 */
class LcdManualProgramView {
 public:
  /**
   * @brief Creates a manual program view.
   *
   * @param display Character display abstraction used for rendering.
   */
  explicit LcdManualProgramView(CharacterDisplay& display) : display_(display) {}

  /**
   * @brief Renders the current manual program editor state.
   *
   * @param snapshot Values and selection state to display.
   */
  void render(const LcdManualProgramSnapshot& snapshot) {
    char line[LcdStatusView::COLUMNS + 1U] = {};

    fillLine(line);
    writeToken(line, "Program manual", 0U);
    writeLine(0U, line);

    const uint8_t count =
        ManualProgramController::fieldCountForMode(snapshot.mode);
    for (uint8_t visibleRow = 0U; visibleRow < 3U; visibleRow++) {
      fillLine(line);
      const uint8_t fieldIndex =
          static_cast<uint8_t>(snapshot.selectedIndex + visibleRow);
      if (fieldIndex < count) {
        const ManualProgramField field =
            ManualProgramController::fieldAtIndex(snapshot.mode, fieldIndex);
        line[0] = marker(snapshot, field);
        formatField(line, snapshot, field);
      }
      writeLine(static_cast<uint8_t>(visibleRow + 1U), line);
    }
  }

 private:
  static char marker(const LcdManualProgramSnapshot& snapshot,
                     ManualProgramField field) {
    if (snapshot.selectedField != field) {
      return ' ';
    }
    return snapshot.editing ? '*' : '>';
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

  static const char* modeLabel(ManualProgramMode mode) {
    if (mode == ManualProgramMode::Boost) {
      return "Boost";
    }
    if (mode == ManualProgramMode::Fluctuating) {
      return "Fluctuant";
    }
    return "Constant";
  }

  static void formatWholeTemperature(char* buffer, size_t bufferSize,
                                     int16_t tempC) {
    snprintf(buffer, bufferSize, "%d.0\xDF""C", static_cast<int>(tempC));
  }

  static void formatField(char* line, const LcdManualProgramSnapshot& snapshot,
                          ManualProgramField field) {
    char value[20] = {};
    if (field == ManualProgramField::Mode) {
      snprintf(value, sizeof(value), "Mod:%s", modeLabel(snapshot.mode));
    } else if (field == ManualProgramField::Temperature) {
      char tempValue[12] = {};
      formatWholeTemperature(tempValue, sizeof(tempValue), snapshot.targetTempC);
      snprintf(value, sizeof(value), "Temp:%s", tempValue);
    } else if (field == ManualProgramField::Duration) {
      snprintf(value, sizeof(value), "Dur:%uh %um",
               snapshot.durationMinutes / 60U,
               snapshot.durationMinutes % 60U);
    } else if (field == ManualProgramField::BoostDelta) {
      char tempValue[12] = {};
      formatWholeTemperature(tempValue, sizeof(tempValue), snapshot.boostDeltaC);
      snprintf(value, sizeof(value), "Boost:+%s", tempValue);
    } else if (field == ManualProgramField::BoostDuration) {
      snprintf(value, sizeof(value), "DurBoost:%um",
               snapshot.boostDurationMinutes);
    } else if (field == ManualProgramField::UpperTemp) {
      char tempValue[12] = {};
      formatWholeTemperature(tempValue, sizeof(tempValue), snapshot.upperTempC);
      snprintf(value, sizeof(value), "Tsup:%s", tempValue);
    } else if (field == ManualProgramField::LowerTemp) {
      char tempValue[12] = {};
      formatWholeTemperature(tempValue, sizeof(tempValue), snapshot.lowerTempC);
      snprintf(value, sizeof(value), "Tinf:%s", tempValue);
    } else if (field == ManualProgramField::UpperDuration) {
      snprintf(value, sizeof(value), "Dur Tsup:%um",
               snapshot.upperDurationMinutes);
    } else if (field == ManualProgramField::LowerDuration) {
      snprintf(value, sizeof(value), "Dur Tinf:%um",
               snapshot.lowerDurationMinutes);
    } else if (field == ManualProgramField::Start) {
      snprintf(value, sizeof(value), "Start");
    } else if (field == ManualProgramField::Save) {
      snprintf(value, sizeof(value), "Salveaza");
    } else if (field == ManualProgramField::Back) {
      snprintf(value, sizeof(value), "Inapoi");
    }
    writeToken(line, value, 1U);
  }

  void writeLine(uint8_t row, const char* line) {
    display_.setCursor(0U, row);
    for (uint8_t column = 0U; column < LcdStatusView::COLUMNS; column++) {
      display_.writeChar(line[column]);
    }
  }

  CharacterDisplay& display_;
};

}  // namespace dehydrator
