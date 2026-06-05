#include <unity.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/ui/LcdManualProgramView.h"

using dehydrator::CharacterDisplay;
using dehydrator::LcdManualProgramSnapshot;
using dehydrator::LcdManualProgramView;
using dehydrator::LcdStatusView;
using dehydrator::ManualProgramField;
using dehydrator::ManualProgramMode;

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

LcdManualProgramSnapshot baseSnapshot() {
  LcdManualProgramSnapshot snapshot;
  snapshot.mode = ManualProgramMode::Constant;
  snapshot.selectedField = ManualProgramField::Mode;
  snapshot.selectedIndex = 0U;
  snapshot.targetTempC = 57;
  snapshot.durationMinutes = 8U * 60U;
  snapshot.boostDeltaC = 10;
  snapshot.boostDurationMinutes = 30;
  snapshot.upperTempC = 62;
  snapshot.lowerTempC = 52;
  snapshot.upperDurationMinutes = 10;
  snapshot.lowerDurationMinutes = 10;
  return snapshot;
}

void test_manual_program_view_renders_mode_selector_first() {
  FakeDisplay display;
  LcdManualProgramView view(display);

  view.render(baseSnapshot());

  assertLineEquals(display, 0U, "Program manual      ");
  assertLineEquals(display, 1U, ">Mod:Constant       ");
  TEST_ASSERT_EQUAL_CHAR(' ', display.cells[2U][0U]);
  TEST_ASSERT_EQUAL_CHAR('T', display.cells[2U][1U]);
  TEST_ASSERT_EQUAL_CHAR('e', display.cells[2U][2U]);
  TEST_ASSERT_EQUAL_CHAR('m', display.cells[2U][3U]);
  TEST_ASSERT_EQUAL_CHAR('p', display.cells[2U][4U]);
  TEST_ASSERT_EQUAL_CHAR(':', display.cells[2U][5U]);
  TEST_ASSERT_EQUAL_CHAR('5', display.cells[2U][6U]);
  TEST_ASSERT_EQUAL_CHAR('7', display.cells[2U][7U]);
  TEST_ASSERT_EQUAL_CHAR(static_cast<char>(0xDF), display.cells[2U][8U]);
  TEST_ASSERT_EQUAL_CHAR('C', display.cells[2U][9U]);
  assertLineEquals(display, 3U, " Dur:8h 0m          ");
}

void test_manual_program_view_renders_boost_fields() {
  FakeDisplay display;
  LcdManualProgramView view(display);
  LcdManualProgramSnapshot snapshot = baseSnapshot();
  snapshot.mode = ManualProgramMode::Boost;
  snapshot.selectedField = ManualProgramField::BoostDelta;
  snapshot.selectedIndex = 3U;
  snapshot.editing = true;

  view.render(snapshot);

  assertLineEquals(display, 1U, "*Boost:+10\xDF""C        ");
  assertLineEquals(display, 2U, " DurBoost:30m       ");
  assertLineEquals(display, 3U, " Start              ");
}

void test_manual_program_view_renders_fluctuating_fields_and_heartbeat() {
  FakeDisplay display;
  LcdManualProgramView view(display);
  LcdManualProgramSnapshot snapshot = baseSnapshot();
  snapshot.mode = ManualProgramMode::Fluctuating;
  snapshot.selectedField = ManualProgramField::UpperTemp;
  snapshot.selectedIndex = 3U;
  snapshot.heartbeatOn = true;

  view.render(snapshot);

  TEST_ASSERT_EQUAL_CHAR('>', display.cells[1U][0U]);
  TEST_ASSERT_EQUAL_CHAR('T', display.cells[1U][1U]);
  TEST_ASSERT_EQUAL_CHAR('s', display.cells[1U][2U]);
  TEST_ASSERT_EQUAL_CHAR('u', display.cells[1U][3U]);
  TEST_ASSERT_EQUAL_CHAR('p', display.cells[1U][4U]);
  TEST_ASSERT_EQUAL_CHAR(':', display.cells[1U][5U]);
  TEST_ASSERT_EQUAL_CHAR('6', display.cells[1U][6U]);
  TEST_ASSERT_EQUAL_CHAR('2', display.cells[1U][7U]);
  TEST_ASSERT_EQUAL_CHAR(static_cast<char>(0xDF), display.cells[1U][8U]);
  TEST_ASSERT_EQUAL_CHAR('C', display.cells[1U][9U]);
  assertLineEquals(display, 2U, " Tinf:52\xDF""C          ");
  TEST_ASSERT_TRUE(display.custom[3U][19U]);
}

void test_manual_program_view_renders_constant_start_and_back_at_end() {
  FakeDisplay display;
  LcdManualProgramView view(display);
  LcdManualProgramSnapshot snapshot = baseSnapshot();
  snapshot.selectedField = ManualProgramField::Start;
  snapshot.selectedIndex = 3U;

  view.render(snapshot);

  assertLineEquals(display, 1U, ">Start              ");
  assertLineEquals(display, 2U, " Salveaza           ");
  assertLineEquals(display, 3U, " Inapoi             ");
}

void test_manual_program_view_renders_boost_start_and_back_at_end() {
  FakeDisplay display;
  LcdManualProgramView view(display);
  LcdManualProgramSnapshot snapshot = baseSnapshot();
  snapshot.mode = ManualProgramMode::Boost;
  snapshot.selectedField = ManualProgramField::Start;
  snapshot.selectedIndex = 5U;

  view.render(snapshot);

  assertLineEquals(display, 1U, ">Start              ");
  assertLineEquals(display, 2U, " Salveaza           ");
  assertLineEquals(display, 3U, " Inapoi             ");
}

void test_manual_program_view_renders_fluctuating_late_fields() {
  FakeDisplay display;
  LcdManualProgramView view(display);
  LcdManualProgramSnapshot snapshot = baseSnapshot();
  snapshot.mode = ManualProgramMode::Fluctuating;
  snapshot.selectedField = ManualProgramField::UpperDuration;
  snapshot.selectedIndex = 5U;

  view.render(snapshot);

  assertLineEquals(display, 1U, ">Dur Tsup:10m       ");
  assertLineEquals(display, 2U, " Dur Tinf:10m       ");
  assertLineEquals(display, 3U, " Start              ");
}

void test_manual_program_view_renders_fluctuating_start_and_back_at_end() {
  FakeDisplay display;
  LcdManualProgramView view(display);
  LcdManualProgramSnapshot snapshot = baseSnapshot();
  snapshot.mode = ManualProgramMode::Fluctuating;
  snapshot.selectedField = ManualProgramField::Start;
  snapshot.selectedIndex = 7U;

  view.render(snapshot);

  assertLineEquals(display, 1U, ">Start              ");
  assertLineEquals(display, 2U, " Salveaza           ");
  assertLineEquals(display, 3U, " Inapoi             ");
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_manual_program_view_renders_mode_selector_first);
  RUN_TEST(test_manual_program_view_renders_boost_fields);
  RUN_TEST(test_manual_program_view_renders_fluctuating_fields_and_heartbeat);
  RUN_TEST(test_manual_program_view_renders_constant_start_and_back_at_end);
  RUN_TEST(test_manual_program_view_renders_boost_start_and_back_at_end);
  RUN_TEST(test_manual_program_view_renders_fluctuating_late_fields);
  RUN_TEST(test_manual_program_view_renders_fluctuating_start_and_back_at_end);
  return UNITY_END();
}
