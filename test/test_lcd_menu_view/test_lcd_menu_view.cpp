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

LcdMenuSnapshot snapshotForIndex(size_t selectedIndex,
                                 bool showStop = false,
                                 bool showPause = false,
                                 bool showResume = false,
                                 const char* title = "Meniu") {
  static const char* labels[MenuController::MAX_ITEM_COUNT] = {};
  MenuController controller;
  dehydrator::MainMenuContext context;
  context.showStopProgram = showStop;
  context.showPauseProgram = showPause;
  context.showResumeProgram = showResume;
  controller.setContext(context);
  controller.fillVisibleItems(labels);
  LcdMenuSnapshot snapshot;
  snapshot.title = title;
  snapshot.items = labels;
  snapshot.itemCount = controller.itemCount();
  snapshot.selectedIndex = selectedIndex;
  return snapshot;
}

void test_menu_view_renders_first_items_and_hint() {
  FakeDisplay display;
  LcdMenuView view(display);

  view.render(snapshotForIndex(0U));

  assertLineEquals(display, 0U, "Meniu               ");
  assertLineEquals(display, 1U, ">Programe presetate ");
  assertLineEquals(display, 2U, " Programe utilizator");
  assertLineEquals(display, 3U, " Program manual     ");
}

void test_menu_view_scrolls_when_selection_moves_down() {
  FakeDisplay display;
  LcdMenuView view(display);

  view.render(snapshotForIndex(3U));

  assertLineEquals(display, 0U, "Meniu               ");
  assertLineEquals(display, 1U, ">Setari             ");
  assertLineEquals(display, 2U, " Inapoi             ");
  assertLineEquals(display, 3U, "                    ");
}

void test_menu_view_keeps_last_item_on_first_line() {
  FakeDisplay display;
  LcdMenuView view(display);

  view.render(snapshotForIndex(4U));

  assertLineEquals(display, 0U, "Meniu               ");
  assertLineEquals(display, 1U, ">Inapoi             ");
  assertLineEquals(display, 2U, "                    ");
  assertLineEquals(display, 3U, "                    ");
}

void test_menu_view_renders_dynamic_stop_and_resume_entries() {
  FakeDisplay display;
  LcdMenuView view(display);

  view.render(snapshotForIndex(0U, true, true, true));

  assertLineEquals(display, 0U, "Meniu               ");
  assertLineEquals(display, 1U, ">Oprire program     ");
  assertLineEquals(display, 2U, " Pauza program      ");
  assertLineEquals(display, 3U, " Reluare program    ");
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_menu_view_renders_first_items_and_hint);
  RUN_TEST(test_menu_view_scrolls_when_selection_moves_down);
  RUN_TEST(test_menu_view_keeps_last_item_on_first_line);
  RUN_TEST(test_menu_view_renders_dynamic_stop_and_resume_entries);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
