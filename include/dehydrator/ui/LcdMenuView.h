#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/ui/LcdStatusView.h"

namespace dehydrator {

/**
 * @brief Snapshot rendered by the Romanian 4x20 bring-up menu.
 */
struct LcdMenuSnapshot {
  /** Fixed array of menu item labels. */
  const char* const* items = nullptr;
  /** Number of items available in `items`. */
  size_t itemCount = 0U;
  /** Currently selected item index. */
  size_t selectedIndex = 0U;
  /** Whether the heartbeat custom symbol should be visible. */
  bool heartbeatOn = false;
};

/**
 * @brief Renders a simple scrolling Romanian menu on the 4x20 LCD.
 */
class LcdMenuView {
 public:
  /**
   * @brief Creates a menu view for the provided character display.
   *
   * @param display LCD character display interface.
   */
  explicit LcdMenuView(CharacterDisplay& display) : display_(display) {}

  /**
   * @brief Renders the current menu selection and navigation hint.
   *
   * @param snapshot Current menu content and selected index.
   */
  void render(const LcdMenuSnapshot& snapshot) {
    char line[LcdStatusView::COLUMNS + 1U] = {};
    const size_t firstVisible = firstVisibleIndex(snapshot);

    for (uint8_t row = 0U; row < 3U; row++) {
      fillLine(line);
      const size_t itemIndex = firstVisible + row;
      if (itemIndex < snapshot.itemCount && snapshot.items != nullptr) {
        line[0] = itemIndex == snapshot.selectedIndex ? '>' : ' ';
        writeToken(line, snapshot.items[itemIndex], 1U);
      }
      writeLine(row, line, false);
    }

    fillLine(line);
    writeToken(line, "Apas=OK Tine=Inap", 0U);
    writeLine(3U, line, snapshot.heartbeatOn);
  }

 private:
  static size_t firstVisibleIndex(const LcdMenuSnapshot& snapshot) {
    if (snapshot.itemCount <= 3U || snapshot.selectedIndex < 2U) {
      return 0U;
    }

    if (snapshot.selectedIndex >= snapshot.itemCount - 1U) {
      return snapshot.itemCount - 3U;
    }

    return snapshot.selectedIndex - 1U;
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
