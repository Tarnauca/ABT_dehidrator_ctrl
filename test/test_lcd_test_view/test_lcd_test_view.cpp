#include <unity.h>

#include "dehydrator/ui/LcdTestView.h"

using dehydrator::CharacterDisplay;
using dehydrator::LcdStatusView;
using dehydrator::LcdTestSnapshot;
using dehydrator::LcdTestView;
using dehydrator::TestField;

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

void assertDegreeSymbolAt(const FakeDisplay& display, uint8_t row,
                          uint8_t column) {
  TEST_ASSERT_EQUAL_CHAR(static_cast<char>(223), display.cells[row][column]);
}

void assertLinePrefixEquals(const FakeDisplay& display, uint8_t row,
                            const char* expected, uint8_t length) {
  for (uint8_t column = 0U; column < length; column++) {
    TEST_ASSERT_EQUAL_CHAR(expected[column], display.cells[row][column]);
  }
}

void test_test_view_renders_sensor_rows_first() {
  FakeDisplay display;
  LcdTestView view(display);
  LcdTestSnapshot snapshot;
  snapshot.selectedField = TestField::NtcTemp;
  snapshot.ntc.valid = true;
  snapshot.ntc.tempDeciC = 520;
  snapshot.tempRh.valid = true;
  snapshot.tempRh.tempDeciC = 380;
  snapshot.tempRh.rhPercent = 41;
  snapshot.command.fanOn = true;
  snapshot.command.heaterOn = false;

  view.render(snapshot);

  assertLineEquals(display, 0U, "Testare             ");
  assertLinePrefixEquals(display, 1U, ">NTC: 52.0", 10U);
  assertDegreeSymbolAt(display, 1U, 10U);
  TEST_ASSERT_EQUAL_CHAR('C', display.cells[1U][11U]);
  assertLinePrefixEquals(display, 2U, " AM2302 T: 38.0", 15U);
  assertDegreeSymbolAt(display, 2U, 15U);
  TEST_ASSERT_EQUAL_CHAR('C', display.cells[2U][16U]);
  assertLineEquals(display, 3U, " AM2302 RH: 41%     ");
}

void test_test_view_renders_output_rows_when_scrolled() {
  FakeDisplay display;
  LcdTestView view(display);
  LcdTestSnapshot snapshot;
  snapshot.selectedField = TestField::Back;
  snapshot.ntc.valid = true;
  snapshot.ntc.tempDeciC = 520;
  snapshot.tempRh.valid = true;
  snapshot.tempRh.tempDeciC = 380;
  snapshot.tempRh.rhPercent = 41;
  snapshot.command.fanOn = true;
  snapshot.command.heaterOn = true;

  view.render(snapshot);

  assertLineEquals(display, 1U, " Fan: ON            ");
  assertLineEquals(display, 2U, " Heat: ON           ");
  assertLineEquals(display, 3U, ">Inapoi             ");
}

void test_test_view_renders_sensor_errors_succinctly() {
  FakeDisplay display;
  LcdTestView view(display);
  LcdTestSnapshot snapshot;
  snapshot.selectedField = TestField::TempRhTemp;
  snapshot.command.fanOn = false;
  snapshot.command.heaterOn = false;

  view.render(snapshot);

  assertLineEquals(display, 1U, " NTC: Eroare        ");
  assertLineEquals(display, 2U, ">AM2302 T: Eroare   ");
  assertLineEquals(display, 3U, " AM2302 RH: Eroare  ");
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_test_view_renders_sensor_rows_first);
  RUN_TEST(test_test_view_renders_output_rows_when_scrolled);
  RUN_TEST(test_test_view_renders_sensor_errors_succinctly);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
