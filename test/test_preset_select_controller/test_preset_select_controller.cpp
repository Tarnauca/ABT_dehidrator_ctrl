#include <unity.h>

#include "dehydrator/ui/PresetSelectController.h"

using dehydrator::PresetSelectController;

void test_rotation_moves_between_presets() {
  PresetSelectController controller;

  const auto result = controller.onRotate(1);

  TEST_ASSERT_TRUE(result.selectionChanged);
  TEST_ASSERT_EQUAL_UINT(1U, controller.selectedIndex());
  TEST_ASSERT_EQUAL_STRING("Ierburi", controller.currentPreset()->label);
}

void test_rotation_stops_at_ends() {
  PresetSelectController controller;

  const auto start = controller.onRotate(-1);
  TEST_ASSERT_FALSE(start.selectionChanged);
  TEST_ASSERT_EQUAL_UINT(0U, controller.selectedIndex());

  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);

  const auto end = controller.onRotate(1);
  TEST_ASSERT_FALSE(end.selectionChanged);
  TEST_ASSERT_EQUAL_UINT(3U, controller.selectedIndex());
}

void test_short_press_confirms_selected_preset() {
  PresetSelectController controller;

  const auto result = controller.onShortPress();

  TEST_ASSERT_TRUE(result.presetSelected);
  TEST_ASSERT_EQUAL_STRING("mere", controller.currentPreset()->token);
}

void test_long_press_returns_to_menu() {
  PresetSelectController controller;

  const auto result = controller.onLongPress();

  TEST_ASSERT_TRUE(result.exitToMenu);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_rotation_moves_between_presets);
  RUN_TEST(test_rotation_stops_at_ends);
  RUN_TEST(test_short_press_confirms_selected_preset);
  RUN_TEST(test_long_press_returns_to_menu);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
