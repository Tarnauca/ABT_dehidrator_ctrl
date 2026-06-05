#include <unity.h>

#include "dehydrator/ui/SettingsMenuController.h"

using dehydrator::SettingsMenuController;
using dehydrator::SettingsMenuItem;

void test_settings_menu_starts_on_ntc_calibration() {
  SettingsMenuController controller;

  TEST_ASSERT_EQUAL_INT(static_cast<int>(SettingsMenuItem::NtcCalibration),
                        static_cast<int>(controller.currentItem()));
  TEST_ASSERT_EQUAL_UINT8(0U, controller.selectedIndex());
}

void test_settings_menu_rotates_through_testare_to_back_without_wrapping() {
  SettingsMenuController controller;

  TEST_ASSERT_TRUE(controller.onRotate(1).selectionChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(SettingsMenuItem::Testare),
                        static_cast<int>(controller.currentItem()));

  TEST_ASSERT_TRUE(controller.onRotate(1).selectionChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(SettingsMenuItem::Back),
                        static_cast<int>(controller.currentItem()));

  TEST_ASSERT_FALSE(controller.onRotate(1).selectionChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(SettingsMenuItem::Back),
                        static_cast<int>(controller.currentItem()));
}

void test_settings_menu_back_resets_to_first_entry() {
  SettingsMenuController controller;
  controller.onRotate(1);
  controller.onRotate(1);

  const dehydrator::SettingsMenuResult result = controller.onShortPress();

  TEST_ASSERT_TRUE(result.exitToMainMenu);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(SettingsMenuItem::NtcCalibration),
                        static_cast<int>(controller.currentItem()));
}

void test_settings_menu_opens_ntc_calibration_first() {
  SettingsMenuController controller;

  const dehydrator::SettingsMenuResult result = controller.onShortPress();

  TEST_ASSERT_TRUE(result.openNtcCalibration);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_settings_menu_starts_on_ntc_calibration);
  RUN_TEST(test_settings_menu_rotates_through_testare_to_back_without_wrapping);
  RUN_TEST(test_settings_menu_back_resets_to_first_entry);
  RUN_TEST(test_settings_menu_opens_ntc_calibration_first);
  return UNITY_END();
}
