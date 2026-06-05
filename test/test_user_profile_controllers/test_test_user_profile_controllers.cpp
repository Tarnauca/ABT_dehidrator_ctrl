#include <unity.h>

#include "dehydrator/ui/UserProfileActionController.h"
#include "dehydrator/ui/UserProfileSlotController.h"

using dehydrator::UserProfileAction;
using dehydrator::UserProfileActionController;
using dehydrator::UserProfileSlotController;

void test_slot_controller_exposes_back_entry_after_last_slot() {
  UserProfileSlotController controller;
  for (uint8_t index = 0U; index < UserProfileSlotController::BACK_INDEX; index++) {
    controller.onRotate(1);
  }

  TEST_ASSERT_TRUE(controller.currentIsBack());
  TEST_ASSERT_TRUE(controller.onShortPress().exitRequested);
}

void test_occupied_profile_actions_include_start_edit_delete_back() {
  UserProfileActionController controller;
  controller.setOccupied(true);

  TEST_ASSERT_EQUAL_INT(static_cast<int>(UserProfileAction::Start),
                        static_cast<int>(controller.currentAction()));
  controller.onRotate(1);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(UserProfileAction::Edit),
                        static_cast<int>(controller.currentAction()));
  controller.onRotate(1);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(UserProfileAction::Delete),
                        static_cast<int>(controller.currentAction()));
}

void test_vacant_profile_actions_start_with_edit() {
  UserProfileActionController controller;
  controller.setOccupied(false);

  TEST_ASSERT_EQUAL_INT(static_cast<int>(UserProfileAction::Edit),
                        static_cast<int>(controller.currentAction()));
  TEST_ASSERT_TRUE(controller.onShortPress().editRequested);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_slot_controller_exposes_back_entry_after_last_slot);
  RUN_TEST(test_occupied_profile_actions_include_start_edit_delete_back);
  RUN_TEST(test_vacant_profile_actions_start_with_edit);
  return UNITY_END();
}
