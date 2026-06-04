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
  /** Currently selected field. */
  ManualProgramField selectedField = ManualProgramField::Temperature;
  /** Whether the selected field is being edited. */
  bool editing = false;
  /** Current target temperature. */
  int16_t targetTempC = 0;
  /** Current duration in minutes. */
  uint16_t durationMinutes = 0;
  /** Whether fluctuating mode is enabled. */
  bool fluctuating = false;
  /** Whether the heartbeat custom symbol should be visible. */
  bool heartbeatOn = false;
};

/**
 * @brief Renders the 4x20 Romanian manual program editor.
 */
class LcdManualProgramView {
 public:
  explicit LcdManualProgramView(CharacterDisplay& display) : display_(display) {}

  void render(const LcdManualProgramSnapshot& snapshot) {
    char line[LcdStatusView::COLUMNS + 1U] = {};

    fillLine(line);
    writeToken(line, "Mod manual", 0U);
    writeLine(0U, line, false);

    fillLine(line);
    line[0] = marker(snapshot, ManualProgramField::Temperature);
    formatTemperatureLine(line, snapshot.targetTempC);
    writeLine(1U, line, false);

    fillLine(line);
    line[0] = marker(snapshot, ManualProgramField::Duration);
    formatDurationLine(line, snapshot.durationMinutes);
    writeLine(2U, line, false);

    fillLine(line);
    if (snapshot.selectedField == ManualProgramField::Fluctuating) {
      line[0] = marker(snapshot, ManualProgramField::Fluctuating);
    } else if (snapshot.selectedField == ManualProgramField::Start) {
      line[5] = marker(snapshot, ManualProgramField::Start);
    } else if (snapshot.selectedField == ManualProgramField::Back) {
      line[11] = marker(snapshot, ManualProgramField::Back);
    }
    formatModeLine(line, snapshot.fluctuating);
    writeLine(3U, line, snapshot.heartbeatOn);
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

  static void formatTemperatureLine(char* line, int16_t targetTempC) {
    char value[16] = {};
    snprintf(value, sizeof(value), "Temp:%d\xDF""C", targetTempC);
    writeToken(line, value, 1U);
  }

  static void formatDurationLine(char* line, uint16_t durationMinutes) {
    char value[20] = {};
    snprintf(value, sizeof(value), "Dur:%uh %um", durationMinutes / 60U,
             durationMinutes % 60U);
    writeToken(line, value, 1U);
  }

  static void formatModeLine(char* line, bool fluctuating) {
    writeToken(line, fluctuating ? "F:Da" : "F:Nu", 1U);
    writeToken(line, "Start", 6U);
    writeToken(line, "Inapoi", 12U);
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
