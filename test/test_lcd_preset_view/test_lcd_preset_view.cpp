#include <unity.h>

#include "dehydrator/ui/LcdPresetView.h"
#include "dehydrator/presets/PresetCatalog.h"

using dehydrator::CharacterDisplay;
using dehydrator::LcdPresetSnapshot;
using dehydrator::LcdPresetView;
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

void assertLinePrefixEquals(const FakeDisplay& display, uint8_t row,
                            const char* expected, uint8_t length) {
  for (uint8_t column = 0U; column < length; column++) {
    TEST_ASSERT_EQUAL_CHAR(expected[column], display.cells[row][column]);
  }
}

LcdPresetSnapshot snapshotForIndex(size_t selectedIndex, bool heartbeatOn) {
  LcdPresetSnapshot snapshot;
  snapshot.presets = dehydrator::PresetCatalog::items();
  snapshot.presetCount = dehydrator::PresetCatalog::PRESET_COUNT;
  snapshot.selectedIndex = selectedIndex;
  snapshot.heartbeatOn = heartbeatOn;
  return snapshot;
}

void test_preset_view_renders_selected_preset_details() {
  FakeDisplay display;
  LcdPresetView view(display);

  view.render(snapshotForIndex(0U, true));

  assertLineEquals(display, 0U, "Pornire preset      ");
  assertLineEquals(display, 1U, ">Mere               ");
  assertLineEquals(display, 2U, "Mod fluctuat        ");
  TEST_ASSERT_EQUAL_CHAR('5', display.cells[3U][0U]);
  TEST_ASSERT_EQUAL_CHAR('0', display.cells[3U][1U]);
  TEST_ASSERT_EQUAL_CHAR('-', display.cells[3U][2U]);
  TEST_ASSERT_EQUAL_CHAR('6', display.cells[3U][3U]);
  TEST_ASSERT_EQUAL_CHAR('5', display.cells[3U][4U]);
  TEST_ASSERT_EQUAL_CHAR(static_cast<char>(0xDF), display.cells[3U][5U]);
  TEST_ASSERT_EQUAL_CHAR('C', display.cells[3U][6U]);
  TEST_ASSERT_EQUAL_CHAR(' ', display.cells[3U][7U]);
  TEST_ASSERT_EQUAL_CHAR('1', display.cells[3U][8U]);
  TEST_ASSERT_EQUAL_CHAR('0', display.cells[3U][9U]);
  TEST_ASSERT_EQUAL_CHAR('h', display.cells[3U][10U]);
  TEST_ASSERT_EQUAL_CHAR(' ', display.cells[3U][11U]);
  TEST_ASSERT_EQUAL_CHAR('0', display.cells[3U][12U]);
  TEST_ASSERT_EQUAL_CHAR('m', display.cells[3U][13U]);
  TEST_ASSERT_TRUE(display.custom[3U][19U]);
}

void test_preset_view_renders_other_selection() {
  FakeDisplay display;
  LcdPresetView view(display);

  view.render(snapshotForIndex(2U, false));

  assertLineEquals(display, 1U, ">Jerky              ");
  assertLineEquals(display, 2U, "Mod fix             ");
  TEST_ASSERT_EQUAL_CHAR('6', display.cells[3U][0U]);
  TEST_ASSERT_EQUAL_CHAR('5', display.cells[3U][1U]);
  TEST_ASSERT_EQUAL_CHAR(static_cast<char>(0xDF), display.cells[3U][2U]);
  TEST_ASSERT_EQUAL_CHAR('C', display.cells[3U][3U]);
  TEST_ASSERT_EQUAL_CHAR(' ', display.cells[3U][4U]);
  TEST_ASSERT_EQUAL_CHAR('8', display.cells[3U][5U]);
  TEST_ASSERT_EQUAL_CHAR('h', display.cells[3U][6U]);
  TEST_ASSERT_EQUAL_CHAR(' ', display.cells[3U][7U]);
  TEST_ASSERT_EQUAL_CHAR('0', display.cells[3U][8U]);
  TEST_ASSERT_EQUAL_CHAR('m', display.cells[3U][9U]);
}

void test_preset_view_renders_back_row() {
  FakeDisplay display;
  LcdPresetView view(display);
  LcdPresetSnapshot snapshot = snapshotForIndex(dehydrator::PresetCatalog::PRESET_COUNT,
                                                false);

  view.render(snapshot);

  assertLineEquals(display, 0U, "Pornire preset      ");
  assertLineEquals(display, 1U, ">Inapoi             ");
  assertLineEquals(display, 2U, "                    ");
  assertLineEquals(display, 3U, "                    ");
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_preset_view_renders_selected_preset_details);
  RUN_TEST(test_preset_view_renders_other_selection);
  RUN_TEST(test_preset_view_renders_back_row);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
