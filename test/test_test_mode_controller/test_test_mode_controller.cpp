#include <unity.h>

#include "dehydrator/ui/TestModeController.h"

using dehydrator::TestField;
using dehydrator::TestModeController;

void test_rotation_moves_from_sensor_rows_to_output_rows() {
  TestModeController controller;

  const auto first = controller.onRotate(1);
  TEST_ASSERT_TRUE(first.selectionChanged);
  TEST_ASSERT_EQUAL(static_cast<int>(TestField::TempRhTemp),
                    static_cast<int>(controller.selectedField()));

  controller.onRotate(1);
  controller.onRotate(1);

  const auto fourth = controller.onRotate(1);
  TEST_ASSERT_TRUE(fourth.selectionChanged);
  TEST_ASSERT_EQUAL(static_cast<int>(TestField::Heater),
                    static_cast<int>(controller.selectedField()));

  const auto fifth = controller.onRotate(-1);
  TEST_ASSERT_TRUE(fifth.selectionChanged);
  TEST_ASSERT_EQUAL(static_cast<int>(TestField::Fan),
                    static_cast<int>(controller.selectedField()));
}

void test_short_press_on_sensor_rows_has_no_effect() {
  TestModeController controller;

  const auto result = controller.onShortPress();

  TEST_ASSERT_FALSE(result.exitToMenu);
  TEST_ASSERT_FALSE(result.outputChanged);
  TEST_ASSERT_FALSE(result.selectionChanged);
  TEST_ASSERT_FALSE(controller.command().fanOn);
  TEST_ASSERT_FALSE(controller.command().heaterOn);
}

void test_toggling_heater_forces_fan_on() {
  TestModeController controller;
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);

  const auto result = controller.onShortPress();
  const auto command = controller.command();

  TEST_ASSERT_TRUE(result.outputChanged);
  TEST_ASSERT_TRUE(command.heaterOn);
  TEST_ASSERT_TRUE(command.fanOn);
}

void test_toggling_fan_off_while_heater_on_also_turns_heater_off() {
  TestModeController controller;
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onShortPress();
  controller.onRotate(-1);

  const auto result = controller.onShortPress();
  const auto command = controller.command();

  TEST_ASSERT_TRUE(result.outputChanged);
  TEST_ASSERT_FALSE(command.fanOn);
  TEST_ASSERT_FALSE(command.heaterOn);
}

void test_short_press_on_back_requests_exit_to_menu() {
  TestModeController controller;
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);

  const auto result = controller.onShortPress();

  TEST_ASSERT_TRUE(result.exitToMenu);
  TEST_ASSERT_EQUAL(static_cast<int>(TestField::NtcTemp),
                    static_cast<int>(controller.selectedField()));
}

void test_long_press_has_no_assigned_action() {
  TestModeController controller;

  const auto result = controller.onLongPress();

  TEST_ASSERT_FALSE(result.exitToMenu);
  TEST_ASSERT_FALSE(result.outputChanged);
  TEST_ASSERT_FALSE(result.selectionChanged);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_rotation_moves_from_sensor_rows_to_output_rows);
  RUN_TEST(test_short_press_on_sensor_rows_has_no_effect);
  RUN_TEST(test_toggling_heater_forces_fan_on);
  RUN_TEST(test_toggling_fan_off_while_heater_on_also_turns_heater_off);
  RUN_TEST(test_short_press_on_back_requests_exit_to_menu);
  RUN_TEST(test_long_press_has_no_assigned_action);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
