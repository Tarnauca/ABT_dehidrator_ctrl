#include <string.h>

#include <unity.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/ui/LcdConfirmReplaceRunView.h"

namespace {

class FakeDisplay final : public dehydrator::CharacterDisplay {
 public:
  FakeDisplay() { clear(); }

  void setCursor(uint8_t column, uint8_t row) override {
    column_ = column;
    row_ = row;
  }

  void writeChar(char value) override {
    if (row_ < 4U && column_ < 20U) {
      cells_[row_][column_] = value;
    }
    if (column_ < 20U) {
      column_++;
    }
  }

  void writeCustom(uint8_t) override { writeChar('#'); }

  void clear() {
    for (uint8_t row = 0U; row < 4U; row++) {
      for (uint8_t column = 0U; column < 20U; column++) {
        cells_[row][column] = ' ';
      }
      cells_[row][20U] = '\0';
    }
  }

  const char* line(uint8_t row) const { return cells_[row]; }

 private:
  char cells_[4U][21U] = {};
  uint8_t column_ = 0U;
  uint8_t row_ = 0U;
};

void assertLineEquals(const FakeDisplay& display, uint8_t row,
                      const char* expected) {
  TEST_ASSERT_EQUAL_STRING(expected, display.line(row));
}

}  // namespace

void test_confirm_view_renders_no_as_default_selection() {
  FakeDisplay display;
  dehydrator::LcdConfirmReplaceRunView view(display);
  dehydrator::LcdConfirmReplaceRunSnapshot snapshot;
  snapshot.confirmSelected = false;

  view.render(snapshot);

  assertLineEquals(display, 0U, "Confirmare          ");
  assertLineEquals(display, 1U, "Pornesti programul  ");
  assertLineEquals(display, 2U, "nou?                ");
  assertLineEquals(display, 3U, ">Nu    Da           ");
}

void test_confirm_view_renders_yes_selected() {
  FakeDisplay display;
  dehydrator::LcdConfirmReplaceRunView view(display);
  dehydrator::LcdConfirmReplaceRunSnapshot snapshot;
  snapshot.confirmSelected = true;

  view.render(snapshot);

  assertLineEquals(display, 1U, "Pornesti programul  ");
  assertLineEquals(display, 2U, "nou?                ");
  TEST_ASSERT_EQUAL_CHAR(' ', display.line(3U)[0]);
  TEST_ASSERT_EQUAL_CHAR('N', display.line(3U)[1]);
  TEST_ASSERT_EQUAL_CHAR('u', display.line(3U)[2]);
  TEST_ASSERT_EQUAL_CHAR('>', display.line(3U)[6]);
  TEST_ASSERT_EQUAL_CHAR('D', display.line(3U)[7]);
  TEST_ASSERT_EQUAL_CHAR('a', display.line(3U)[8]);
  TEST_ASSERT_EQUAL_CHAR(' ', display.line(3U)[19]);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_confirm_view_renders_no_as_default_selection);
  RUN_TEST(test_confirm_view_renders_yes_selected);
  return UNITY_END();
}
