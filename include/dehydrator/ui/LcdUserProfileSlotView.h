#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/persistence/UserProfileStore.h"
#include "dehydrator/ui/LcdStatusView.h"

namespace dehydrator {

/**
 * @brief Snapshot rendered by the user-profile slot list.
 */
struct LcdUserProfileSlotSnapshot {
  /** Title shown on the top line. */
  const char* title = "Programe utilizator";
  /** Slot records to display. */
  const UserProfileSlotRecord* slots = nullptr;
  /** Currently selected index, or BACK_INDEX for `Inapoi`. */
  uint8_t selectedIndex = 0U;
};

/**
 * @brief Renders one scrollable 10-slot user-profile list.
 */
class LcdUserProfileSlotView {
 public:
  /**
   * @brief Creates the slot-list view.
   *
   * @param display LCD character display interface.
   */
  explicit LcdUserProfileSlotView(CharacterDisplay& display) : display_(display) {}

  /**
   * @brief Renders the current visible window around the selected slot.
   *
   * @param snapshot Slot records, selection, and title.
   */
  void render(const LcdUserProfileSlotSnapshot& snapshot) {
    char line[LcdStatusView::COLUMNS + 1U] = {};

    fillLine(line);
    writeToken(line, snapshot.title, 0U);
    writeLine(0U, line);

    const uint8_t firstVisible = snapshot.selectedIndex;
    for (uint8_t row = 0U; row < 3U; row++) {
      const uint8_t entryIndex = static_cast<uint8_t>(firstVisible + row);
      fillLine(line);
      if (entryIndex <= UserProfileStore::SLOT_COUNT) {
        line[0] = entryIndex == snapshot.selectedIndex ? '>' : ' ';
        if (entryIndex == UserProfileStore::SLOT_COUNT) {
          writeToken(line, "Inapoi", 1U);
        } else {
          formatSlot(line, entryIndex, snapshot.slots);
        }
      }
      writeLine(static_cast<uint8_t>(row + 1U), line);
    }
  }

 private:
  static void formatSlot(char* line, uint8_t slotIndex,
                         const UserProfileSlotRecord* slots) {
    writeProfileToken(line, slotIndex);
    if (slots == nullptr || !slots[slotIndex].occupied) {
      writeToken(line, " (nedef.)", 9U);
    }
  }

  static void writeProfileToken(char* line, uint8_t slotIndex) {
    line[1U] = 'P';
    line[2U] = 'r';
    line[3U] = 'o';
    line[4U] = 'f';
    line[5U] = 'i';
    line[6U] = 'l';
    line[7U] = ' ';

    uint8_t column = 8U;
    const uint8_t number = static_cast<uint8_t>(slotIndex + 1U);
    if (number >= 10U) {
      line[column++] = '1';
      line[column++] = '0';
      return;
    }

    line[column] = static_cast<char>('0' + number);
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

  void writeLine(uint8_t row, const char* line) {
    display_.setCursor(0U, row);
    for (uint8_t column = 0U; column < LcdStatusView::COLUMNS; column++) {
      display_.writeChar(line[column]);
    }
  }

  CharacterDisplay& display_;
};

}  // namespace dehydrator
