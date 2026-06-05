#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/ui/LcdStatusView.h"
#include "dehydrator/ui/NtcCalibrationController.h"

namespace dehydrator {

/**
 * @brief Snapshot rendered by the NTC calibration editor.
 */
struct LcdNtcCalibrationSnapshot {
  /** Selected field. */
  NtcCalibrationField selectedField = NtcCalibrationField::Offset;
  /** Selected row index. */
  uint8_t selectedIndex = 0U;
  /** Whether the selected field is being edited. */
  bool editing = false;
  /** Current editable offset in centi-Celsius. */
  int16_t offsetCentiC = 0;
  /** Current editable scale in ppm. */
  int32_t scalePpm = 1000000;
};

/**
 * @brief Renders the 4x20 Romanian NTC calibration editor.
 */
class LcdNtcCalibrationView {
 public:
  /**
   * @brief Creates a calibration view for the provided display.
   *
   * @param display Character display abstraction used for rendering.
   */
  explicit LcdNtcCalibrationView(CharacterDisplay& display) : display_(display) {}

  /**
   * @brief Renders the current NTC calibration editor state.
   *
   * @param snapshot Values and selection state to display.
   */
  void render(const LcdNtcCalibrationSnapshot& snapshot) {
    char line[LcdStatusView::COLUMNS + 1U] = {};

    fillLine(line);
    writeToken(line, "Calibrare NTC", 0U);
    writeLine(0U, line);

    for (uint8_t row = 0U; row < 3U; row++) {
      fillLine(line);
      const uint8_t fieldIndex =
          static_cast<uint8_t>(snapshot.selectedIndex + row);
      if (fieldIndex < NtcCalibrationController::FIELD_COUNT) {
        const NtcCalibrationField field = fieldAt(fieldIndex);
        line[0] = marker(snapshot, field);
        formatField(line, snapshot, field);
      }
      writeLine(static_cast<uint8_t>(row + 1U), line);
    }
  }

 private:
  static NtcCalibrationField fieldAt(uint8_t index) {
    switch (index) {
      case 0U:
        return NtcCalibrationField::Offset;
      case 1U:
        return NtcCalibrationField::Scale;
      case 2U:
        return NtcCalibrationField::Save;
      case 3U:
        return NtcCalibrationField::Restore;
      case 4U:
      default:
        return NtcCalibrationField::Back;
    }
  }

  static char marker(const LcdNtcCalibrationSnapshot& snapshot,
                     NtcCalibrationField field) {
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

  static void formatField(char* line, const LcdNtcCalibrationSnapshot& snapshot,
                          NtcCalibrationField field) {
    char value[20] = {};
    if (field == NtcCalibrationField::Offset) {
      const int16_t absOffset = snapshot.offsetCentiC >= 0
                                    ? snapshot.offsetCentiC
                                    : static_cast<int16_t>(-snapshot.offsetCentiC);
      snprintf(value, sizeof(value), "Offset:%c%d.%d\xDF""C",
               snapshot.offsetCentiC >= 0 ? '+' : '-', absOffset / 100,
               (absOffset % 100) / 10);
    } else if (field == NtcCalibrationField::Scale) {
      const int32_t absScale = snapshot.scalePpm >= 0 ? snapshot.scalePpm : 0;
      snprintf(value, sizeof(value), "Scala:%ld.%02ld",
               static_cast<long>(absScale / 1000000L),
               static_cast<long>((absScale % 1000000L) / 10000L));
    } else if (field == NtcCalibrationField::Save) {
      snprintf(value, sizeof(value), "Salveaza");
    } else if (field == NtcCalibrationField::Restore) {
      snprintf(value, sizeof(value), "Restabileste");
    } else {
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
