#include <unity.h>

#include "dehydrator/ui/LcdStatusView.h"

using dehydrator::CharacterDisplay;
using dehydrator::LcdStatusSnapshot;
using dehydrator::LcdStatusView;

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

LcdStatusSnapshot validSnapshot() {
  LcdStatusSnapshot snapshot;
  snapshot.stateLabel = "INACTIV";
  snapshot.pt50TempC = 57;
  snapshot.pt50Valid = true;
  snapshot.rhPercent = 43U;
  snapshot.rhValid = true;
  snapshot.heaterOn = false;
  snapshot.fanOn = true;
  snapshot.heartbeatOn = true;
  return snapshot;
}

void test_status_view_renders_romanian_4x20_status_lines() {
  FakeDisplay display;
  LcdStatusView view(display);

  view.render(validSnapshot());

  assertLineEquals(display, 0U, "Stare: INACTIV      ");
  assertLineEquals(display, 1U, "T:57C    RH:43%     ");
  assertLineEquals(display, 2U, "H:OFF     F:ON      ");
  TEST_ASSERT_TRUE(display.custom[3U][19U]);
  TEST_ASSERT_EQUAL_CHAR(static_cast<char>(LcdStatusView::HEARTBEAT_CHAR),
                         display.cells[3U][19U]);
}

void test_status_view_renders_missing_sensor_values() {
  FakeDisplay display;
  LcdStatusView view(display);
  LcdStatusSnapshot snapshot = validSnapshot();
  snapshot.pt50Valid = false;
  snapshot.rhValid = false;
  snapshot.heartbeatOn = false;

  view.render(snapshot);

  assertLineEquals(display, 1U, "T:--C    RH:--%     ");
  TEST_ASSERT_FALSE(display.custom[3U][19U]);
  TEST_ASSERT_EQUAL_CHAR(' ', display.cells[3U][19U]);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_status_view_renders_romanian_4x20_status_lines);
  RUN_TEST(test_status_view_renders_missing_sensor_values);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
