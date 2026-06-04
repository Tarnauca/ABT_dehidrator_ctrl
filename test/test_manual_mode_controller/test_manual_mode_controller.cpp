#include <unity.h>

#include "dehydrator/ui/ManualModeController.h"

using dehydrator::ManualField;
using dehydrator::ManualModeController;

void test_rotation_switches_between_fan_and_heater_fields() {
  ManualModeController controller;

  const auto first = controller.onRotate(1);
  TEST_ASSERT_TRUE(first.selectionChanged);
  TEST_ASSERT_EQUAL(static_cast<int>(ManualField::Heater),
                    static_cast<int>(controller.selectedField()));

  const auto second = controller.onRotate(1);
  TEST_ASSERT_TRUE(second.selectionChanged);
  TEST_ASSERT_EQUAL(static_cast<int>(ManualField::Back),
                    static_cast<int>(controller.selectedField()));

  const auto third = controller.onRotate(-1);
  TEST_ASSERT_TRUE(third.selectionChanged);
  TEST_ASSERT_EQUAL(static_cast<int>(ManualField::Heater),
                    static_cast<int>(controller.selectedField()));
}

void test_toggling_heater_forces_fan_on() {
  ManualModeController controller;
  controller.onRotate(1);

  const auto result = controller.onShortPress();
  const auto command = controller.command();

  TEST_ASSERT_TRUE(result.outputChanged);
  TEST_ASSERT_TRUE(command.heaterOn);
  TEST_ASSERT_TRUE(command.fanOn);
}

void test_toggling_fan_off_while_heater_on_also_turns_heater_off() {
  ManualModeController controller;
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
  ManualModeController controller;
  controller.onRotate(1);
  controller.onRotate(1);

  const auto result = controller.onShortPress();

  TEST_ASSERT_TRUE(result.exitToMenu);
}

void test_long_press_requests_exit_to_menu() {
  ManualModeController controller;

  const auto result = controller.onLongPress();

  TEST_ASSERT_TRUE(result.exitToMenu);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_rotation_switches_between_fan_and_heater_fields);
  RUN_TEST(test_toggling_heater_forces_fan_on);
  RUN_TEST(test_toggling_fan_off_while_heater_on_also_turns_heater_off);
  RUN_TEST(test_short_press_on_back_requests_exit_to_menu);
  RUN_TEST(test_long_press_requests_exit_to_menu);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
