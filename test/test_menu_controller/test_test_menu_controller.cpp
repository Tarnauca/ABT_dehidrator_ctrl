#include <unity.h>

#include "dehydrator/ui/MenuController.h"

using dehydrator::MenuController;
using dehydrator::UiAction;
using dehydrator::UiResult;
using dehydrator::UiScreen;

void test_short_press_opens_menu_from_status() {
  MenuController controller;

  const UiResult result = controller.onShortPress();

  TEST_ASSERT_EQUAL(static_cast<int>(UiAction::OpenMenu),
                    static_cast<int>(result.action));
  TEST_ASSERT_EQUAL(static_cast<int>(UiScreen::Menu),
                    static_cast<int>(controller.screen()));
  TEST_ASSERT_EQUAL_UINT(0U, controller.selectedIndex());
}

void test_rotation_moves_selection_when_menu_is_open() {
  MenuController controller;
  controller.onShortPress();

  const UiResult result = controller.onRotate(1);

  TEST_ASSERT_EQUAL(static_cast<int>(UiAction::MoveSelection),
                    static_cast<int>(result.action));
  TEST_ASSERT_EQUAL_UINT(1U, controller.selectedIndex());
  TEST_ASSERT_EQUAL_STRING("Mod manual", controller.currentItem());
}

void test_rotation_stops_at_end_of_menu() {
  MenuController controller;
  controller.onShortPress();
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);

  const UiResult result = controller.onRotate(1);

  TEST_ASSERT_EQUAL(static_cast<int>(UiAction::None),
                    static_cast<int>(result.action));
  TEST_ASSERT_EQUAL_UINT(MenuController::ITEM_COUNT - 1U, controller.selectedIndex());
  TEST_ASSERT_EQUAL_STRING("Inapoi", controller.currentItem());
}

void test_rotation_stops_at_start_of_menu() {
  MenuController controller;
  controller.onShortPress();

  const UiResult result = controller.onRotate(-1);

  TEST_ASSERT_EQUAL(static_cast<int>(UiAction::None),
                    static_cast<int>(result.action));
  TEST_ASSERT_EQUAL_UINT(0U, controller.selectedIndex());
  TEST_ASSERT_EQUAL_STRING("Pornire preset", controller.currentItem());
}

void test_short_press_on_menu_selects_current_item() {
  MenuController controller;
  controller.onShortPress();
  controller.onRotate(1);

  const UiResult result = controller.onShortPress();

  TEST_ASSERT_EQUAL(static_cast<int>(UiAction::SelectItem),
                    static_cast<int>(result.action));
  TEST_ASSERT_EQUAL_STRING("Mod manual", result.selectedItem);
}

void test_short_press_on_inapoi_closes_menu() {
  MenuController controller;
  controller.onShortPress();
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);
  controller.onRotate(1);

  const UiResult result = controller.onShortPress();

  TEST_ASSERT_EQUAL(static_cast<int>(UiAction::CloseMenu),
                    static_cast<int>(result.action));
  TEST_ASSERT_EQUAL(static_cast<int>(UiScreen::Status),
                    static_cast<int>(controller.screen()));
}

void test_return_to_status_resynchronizes_menu_state() {
  MenuController controller;
  controller.onShortPress();
  controller.onRotate(1);

  controller.returnToStatus();

  TEST_ASSERT_EQUAL(static_cast<int>(UiScreen::Status),
                    static_cast<int>(controller.screen()));

  const UiResult result = controller.onShortPress();
  TEST_ASSERT_EQUAL(static_cast<int>(UiAction::OpenMenu),
                    static_cast<int>(result.action));
}

void test_enter_menu_resynchronizes_menu_state() {
  MenuController controller;

  controller.enterMenu();

  TEST_ASSERT_EQUAL(static_cast<int>(UiScreen::Menu),
                    static_cast<int>(controller.screen()));
  const UiResult result = controller.onRotate(1);
  TEST_ASSERT_EQUAL(static_cast<int>(UiAction::MoveSelection),
                    static_cast<int>(result.action));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_short_press_opens_menu_from_status);
  RUN_TEST(test_rotation_moves_selection_when_menu_is_open);
  RUN_TEST(test_rotation_stops_at_end_of_menu);
  RUN_TEST(test_rotation_stops_at_start_of_menu);
  RUN_TEST(test_short_press_on_menu_selects_current_item);
  RUN_TEST(test_short_press_on_inapoi_closes_menu);
  RUN_TEST(test_return_to_status_resynchronizes_menu_state);
  RUN_TEST(test_enter_menu_resynchronizes_menu_state);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
