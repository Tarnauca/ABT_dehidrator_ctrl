#include <unity.h>

#include "dehydrator/ui/LcdManualView.h"

using dehydrator::CharacterDisplay;
using dehydrator::LcdManualSnapshot;
using dehydrator::LcdManualView;
using dehydrator::LcdStatusView;
using dehydrator::ManualField;

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

void test_manual_view_renders_fan_selected() {
  FakeDisplay display;
  LcdManualView view(display);
  LcdManualSnapshot snapshot;
  snapshot.selectedField = ManualField::Fan;
  snapshot.command.fanOn = true;
  snapshot.command.heaterOn = false;
  snapshot.heartbeatOn = true;

  view.render(snapshot);

  assertLineEquals(display, 0U, "Mod manual          ");
  assertLineEquals(display, 1U, ">Fan: ON            ");
  assertLineEquals(display, 2U, " Heat: OFF          ");
  assertLinePrefixEquals(display, 3U, "Apas=Sch Tine=Inap", 18U);
  TEST_ASSERT_TRUE(display.custom[3U][19U]);
}

void test_manual_view_renders_heater_selected() {
  FakeDisplay display;
  LcdManualView view(display);
  LcdManualSnapshot snapshot;
  snapshot.selectedField = ManualField::Heater;
  snapshot.command.fanOn = true;
  snapshot.command.heaterOn = true;

  view.render(snapshot);

  assertLineEquals(display, 1U, " Fan: ON            ");
  assertLineEquals(display, 2U, ">Heat: ON           ");
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_manual_view_renders_fan_selected);
  RUN_TEST(test_manual_view_renders_heater_selected);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
