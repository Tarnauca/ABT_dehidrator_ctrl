#include <unity.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/ui/LcdManualProgramView.h"

using dehydrator::CharacterDisplay;
using dehydrator::LcdManualProgramSnapshot;
using dehydrator::LcdManualProgramView;
using dehydrator::LcdStatusView;
using dehydrator::ManualProgramField;

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

void test_manual_program_view_renders_temperature_selected() {
  FakeDisplay display;
  LcdManualProgramView view(display);
  LcdManualProgramSnapshot snapshot;
  snapshot.selectedField = ManualProgramField::Temperature;
  snapshot.targetTempC = 57;
  snapshot.durationMinutes = 8U * 60U;
  snapshot.fluctuating = false;

  view.render(snapshot);

  assertLineEquals(display, 0U, "Mod manual          ");
  TEST_ASSERT_EQUAL_CHAR('>', display.cells[1U][0U]);
  TEST_ASSERT_EQUAL_CHAR('T', display.cells[1U][1U]);
  TEST_ASSERT_EQUAL_CHAR('e', display.cells[1U][2U]);
  TEST_ASSERT_EQUAL_CHAR('m', display.cells[1U][3U]);
  TEST_ASSERT_EQUAL_CHAR('p', display.cells[1U][4U]);
  TEST_ASSERT_EQUAL_CHAR(':', display.cells[1U][5U]);
  TEST_ASSERT_EQUAL_CHAR('5', display.cells[1U][6U]);
  TEST_ASSERT_EQUAL_CHAR('7', display.cells[1U][7U]);
  TEST_ASSERT_EQUAL_CHAR(static_cast<char>(0xDF), display.cells[1U][8U]);
  TEST_ASSERT_EQUAL_CHAR('C', display.cells[1U][9U]);
  assertLineEquals(display, 2U, " Dur:8h 0m          ");
}

void test_manual_program_view_renders_editing_marker_and_footer() {
  FakeDisplay display;
  LcdManualProgramView view(display);
  LcdManualProgramSnapshot snapshot;
  snapshot.selectedField = ManualProgramField::Fluctuating;
  snapshot.editing = true;
  snapshot.targetTempC = 60;
  snapshot.durationMinutes = 9U * 60U;
  snapshot.fluctuating = true;
  snapshot.heartbeatOn = true;

  view.render(snapshot);

  TEST_ASSERT_EQUAL_CHAR('*', display.cells[3U][0U]);
  TEST_ASSERT_EQUAL_CHAR('F', display.cells[3U][1U]);
  TEST_ASSERT_EQUAL_CHAR(':', display.cells[3U][2U]);
  TEST_ASSERT_EQUAL_CHAR('D', display.cells[3U][3U]);
  TEST_ASSERT_EQUAL_CHAR('a', display.cells[3U][4U]);
  TEST_ASSERT_EQUAL_CHAR(' ', display.cells[3U][5U]);
  TEST_ASSERT_EQUAL_CHAR('S', display.cells[3U][6U]);
  TEST_ASSERT_EQUAL_CHAR('t', display.cells[3U][7U]);
  TEST_ASSERT_EQUAL_CHAR('a', display.cells[3U][8U]);
  TEST_ASSERT_EQUAL_CHAR('r', display.cells[3U][9U]);
  TEST_ASSERT_EQUAL_CHAR('t', display.cells[3U][10U]);
  TEST_ASSERT_EQUAL_CHAR(' ', display.cells[3U][11U]);
  TEST_ASSERT_EQUAL_CHAR('I', display.cells[3U][12U]);
  TEST_ASSERT_EQUAL_CHAR('n', display.cells[3U][13U]);
  TEST_ASSERT_EQUAL_CHAR('a', display.cells[3U][14U]);
  TEST_ASSERT_EQUAL_CHAR('p', display.cells[3U][15U]);
  TEST_ASSERT_EQUAL_CHAR('o', display.cells[3U][16U]);
  TEST_ASSERT_EQUAL_CHAR('i', display.cells[3U][17U]);
  TEST_ASSERT_TRUE(display.custom[3U][19U]);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_manual_program_view_renders_temperature_selected);
  RUN_TEST(test_manual_program_view_renders_editing_marker_and_footer);
  return UNITY_END();
}
