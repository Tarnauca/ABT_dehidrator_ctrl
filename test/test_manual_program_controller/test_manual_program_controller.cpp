#include <unity.h>

#include "dehydrator/ui/ManualProgramController.h"

using dehydrator::ManualProgramController;
using dehydrator::ManualProgramField;
using dehydrator::ProfileMode;

void test_navigation_moves_through_manual_program_fields() {
  ManualProgramController controller;

  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::Temperature),
                        static_cast<int>(controller.selectedField()));
  TEST_ASSERT_TRUE(controller.onRotate(1).selectionChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::Duration),
                        static_cast<int>(controller.selectedField()));
  TEST_ASSERT_TRUE(controller.onRotate(1).selectionChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::Fluctuating),
                        static_cast<int>(controller.selectedField()));
}

void test_short_press_toggles_edit_mode_and_rotation_changes_temperature() {
  ManualProgramController controller;

  controller.onShortPress();
  TEST_ASSERT_TRUE(controller.editing());

  const auto result = controller.onRotate(1);

  TEST_ASSERT_TRUE(result.valueChanged);
  TEST_ASSERT_EQUAL_INT(58, controller.targetTempC());
}

void test_manual_program_profile_is_fixed_by_default() {
  ManualProgramController controller;

  const auto profile = controller.profile();

  TEST_ASSERT_EQUAL_INT(static_cast<int>(ProfileMode::Fixed),
                        static_cast<int>(profile.mode));
  TEST_ASSERT_EQUAL_INT(57, profile.targetTempC);
}

void test_manual_program_profile_builds_fluctuating_range() {
  ManualProgramController controller;
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onShortPress();
  controller.onRotate(1);

  const auto profile = controller.profile();

  TEST_ASSERT_EQUAL_INT(static_cast<int>(ProfileMode::Fluctuating),
                        static_cast<int>(profile.mode));
  TEST_ASSERT_EQUAL_INT(52, profile.lowTempC);
  TEST_ASSERT_EQUAL_INT(62, profile.highTempC);
  TEST_ASSERT_EQUAL_UINT(20U, profile.highPhaseMinutes);
  TEST_ASSERT_EQUAL_UINT(20U, profile.lowPhaseMinutes);
}

void test_back_resets_selection_to_temperature() {
  ManualProgramController controller;
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);

  const auto result = controller.onShortPress();

  TEST_ASSERT_TRUE(result.exitToMenu);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::Temperature),
                        static_cast<int>(controller.selectedField()));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_navigation_moves_through_manual_program_fields);
  RUN_TEST(test_short_press_toggles_edit_mode_and_rotation_changes_temperature);
  RUN_TEST(test_manual_program_profile_is_fixed_by_default);
  RUN_TEST(test_manual_program_profile_builds_fluctuating_range);
  RUN_TEST(test_back_resets_selection_to_temperature);
  return UNITY_END();
}
