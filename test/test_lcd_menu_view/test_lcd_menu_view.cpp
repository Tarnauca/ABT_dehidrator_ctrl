#include <unity.h>

#include "dehydrator/ui/LcdMenuView.h"
#include "dehydrator/ui/MenuController.h"

using dehydrator::CharacterDisplay;
using dehydrator::LcdMenuSnapshot;
using dehydrator::LcdMenuView;
using dehydrator::LcdStatusView;
using dehydrator::MenuController;

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

LcdMenuSnapshot snapshotForIndex(size_t selectedIndex, bool heartbeatOn) {
  LcdMenuSnapshot snapshot;
  snapshot.items = MenuController::items();
  snapshot.itemCount = MenuController::ITEM_COUNT;
  snapshot.selectedIndex = selectedIndex;
  snapshot.heartbeatOn = heartbeatOn;
  return snapshot;
}

void test_menu_view_renders_first_items_and_hint() {
  FakeDisplay display;
  LcdMenuView view(display);

  view.render(snapshotForIndex(0U, true));

  assertLineEquals(display, 0U, ">Pornire preset     ");
  assertLineEquals(display, 1U, " Mod manual         ");
  assertLineEquals(display, 2U, " Setari             ");
  assertLinePrefixEquals(display, 3U, "Apas=OK Tine=Inap", 17U);
  TEST_ASSERT_TRUE(display.custom[3U][19U]);
}

void test_menu_view_scrolls_when_selection_moves_down() {
  FakeDisplay display;
  LcdMenuView view(display);

  view.render(snapshotForIndex(3U, false));

  assertLineEquals(display, 0U, " Setari             ");
  assertLineEquals(display, 1U, ">Reluare program    ");
  assertLineEquals(display, 2U, " Oprire             ");
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_menu_view_renders_first_items_and_hint);
  RUN_TEST(test_menu_view_scrolls_when_selection_moves_down);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
