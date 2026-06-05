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
  /** Title shown on the first LCD row. */
  const char* title = "Meniu";
  /** Fixed array of menu item labels. */
  const char* const* items = nullptr;
  /** Number of items available in `items`. */
  size_t itemCount = 0U;
  /** Currently selected item index. */
  size_t selectedIndex = 0U;
};

/**
 * @brief Renders a simple Romanian menu on the 4x20 LCD.
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
   * @brief Renders the current menu section and selection.
   *
   * @param snapshot Current menu content and selected index.
   */
  void render(const LcdMenuSnapshot& snapshot) {
    char line[LcdStatusView::COLUMNS + 1U] = {};
    fillLine(line);
    writeToken(line, snapshot.title, 0U);
    writeLine(0U, line);

    const size_t firstVisible = firstVisibleIndex(snapshot);

    for (uint8_t row = 1U; row < 4U; row++) {
      fillLine(line);
      const size_t itemIndex = firstVisible + (row - 1U);
      if (itemIndex < snapshot.itemCount && snapshot.items != nullptr) {
        line[0] = itemIndex == snapshot.selectedIndex ? '>' : ' ';
        writeToken(line, snapshot.items[itemIndex], 1U);
      }
      writeLine(row, line);
    }
  }

 private:
  static size_t firstVisibleIndex(const LcdMenuSnapshot& snapshot) {
    if (snapshot.itemCount == 0U || snapshot.selectedIndex >= snapshot.itemCount) {
      return 0U;
    }

    return snapshot.selectedIndex;
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

  void writeLine(uint8_t row, const char* line) {
    display_.setCursor(0U, row);
    for (uint8_t column = 0U; column < LcdStatusView::COLUMNS; column++) {
      display_.writeChar(line[column]);
    }
  }

  CharacterDisplay& display_;
};

}  // namespace dehydrator
