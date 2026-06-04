#include <unity.h>

#include "dehydrator/app/PresetRunController.h"

using dehydrator::PresetCatalog;
using dehydrator::PresetRunController;
using dehydrator::RunState;

void test_start_preset_enters_running_state_and_enables_fan() {
  PresetRunController controller;

  TEST_ASSERT_TRUE(controller.startPreset(PresetCatalog::items()[0]));

  const auto snapshot = controller.snapshot();
  const auto command = controller.outputCommand();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(RunState::Running),
                        static_cast<int>(snapshot.state));
  TEST_ASSERT_EQUAL_STRING("RULARE", controller.stateLabelRo());
  TEST_ASSERT_EQUAL_STRING("mere", controller.activePreset()->token);
  TEST_ASSERT_TRUE(command.fanOn);
  TEST_ASSERT_FALSE(command.heaterOn);
}

void test_running_update_uses_temperature_control_to_turn_heater_on() {
  PresetRunController controller;
  TEST_ASSERT_TRUE(controller.startPreset(PresetCatalog::items()[2]));

  controller.update(10U, true, 60);

  const auto command = controller.outputCommand();
  TEST_ASSERT_TRUE(command.fanOn);
  TEST_ASSERT_TRUE(command.heaterOn);
  TEST_ASSERT_EQUAL_INT(65, controller.lastTargetTempC());
}

void test_invalid_pt50_keeps_heater_off_while_run_continues() {
  PresetRunController controller;
  TEST_ASSERT_TRUE(controller.startPreset(PresetCatalog::items()[2]));

  controller.update(10U, false, 0);

  const auto snapshot = controller.snapshot();
  const auto command = controller.outputCommand();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(RunState::Running),
                        static_cast<int>(snapshot.state));
  TEST_ASSERT_TRUE(command.fanOn);
  TEST_ASSERT_FALSE(command.heaterOn);
}

void test_finish_alarm_can_be_acknowledged_back_to_idle() {
  PresetRunController controller;
  TEST_ASSERT_TRUE(controller.startPreset(PresetCatalog::items()[1]));

  controller.update(6U * 60U * 60U, true, 40);
  controller.update(3U * 60U, true, 40);

  TEST_ASSERT_EQUAL_INT(static_cast<int>(RunState::FinishedAlarm),
                        static_cast<int>(controller.snapshot().state));
  TEST_ASSERT_TRUE(controller.outputCommand().buzzerOn);
  TEST_ASSERT_TRUE(controller.acknowledgeFinished());
  TEST_ASSERT_EQUAL_INT(static_cast<int>(RunState::Idle),
                        static_cast<int>(controller.snapshot().state));
  TEST_ASSERT_NULL(controller.activePreset());
  TEST_ASSERT_FALSE(controller.outputCommand().fanOn);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_start_preset_enters_running_state_and_enables_fan);
  RUN_TEST(test_running_update_uses_temperature_control_to_turn_heater_on);
  RUN_TEST(test_invalid_pt50_keeps_heater_off_while_run_continues);
  RUN_TEST(test_finish_alarm_can_be_acknowledged_back_to_idle);
  return UNITY_END();
}
