#include <unity.h>

#include "dehydrator/ui/LcdNtcCalibrationView.h"

using dehydrator::CharacterDisplay;
using dehydrator::LcdNtcCalibrationSnapshot;
using dehydrator::LcdNtcCalibrationView;
using dehydrator::LcdStatusView;
using dehydrator::NtcCalibrationField;

class FakeDisplay : public CharacterDisplay {
 public:
  char cells[LcdStatusView::ROWS][LcdStatusView::COLUMNS] = {};
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
    }
    cursorColumn++;
  }

  void writeCustom(uint8_t code) override {
    writeChar(static_cast<char>(code));
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

void test_view_renders_offset_scale_and_save() {
  FakeDisplay display;
  LcdNtcCalibrationView view(display);
  LcdNtcCalibrationSnapshot snapshot;
  snapshot.selectedField = NtcCalibrationField::Offset;
  snapshot.selectedIndex = 0U;
  snapshot.offsetCentiC = 130;
  snapshot.scalePpm = 1030000;

  view.render(snapshot);

  assertLineEquals(display, 0U, "Calibrare NTC       ");
  for (uint8_t column = 0U; column < 12U; column++) {
    TEST_ASSERT_EQUAL_CHAR(">Offset:+1.3"[column], display.cells[1U][column]);
  }
  assertDegreeSymbolAt(display, 1U, 12U);
  TEST_ASSERT_EQUAL_CHAR('C', display.cells[1U][13U]);
  assertLineEquals(display, 2U, " Scala:1.03         ");
  assertLineEquals(display, 3U, " Salveaza           ");
}

void test_view_renders_restore_and_back_when_scrolled() {
  FakeDisplay display;
  LcdNtcCalibrationView view(display);
  LcdNtcCalibrationSnapshot snapshot;
  snapshot.selectedField = NtcCalibrationField::Back;
  snapshot.selectedIndex = 3U;
  snapshot.offsetCentiC = -70;
  snapshot.scalePpm = 980000;

  view.render(snapshot);

  assertLineEquals(display, 1U, " Restabileste       ");
  assertLineEquals(display, 2U, ">Inapoi             ");
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_view_renders_offset_scale_and_save);
  RUN_TEST(test_view_renders_restore_and_back_when_scrolled);
  return UNITY_END();
}
