#include <unity.h>

#include "dehydrator/ui/SavePromptController.h"

using dehydrator::SavePromptChoice;
using dehydrator::SavePromptController;

void test_default_choice_is_no() {
  SavePromptController controller;

  TEST_ASSERT_EQUAL_INT(static_cast<int>(SavePromptChoice::No),
                        static_cast<int>(controller.currentChoice()));
}

void test_rotation_moves_between_three_choices() {
  SavePromptController controller;

  TEST_ASSERT_TRUE(controller.onRotate(1).selectionChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(SavePromptChoice::Cancel),
                        static_cast<int>(controller.currentChoice()));
  TEST_ASSERT_TRUE(controller.onRotate(-1).selectionChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(SavePromptChoice::No),
                        static_cast<int>(controller.currentChoice()));
  TEST_ASSERT_TRUE(controller.onRotate(-1).selectionChanged);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(SavePromptChoice::Yes),
                        static_cast<int>(controller.currentChoice()));
}

void test_short_press_confirms_current_choice() {
  SavePromptController controller;
  controller.onRotate(-1);

  const auto result = controller.onShortPress();

  TEST_ASSERT_TRUE(result.confirmed);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(SavePromptChoice::Yes),
                        static_cast<int>(result.choice));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_default_choice_is_no);
  RUN_TEST(test_rotation_moves_between_three_choices);
  RUN_TEST(test_short_press_confirms_current_choice);
  return UNITY_END();
}
