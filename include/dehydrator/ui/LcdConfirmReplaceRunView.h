#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/ui/LcdStatusView.h"

namespace dehydrator {

/**
 * @brief Snapshot rendered by the replace-run confirmation screen.
 */
struct LcdConfirmReplaceRunSnapshot {
  /** Whether `Da` is currently selected. */
  bool confirmSelected = false;
  /** Whether the heartbeat custom symbol should be visible. */
  bool heartbeatOn = false;
};

/**
 * @brief Renders a simple Romanian yes/no confirmation for replacing a run.
 */
class LcdConfirmReplaceRunView {
 public:
  /**
   * @brief Creates the confirmation view for the provided display.
   *
   * @param display LCD character display interface.
   */
  explicit LcdConfirmReplaceRunView(CharacterDisplay& display)
      : display_(display) {}

  /**
   * @brief Renders the replace-run confirmation screen.
   *
   * @param snapshot Current yes/no selection and heartbeat state.
   */
  void render(const LcdConfirmReplaceRunSnapshot& snapshot) {
    char line[LcdStatusView::COLUMNS + 1U] = {};

    fillLine(line);
    writeToken(line, "Confirmare", 0U);
    writeLine(0U, line, false);

    fillLine(line);
    writeToken(line, "Inlocuire program?", 0U);
    writeLine(1U, line, false);

    fillLine(line);
    writeToken(line, snapshot.confirmSelected ? " Nu   >Da" : ">Nu    Da", 0U);
    writeLine(2U, line, false);

    fillLine(line);
    writeLine(3U, line, snapshot.heartbeatOn);
  }

 private:
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
