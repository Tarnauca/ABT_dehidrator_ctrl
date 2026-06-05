#include <unity.h>

#include "dehydrator/ui/NtcCalibrationController.h"

using dehydrator::NtcCalibrationController;
using dehydrator::NtcCalibrationField;

void test_load_starts_on_offset_with_defaults() {
  NtcCalibrationController controller;
  controller.loadFromCalibration(dehydrator::config::CALIBRATION);

  TEST_ASSERT_EQUAL_INT(static_cast<int>(NtcCalibrationField::Offset),
                        static_cast<int>(controller.selectedField()));
  TEST_ASSERT_EQUAL_UINT8(0U, controller.selectedIndex());
  TEST_ASSERT_FALSE(controller.editing());
  TEST_ASSERT_FALSE(controller.dirty());
}

void test_offset_edits_in_tenth_degree_steps() {
  NtcCalibrationController controller;
  controller.loadFromCalibration(dehydrator::config::CALIBRATION);
  controller.onShortPress();

  const auto result = controller.onRotate(1);

  TEST_ASSERT_TRUE(result.valueChanged);
  TEST_ASSERT_EQUAL_INT16(10, controller.offsetCentiC());
  TEST_ASSERT_TRUE(controller.dirty());
}

void test_scale_edits_in_hundredth_steps() {
  NtcCalibrationController controller;
  controller.loadFromCalibration(dehydrator::config::CALIBRATION);
  controller.onRotate(1);
  controller.onShortPress();

  const auto result = controller.onRotate(-1);

  TEST_ASSERT_TRUE(result.valueChanged);
  TEST_ASSERT_EQUAL_INT32(990000, controller.scalePpm());
  TEST_ASSERT_TRUE(controller.dirty());
}

void test_restore_loads_firmware_defaults_into_editor() {
  NtcCalibrationController controller;
  controller.loadFromCalibration(dehydrator::config::CALIBRATION);
  controller.onShortPress();
  controller.onRotate(1);
  controller.onShortPress();
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onShortPress();

  TEST_ASSERT_EQUAL_INT16(dehydrator::config::CALIBRATION.ntcOffsetCentiC,
                          controller.offsetCentiC());
  TEST_ASSERT_EQUAL_INT32(dehydrator::config::CALIBRATION.ntcScalePpm,
                          controller.scalePpm());
}

void test_save_clears_dirty_and_requests_persist() {
  NtcCalibrationController controller;
  controller.loadFromCalibration(dehydrator::config::CALIBRATION);
  controller.onShortPress();
  controller.onRotate(1);
  controller.onShortPress();
  controller.onRotate(1);
  controller.onRotate(1);

  const auto result = controller.onShortPress();

  TEST_ASSERT_TRUE(result.saveRequested);
  TEST_ASSERT_TRUE(controller.dirty());
  controller.markSaved();
  TEST_ASSERT_FALSE(controller.dirty());
}

void test_back_discards_unsaved_changes_and_exits() {
  NtcCalibrationController controller;
  controller.loadFromCalibration(dehydrator::config::CALIBRATION);
  controller.onShortPress();
  controller.onRotate(1);
  controller.onShortPress();
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);

  const auto result = controller.onShortPress();

  TEST_ASSERT_TRUE(result.exitToSettings);
  TEST_ASSERT_EQUAL_INT16(dehydrator::config::CALIBRATION.ntcOffsetCentiC,
                          controller.offsetCentiC());
  TEST_ASSERT_EQUAL_INT32(dehydrator::config::CALIBRATION.ntcScalePpm,
                          controller.scalePpm());
  TEST_ASSERT_FALSE(controller.dirty());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_load_starts_on_offset_with_defaults);
  RUN_TEST(test_offset_edits_in_tenth_degree_steps);
  RUN_TEST(test_scale_edits_in_hundredth_steps);
  RUN_TEST(test_restore_loads_firmware_defaults_into_editor);
  RUN_TEST(test_save_clears_dirty_and_requests_persist);
  RUN_TEST(test_back_discards_unsaved_changes_and_exits);
  return UNITY_END();
}
