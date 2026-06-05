#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/ui/LcdStatusView.h"

namespace dehydrator {

/**
 * @brief Snapshot rendered by one yes/no confirmation view.
 */
struct LcdBinaryConfirmSnapshot {
  /** Title shown on the top line. */
  const char* title = "Confirmare";
  /** Prompt shown on the second line. */
  const char* prompt = "";
  /** Whether `Da` is currently selected. */
  bool confirmSelected = false;
  /** Whether the heartbeat custom symbol should be visible. */
  bool heartbeatOn = false;
};

/**
 * @brief Renders a compact yes/no confirmation screen.
 */
class LcdBinaryConfirmView {
 public:
  /**
   * @brief Creates the binary confirm view.
   *
   * @param display LCD character display interface.
   */
  explicit LcdBinaryConfirmView(CharacterDisplay& display) : display_(display) {}

  /**
   * @brief Renders one yes/no confirmation prompt.
   *
   * @param snapshot Title, prompt, selection, and heartbeat state.
   */
  void render(const LcdBinaryConfirmSnapshot& snapshot) {
    char line[LcdStatusView::COLUMNS + 1U] = {};

    fillLine(line);
    writeToken(line, snapshot.title, 0U);
    writeLine(0U, line, false);

    fillLine(line);
    writeToken(line, snapshot.prompt, 0U);
    writeLine(1U, line, false);

    fillLine(line);
    line[0] = '>';
    writeToken(line, snapshot.confirmSelected ? "Da" : "Nu", 1U);
    writeLine(2U, line, false);

    fillLine(line);
    writeToken(line, "Roteste si apasa", 0U);
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
    for (size_t index = 0U; token[index] != '\0' &&
                            column < LcdStatusView::COLUMNS;
         index++, column++) {
      line[column] = token[index];
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
