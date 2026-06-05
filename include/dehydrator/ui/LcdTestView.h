#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/interfaces/OutputController.h"
#include "dehydrator/ui/LcdStatusView.h"
#include "dehydrator/ui/TestModeController.h"

namespace dehydrator {

/**
 * @brief Snapshot rendered by the direct-output test screen.
 */
struct LcdTestSnapshot {
  /** Selected editable field. */
  TestField selectedField = TestField::Fan;
  /** Current logical test output command. */
  OutputCommand command;
  /** Whether the heartbeat custom symbol should be visible. */
  bool heartbeatOn = false;
};

/**
 * @brief Renders the 4x20 Romanian direct-output test screen.
 */
class LcdTestView {
 public:
  /**
   * @brief Creates a test view for the provided character display.
   *
   * @param display LCD character display interface.
   */
  explicit LcdTestView(CharacterDisplay& display) : display_(display) {}

  /**
   * @brief Renders the current test mode state.
   *
   * @param snapshot Current selection and logical outputs.
   */
  void render(const LcdTestSnapshot& snapshot) {
    char line[LcdStatusView::COLUMNS + 1U] = {};

    fillLine(line);
    writeToken(line, "Testare", 0U);
    writeLine(0U, line, false);

    fillLine(line);
    line[0] = snapshot.selectedField == TestField::Fan ? '>' : ' ';
    writeToken(line, "Fan:", 1U);
    writeToken(line, snapshot.command.fanOn ? "ON" : "OFF", 6U);
    writeLine(1U, line, false);

    fillLine(line);
    line[0] = snapshot.selectedField == TestField::Heater ? '>' : ' ';
    writeToken(line, "Heat:", 1U);
    writeToken(line, snapshot.command.heaterOn ? "ON" : "OFF", 7U);
    writeLine(2U, line, false);

    fillLine(line);
    line[0] = snapshot.selectedField == TestField::Back ? '>' : ' ';
    writeToken(line, "Inapoi", 1U);
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
