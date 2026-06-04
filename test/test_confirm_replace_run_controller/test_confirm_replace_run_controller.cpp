#include <unity.h>

#include "dehydrator/ui/ConfirmReplaceRunController.h"

using dehydrator::ConfirmReplaceRunController;

void test_default_selection_is_no() {
  ConfirmReplaceRunController controller;

  TEST_ASSERT_FALSE(controller.confirmSelected());
}

void test_rotation_can_select_yes() {
  ConfirmReplaceRunController controller;

  const auto result = controller.onRotate(1);

  TEST_ASSERT_TRUE(result.selectionChanged);
  TEST_ASSERT_TRUE(controller.confirmSelected());
}

void test_short_press_on_no_cancels() {
  ConfirmReplaceRunController controller;

  const auto result = controller.onShortPress();

  TEST_ASSERT_TRUE(result.cancelled);
  TEST_ASSERT_FALSE(result.confirmed);
}

void test_short_press_on_yes_confirms() {
  ConfirmReplaceRunController controller;
  controller.onRotate(1);

  const auto result = controller.onShortPress();

  TEST_ASSERT_TRUE(result.confirmed);
  TEST_ASSERT_FALSE(result.cancelled);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_default_selection_is_no);
  RUN_TEST(test_rotation_can_select_yes);
  RUN_TEST(test_short_press_on_no_cancels);
  RUN_TEST(test_short_press_on_yes_confirms);
  return UNITY_END();
}
