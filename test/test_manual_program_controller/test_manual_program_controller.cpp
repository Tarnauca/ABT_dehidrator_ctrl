#include <unity.h>

#include "dehydrator/ui/ManualProgramController.h"

using dehydrator::ManualProgramController;
using dehydrator::ManualProgramField;
using dehydrator::ManualProgramMode;
using dehydrator::ProfileMode;

void enterEdit(ManualProgramController& controller) {
  controller.onShortPress();
  TEST_ASSERT_TRUE(controller.editing());
}

void leaveEdit(ManualProgramController& controller) {
  controller.onShortPress();
  TEST_ASSERT_FALSE(controller.editing());
}

void selectMode(ManualProgramController& controller, ManualProgramMode mode) {
  enterEdit(controller);
  while (controller.mode() != mode) {
    controller.onRotate(1);
  }
  leaveEdit(controller);
}

void selectField(ManualProgramController& controller, ManualProgramField field) {
  uint8_t targetIndex = 0U;
  for (uint8_t index = 0U; index < controller.fieldCount(); index++) {
    if (controller.fieldAt(index) == field) {
      targetIndex = index;
      break;
    }
  }

  while (controller.selectedField() != field) {
    const int8_t direction = controller.selectedIndex() < targetIndex ? 1 : -1;
    const auto result = controller.onRotate(direction);
    TEST_ASSERT_TRUE(result.selectionChanged);
  }
}

void test_manual_program_starts_on_mode_selector() {
  ManualProgramController controller;

  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::Mode),
                        static_cast<int>(controller.selectedField()));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramMode::Constant),
                        static_cast<int>(controller.mode()));
}

void test_mode_selection_stops_at_edges_and_resets_to_mode() {
  ManualProgramController controller;

  enterEdit(controller);
  TEST_ASSERT_FALSE(controller.onRotate(-1).valueChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramMode::Constant),
                        static_cast<int>(controller.mode()));
  TEST_ASSERT_TRUE(controller.onRotate(1).valueChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramMode::Boost),
                        static_cast<int>(controller.mode()));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::Mode),
                        static_cast<int>(controller.selectedField()));
  TEST_ASSERT_TRUE(controller.editing());
  TEST_ASSERT_TRUE(controller.onRotate(1).valueChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramMode::Fluctuating),
                        static_cast<int>(controller.mode()));
  TEST_ASSERT_FALSE(controller.onRotate(1).valueChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramMode::Fluctuating),
                        static_cast<int>(controller.mode()));
}

void test_constant_mode_field_list_and_profile() {
  ManualProgramController controller;

  TEST_ASSERT_TRUE(controller.onRotate(1).selectionChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::Temperature),
                        static_cast<int>(controller.selectedField()));
  TEST_ASSERT_TRUE(controller.onRotate(1).selectionChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::Duration),
                        static_cast<int>(controller.selectedField()));

  const auto profile = controller.profile();

  TEST_ASSERT_EQUAL_INT(static_cast<int>(ProfileMode::Fixed),
                        static_cast<int>(profile.mode));
  TEST_ASSERT_EQUAL_INT(57, profile.targetTempC);
  TEST_ASSERT_EQUAL_UINT(8U * 60U, profile.durationMinutes);
}

void test_boost_mode_builds_initial_boost_profile() {
  ManualProgramController controller;
  selectMode(controller, ManualProgramMode::Boost);

  const auto profile = controller.profile();

  TEST_ASSERT_EQUAL_INT(static_cast<int>(ProfileMode::Boost),
                        static_cast<int>(profile.mode));
  TEST_ASSERT_EQUAL_INT(57, profile.targetTempC);
  TEST_ASSERT_EQUAL_INT(67, profile.highTempC);
  TEST_ASSERT_EQUAL_UINT(30U, profile.highPhaseMinutes);
  TEST_ASSERT_EQUAL_UINT(8U * 60U, profile.durationMinutes);
}

void test_boost_delta_is_limited_to_20_and_75_degrees() {
  ManualProgramController controller;
  selectMode(controller, ManualProgramMode::Boost);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::BoostDelta),
                        static_cast<int>(controller.selectedField()));

  enterEdit(controller);
  TEST_ASSERT_TRUE(controller.onRotate(1).valueChanged);
  TEST_ASSERT_FALSE(controller.onRotate(1).valueChanged);

  TEST_ASSERT_EQUAL_INT(15, controller.boostDeltaC());
  TEST_ASSERT_EQUAL_INT(72, controller.targetTempC() + controller.boostDeltaC());
}

void test_boost_duration_cannot_exceed_half_total_duration() {
  ManualProgramController controller;
  selectMode(controller, ManualProgramMode::Boost);
  selectField(controller, ManualProgramField::BoostDuration);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::BoostDuration),
                        static_cast<int>(controller.selectedField()));

  enterEdit(controller);
  for (uint8_t index = 0U; index < 90U; index++) {
    controller.onRotate(1);
  }

  TEST_ASSERT_EQUAL_UINT(240U, controller.boostDurationMinutes());
}

void test_boost_delta_and_duration_lower_bounds_are_blocked() {
  ManualProgramController controller;
  selectMode(controller, ManualProgramMode::Boost);
  selectField(controller, ManualProgramField::BoostDelta);

  enterEdit(controller);
  TEST_ASSERT_TRUE(controller.onRotate(-1).valueChanged);
  TEST_ASSERT_TRUE(controller.onRotate(-1).valueChanged);
  TEST_ASSERT_FALSE(controller.onRotate(-1).valueChanged);
  TEST_ASSERT_EQUAL_INT(0, controller.boostDeltaC());
  leaveEdit(controller);

  selectField(controller, ManualProgramField::BoostDuration);
  enterEdit(controller);
  for (uint8_t index = 0U; index < 10U; index++) {
    controller.onRotate(-1);
  }
  TEST_ASSERT_EQUAL_UINT(5U, controller.boostDurationMinutes());
  TEST_ASSERT_FALSE(controller.onRotate(-1).valueChanged);
}

void test_total_duration_cannot_drop_below_twice_boost_duration() {
  ManualProgramController controller;
  selectMode(controller, ManualProgramMode::Boost);
  selectField(controller, ManualProgramField::BoostDuration);
  enterEdit(controller);
  for (uint8_t index = 0U; index < 42U; index++) {
    controller.onRotate(1);
  }
  TEST_ASSERT_EQUAL_UINT(240U, controller.boostDurationMinutes());
  leaveEdit(controller);

  selectField(controller, ManualProgramField::Duration);
  enterEdit(controller);
  TEST_ASSERT_FALSE(controller.onRotate(-1).valueChanged);
  TEST_ASSERT_EQUAL_UINT(480U, controller.durationMinutes());
}

void test_fluctuating_mode_builds_absolute_targets_and_phase_durations() {
  ManualProgramController controller;
  selectMode(controller, ManualProgramMode::Fluctuating);

  const auto profile = controller.profile();

  TEST_ASSERT_EQUAL_INT(static_cast<int>(ProfileMode::Fluctuating),
                        static_cast<int>(profile.mode));
  TEST_ASSERT_EQUAL_INT(57, profile.targetTempC);
  TEST_ASSERT_EQUAL_INT(62, profile.highTempC);
  TEST_ASSERT_EQUAL_INT(52, profile.lowTempC);
  TEST_ASSERT_EQUAL_UINT(10U, profile.highPhaseMinutes);
  TEST_ASSERT_EQUAL_UINT(10U, profile.lowPhaseMinutes);
}

void test_fluctuating_absolute_targets_are_blocked_outside_reference_window() {
  ManualProgramController controller;
  selectMode(controller, ManualProgramMode::Fluctuating);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::UpperTemp),
                        static_cast<int>(controller.selectedField()));

  enterEdit(controller);
  for (uint8_t index = 0U; index < 20U; index++) {
    controller.onRotate(1);
  }

  TEST_ASSERT_EQUAL_INT(67, controller.upperTempC());
}

void test_fluctuating_targets_stay_on_expected_side_of_reference() {
  ManualProgramController controller;
  selectMode(controller, ManualProgramMode::Fluctuating);
  selectField(controller, ManualProgramField::UpperTemp);

  enterEdit(controller);
  for (uint8_t index = 0U; index < 20U; index++) {
    controller.onRotate(-1);
  }
  TEST_ASSERT_EQUAL_INT(57, controller.upperTempC());
  TEST_ASSERT_FALSE(controller.onRotate(-1).valueChanged);
  leaveEdit(controller);

  selectField(controller, ManualProgramField::LowerTemp);
  enterEdit(controller);
  for (uint8_t index = 0U; index < 20U; index++) {
    controller.onRotate(1);
  }
  TEST_ASSERT_EQUAL_INT(57, controller.lowerTempC());
  TEST_ASSERT_FALSE(controller.onRotate(1).valueChanged);
}

void test_fluctuating_lower_target_is_blocked_below_reference_window() {
  ManualProgramController controller;
  selectMode(controller, ManualProgramMode::Fluctuating);
  selectField(controller, ManualProgramField::LowerTemp);

  enterEdit(controller);
  for (uint8_t index = 0U; index < 20U; index++) {
    controller.onRotate(-1);
  }

  TEST_ASSERT_EQUAL_INT(47, controller.lowerTempC());
  TEST_ASSERT_FALSE(controller.onRotate(-1).valueChanged);
}

void test_fluctuating_phase_duration_bounds_are_blocked() {
  ManualProgramController controller;
  selectMode(controller, ManualProgramMode::Fluctuating);
  selectField(controller, ManualProgramField::UpperDuration);

  enterEdit(controller);
  for (uint8_t index = 0U; index < 20U; index++) {
    controller.onRotate(-1);
  }
  TEST_ASSERT_EQUAL_UINT(5U, controller.upperDurationMinutes());
  TEST_ASSERT_FALSE(controller.onRotate(-1).valueChanged);

  for (uint8_t index = 0U; index < 30U; index++) {
    controller.onRotate(1);
  }
  TEST_ASSERT_EQUAL_UINT(20U, controller.upperDurationMinutes());
  TEST_ASSERT_FALSE(controller.onRotate(1).valueChanged);
}

void test_reference_edit_is_blocked_when_existing_fluctuating_targets_would_be_invalid() {
  ManualProgramController controller;
  selectMode(controller, ManualProgramMode::Fluctuating);
  selectField(controller, ManualProgramField::UpperTemp);

  enterEdit(controller);
  for (uint8_t index = 0U; index < 20U; index++) {
    controller.onRotate(1);
  }
  TEST_ASSERT_EQUAL_INT(67, controller.upperTempC());
  leaveEdit(controller);

  selectField(controller, ManualProgramField::LowerTemp);
  enterEdit(controller);
  for (uint8_t index = 0U; index < 20U; index++) {
    controller.onRotate(-1);
  }
  TEST_ASSERT_EQUAL_INT(47, controller.lowerTempC());
  leaveEdit(controller);

  selectField(controller, ManualProgramField::Temperature);

  enterEdit(controller);
  TEST_ASSERT_FALSE(controller.onRotate(1).valueChanged);
  TEST_ASSERT_FALSE(controller.onRotate(-1).valueChanged);
  TEST_ASSERT_EQUAL_INT(57, controller.targetTempC());
}

void test_back_resets_selection_to_mode() {
  ManualProgramController controller;
  selectMode(controller, ManualProgramMode::Fluctuating);
  for (uint8_t index = 0U; index < 9U; index++) {
    controller.onRotate(1);
  }

  const auto result = controller.onShortPress();

  TEST_ASSERT_TRUE(result.exitToMenu);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::Mode),
                        static_cast<int>(controller.selectedField()));
}

void test_save_field_is_present_before_back() {
  ManualProgramController controller;
  selectField(controller, ManualProgramField::Save);

  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::Save),
                        static_cast<int>(controller.selectedField()));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramField::Back),
                        static_cast<int>(controller.fieldAt(controller.selectedIndex() + 1U)));
}

void test_editing_marks_profile_dirty_and_save_clears_dirty() {
  ManualProgramController controller;
  selectField(controller, ManualProgramField::Temperature);
  enterEdit(controller);
  TEST_ASSERT_TRUE(controller.onRotate(1).valueChanged);
  TEST_ASSERT_TRUE(controller.dirty());
  leaveEdit(controller);

  controller.markSaved(2U);

  TEST_ASSERT_FALSE(controller.dirty());
  TEST_ASSERT_TRUE(controller.hasAssociatedSlot());
  TEST_ASSERT_EQUAL_UINT(2U, controller.associatedSlot());
}

void test_discard_changes_restores_last_saved_profile() {
  ManualProgramController controller;
  controller.loadProfile(
      dehydrator::ProfileConfig{ProfileMode::Boost, 55, 0, 65, 360, 30, 0}, true,
      1U);
  selectField(controller, ManualProgramField::Temperature);
  enterEdit(controller);
  TEST_ASSERT_TRUE(controller.onRotate(1).valueChanged);
  leaveEdit(controller);
  TEST_ASSERT_TRUE(controller.dirty());

  controller.discardChanges();

  TEST_ASSERT_FALSE(controller.dirty());
  TEST_ASSERT_EQUAL_INT(55, controller.targetTempC());
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ManualProgramMode::Boost),
                        static_cast<int>(controller.mode()));
  TEST_ASSERT_TRUE(controller.hasAssociatedSlot());
  TEST_ASSERT_EQUAL_UINT(1U, controller.associatedSlot());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_manual_program_starts_on_mode_selector);
  RUN_TEST(test_mode_selection_stops_at_edges_and_resets_to_mode);
  RUN_TEST(test_constant_mode_field_list_and_profile);
  RUN_TEST(test_boost_mode_builds_initial_boost_profile);
  RUN_TEST(test_boost_delta_is_limited_to_20_and_75_degrees);
  RUN_TEST(test_boost_duration_cannot_exceed_half_total_duration);
  RUN_TEST(test_boost_delta_and_duration_lower_bounds_are_blocked);
  RUN_TEST(test_total_duration_cannot_drop_below_twice_boost_duration);
  RUN_TEST(test_fluctuating_mode_builds_absolute_targets_and_phase_durations);
  RUN_TEST(test_fluctuating_absolute_targets_are_blocked_outside_reference_window);
  RUN_TEST(test_fluctuating_targets_stay_on_expected_side_of_reference);
  RUN_TEST(test_fluctuating_lower_target_is_blocked_below_reference_window);
  RUN_TEST(test_fluctuating_phase_duration_bounds_are_blocked);
  RUN_TEST(test_reference_edit_is_blocked_when_existing_fluctuating_targets_would_be_invalid);
  RUN_TEST(test_back_resets_selection_to_mode);
  RUN_TEST(test_save_field_is_present_before_back);
  RUN_TEST(test_editing_marks_profile_dirty_and_save_clears_dirty);
  RUN_TEST(test_discard_changes_restores_last_saved_profile);
  return UNITY_END();
}
