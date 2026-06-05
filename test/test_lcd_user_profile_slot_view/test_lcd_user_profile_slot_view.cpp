#include <unity.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/ui/LcdStatusView.h"
#include "dehydrator/ui/LcdUserProfileSlotView.h"

using dehydrator::CharacterDisplay;
using dehydrator::LcdStatusView;
using dehydrator::LcdUserProfileSlotSnapshot;
using dehydrator::LcdUserProfileSlotView;
using dehydrator::ProfileConfig;
using dehydrator::ProfileMode;
using dehydrator::UserProfileSlotRecord;

class FakeDisplay : public CharacterDisplay {
 public:
  char cells[LcdStatusView::ROWS][LcdStatusView::COLUMNS] = {};
  bool custom[LcdStatusView::ROWS][LcdStatusView::COLUMNS] = {};
  uint8_t cursorColumn = 0U;
  uint8_t cursorRow = 0U;

  void setCursor(uint8_t column, uint8_t row) override {
    cursorColumn = column;
    cursorRow = row;
  }

  void writeChar(char value) override {
    if (cursorRow < LcdStatusView::ROWS &&
        cursorColumn < LcdStatusView::COLUMNS) {
      cells[cursorRow][cursorColumn] = value;
      custom[cursorRow][cursorColumn] = false;
    }
    cursorColumn++;
  }

  void writeCustom(uint8_t code) override {
    if (cursorRow < LcdStatusView::ROWS &&
        cursorColumn < LcdStatusView::COLUMNS) {
      cells[cursorRow][cursorColumn] = static_cast<char>(code);
      custom[cursorRow][cursorColumn] = true;
    }
    cursorColumn++;
  }
};

void assertLineEquals(const FakeDisplay& display, uint8_t row,
                      const char* expected) {
  for (uint8_t column = 0U; column < LcdStatusView::COLUMNS; column++) {
    TEST_ASSERT_EQUAL_CHAR(expected[column], display.cells[row][column]);
  }
}

void assertLinePrefixEquals(const FakeDisplay& display, uint8_t row,
                            const char* expected, uint8_t length) {
  for (uint8_t column = 0U; column < length; column++) {
    TEST_ASSERT_EQUAL_CHAR(expected[column], display.cells[row][column]);
  }
}

void test_slot_view_marks_vacant_profiles_as_undefined() {
  FakeDisplay display;
  LcdUserProfileSlotView view(display);
  UserProfileSlotRecord slots[dehydrator::UserProfileStore::SLOT_COUNT] = {};
  slots[1].occupied = true;
  slots[1].profile = ProfileConfig{ProfileMode::Boost, 55, 0, 65, 360, 30, 0};

  LcdUserProfileSlotSnapshot snapshot;
  snapshot.slots = slots;
  snapshot.selectedIndex = 0U;
  view.render(snapshot);

  assertLinePrefixEquals(display, 0U, "Programe utilizator", 19U);
  assertLineEquals(display, 1U, ">Profil 1 (nedef.)  ");
  assertLineEquals(display, 2U, " Profil 2           ");
  assertLineEquals(display, 3U, " Profil 3 (nedef.)  ");
}

void test_slot_view_keeps_back_entry_last() {
  FakeDisplay display;
  LcdUserProfileSlotView view(display);
  UserProfileSlotRecord slots[dehydrator::UserProfileStore::SLOT_COUNT] = {};

  LcdUserProfileSlotSnapshot snapshot;
  snapshot.slots = slots;
  snapshot.selectedIndex = dehydrator::UserProfileStore::SLOT_COUNT;
  view.render(snapshot);

  assertLineEquals(display, 1U, ">Inapoi             ");
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_slot_view_marks_vacant_profiles_as_undefined);
  RUN_TEST(test_slot_view_keeps_back_entry_last);
  return UNITY_END();
}
