#include <unity.h>

#include "dehydrator/ui/LcdStatusView.h"

using dehydrator::CharacterDisplay;
using dehydrator::LcdStatusSnapshot;
using dehydrator::LcdStatusView;
using dehydrator::ProfileConfig;
using dehydrator::ProfileMode;
using dehydrator::StatusActivityIndicator;
using dehydrator::StatusPage;

class FakeDisplay : public CharacterDisplay {
 public:
  char cells[LcdStatusView::ROWS][LcdStatusView::COLUMNS] = {};
  bool custom[LcdStatusView::ROWS][LcdStatusView::COLUMNS] = {};
  uint8_t cursorColumn = 0U;
  uint8_t cursorRow = 0U;

  /**
   * @brief Moves the emulated LCD cursor.
   *
   * @param column Zero-based target column.
   * @param row Zero-based target row.
   */
  void setCursor(uint8_t column, uint8_t row) override {
    cursorColumn = column;
    cursorRow = row;
  }

  /**
   * @brief Writes one normal character into the emulated LCD buffer.
   *
   * @param value Character byte to store.
   */
  void writeChar(char value) override {
    if (cursorRow < LcdStatusView::ROWS &&
        cursorColumn < LcdStatusView::COLUMNS) {
      cells[cursorRow][cursorColumn] = value;
      custom[cursorRow][cursorColumn] = false;
    }
    cursorColumn++;
  }

  /**
   * @brief Writes one custom character marker into the emulated LCD buffer.
   *
   * @param code Custom character slot code.
   */
  void writeCustom(uint8_t code) override {
    if (cursorRow < LcdStatusView::ROWS &&
        cursorColumn < LcdStatusView::COLUMNS) {
      cells[cursorRow][cursorColumn] = static_cast<char>(code);
      custom[cursorRow][cursorColumn] = true;
    }
    cursorColumn++;
  }
};

/**
 * @brief Asserts that one LCD row matches the provided fixed-width text.
 *
 * @param display Emulated LCD buffer.
 * @param row Zero-based row index.
 * @param expected Full 20-character expected row contents.
 */
void assertLineEquals(const FakeDisplay& display, uint8_t row,
                      const char* expected) {
  for (uint8_t column = 0U; column < LcdStatusView::COLUMNS; column++) {
    TEST_ASSERT_EQUAL_CHAR(expected[column], display.cells[row][column]);
  }
}

/**
 * @brief Asserts that one LCD row matches one prefix up to a given length.
 *
 * @param display Emulated LCD buffer.
 * @param row Zero-based row index.
 * @param expected Expected prefix contents.
 * @param length Number of columns to compare.
 */
void assertLinePrefixEquals(const FakeDisplay& display, uint8_t row,
                            const char* expected, uint8_t length) {
  for (uint8_t column = 0U; column < length; column++) {
    TEST_ASSERT_EQUAL_CHAR(expected[column], display.cells[row][column]);
  }
}

/**
 * @brief Creates one representative status snapshot for renderer tests.
 *
 * @return Snapshot populated with valid sensor and program values.
 */
LcdStatusSnapshot validSnapshot() {
  LcdStatusSnapshot snapshot;
  snapshot.page = StatusPage::Summary;
  snapshot.programLabel = "Mere";
  snapshot.ntcTempDeciC = 570;
  snapshot.ntcValid = true;
  snapshot.rhPercent = 43U;
  snapshot.rhValid = true;
  snapshot.elapsedMinutes = 90U;
  snapshot.remainingMinutes = 510U;
  snapshot.profileValid = true;
  snapshot.profile =
      ProfileConfig{ProfileMode::Fluctuating, 57, 50, 65, 600U, 20U, 20U};
  snapshot.heaterOn = false;
  snapshot.fanOn = true;
  snapshot.activityIndicator = StatusActivityIndicator::Running;
  snapshot.activityIndicatorOn = true;
  return snapshot;
}

void test_status_view_renders_summary_page() {
  FakeDisplay display;
  LcdStatusView view(display);

  view.render(validSnapshot());

  assertLinePrefixEquals(display, 0U, "Program: Mere       ", 19U);
  assertLineEquals(display, 1U, "Temp: 57.0\xDF""C RH: 43%");
  assertLineEquals(display, 2U, "Timp scurs: 1h 30m  ");
  TEST_ASSERT_TRUE(display.custom[0U][19U]);
  TEST_ASSERT_EQUAL_CHAR(static_cast<char>(LcdStatusView::PLAY_CHAR),
                         display.cells[0U][19U]);
}

void test_status_view_renders_missing_sensor_values() {
  FakeDisplay display;
  LcdStatusView view(display);
  LcdStatusSnapshot snapshot = validSnapshot();
  snapshot.ntcValid = false;
  snapshot.rhValid = false;
  snapshot.activityIndicator = StatusActivityIndicator::None;
  snapshot.activityIndicatorOn = false;

  view.render(snapshot);

  assertLineEquals(display, 1U, "Temp: --.-\xDF""C RH: --%");
  TEST_ASSERT_FALSE(display.custom[0U][19U]);
  TEST_ASSERT_EQUAL_CHAR(' ', display.cells[0U][19U]);
}

void test_status_view_renders_paused_indicator_on_summary_page() {
  FakeDisplay display;
  LcdStatusView view(display);
  LcdStatusSnapshot snapshot = validSnapshot();
  snapshot.activityIndicator = StatusActivityIndicator::Paused;

  view.render(snapshot);

  TEST_ASSERT_TRUE(display.custom[0U][19U]);
  TEST_ASSERT_EQUAL_CHAR(static_cast<char>(LcdStatusView::PAUSE_CHAR),
                         display.cells[0U][19U]);
}

void test_status_view_renders_boost_parameter_page() {
  FakeDisplay display;
  LcdStatusView view(display);
  LcdStatusSnapshot snapshot = validSnapshot();
  snapshot.page = StatusPage::ParametersPrimary;
  snapshot.profile =
      ProfileConfig{ProfileMode::Boost, 55, 0, 65, 480U, 30U, 0U};

  view.render(snapshot);

  assertLinePrefixEquals(display, 0U, "Temp: 55.0\xDF""C", 12U);
  assertLineEquals(display, 1U, "Durata: 8h 0m       ");
  assertLinePrefixEquals(display, 2U, "Boost: +10.0\xDF""C", 14U);
}

void test_status_view_renders_fluctuating_secondary_parameter_page() {
  FakeDisplay display;
  LcdStatusView view(display);
  LcdStatusSnapshot snapshot = validSnapshot();
  snapshot.page = StatusPage::ParametersSecondary;

  view.render(snapshot);

  assertLineEquals(display, 0U, "Dur. Tsup: 0h 20m   ");
  assertLineEquals(display, 1U, "Dur. Tinf: 0h 20m   ");
}

void test_status_view_renders_outputs_page() {
  FakeDisplay display;
  LcdStatusView view(display);
  LcdStatusSnapshot snapshot = validSnapshot();
  snapshot.page = StatusPage::Outputs;
  snapshot.heaterOn = true;
  snapshot.fanOn = false;

  view.render(snapshot);

  assertLineEquals(display, 0U, "Incalzitor: Pornit  ");
  assertLineEquals(display, 1U, "Ventilator: Oprit   ");
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_status_view_renders_summary_page);
  RUN_TEST(test_status_view_renders_missing_sensor_values);
  RUN_TEST(test_status_view_renders_paused_indicator_on_summary_page);
  RUN_TEST(test_status_view_renders_boost_parameter_page);
  RUN_TEST(test_status_view_renders_fluctuating_secondary_parameter_page);
  RUN_TEST(test_status_view_renders_outputs_page);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
