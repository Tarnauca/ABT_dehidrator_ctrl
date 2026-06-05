#pragma once

#include <stdint.h>
#include <stdio.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/ui/LcdStatusView.h"
#include "dehydrator/ui/SavePromptController.h"

namespace dehydrator {

/**
 * @brief Snapshot rendered by the three-way save prompt.
 */
struct LcdSavePromptSnapshot {
  /** Prompt title shown on the top line. */
  const char* title = "Salveaza profil?";
  /** Currently highlighted choice. */
  SavePromptChoice choice = SavePromptChoice::No;
  /** Whether the heartbeat custom symbol should be visible. */
  bool heartbeatOn = false;
};

/**
 * @brief Renders one compact `Da / Nu / Renunta` prompt on the 4x20 LCD.
 */
class LcdSavePromptView {
 public:
  /**
   * @brief Creates the save prompt view.
   *
   * @param display LCD character display interface.
   */
  explicit LcdSavePromptView(CharacterDisplay& display) : display_(display) {}

  /**
   * @brief Renders the current prompt title and selected choice.
   *
   * @param snapshot Prompt title, choice, and heartbeat state.
   */
  void render(const LcdSavePromptSnapshot& snapshot) {
    char line[LcdStatusView::COLUMNS + 1U] = {};

    fillLine(line);
    writeToken(line, snapshot.title, 0U);
    writeLine(0U, line, false);

    fillLine(line);
    writeToken(line, "Alege:", 0U);
    writeLine(1U, line, false);

    fillLine(line);
    line[0] = '>';
    writeToken(line, choiceLabel(snapshot.choice), 1U);
    writeLine(2U, line, false);

    fillLine(line);
    writeToken(line, "Roteste si apasa", 0U);
    writeLine(3U, line, snapshot.heartbeatOn);
  }

 private:
  static const char* choiceLabel(SavePromptChoice choice) {
    if (choice == SavePromptChoice::Yes) {
      return "Da";
    }
    if (choice == SavePromptChoice::Cancel) {
      return "Renunta";
    }
    return "Nu";
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
