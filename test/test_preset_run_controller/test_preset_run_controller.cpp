#include <unity.h>

#include "dehydrator/app/PresetRunController.h"

using dehydrator::PresetCatalog;
using dehydrator::PresetRunController;
using dehydrator::ProfileConfig;
using dehydrator::ProfileMode;
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

void test_invalid_ntc_keeps_heater_off_while_run_continues() {
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

void test_confirmed_stop_returns_running_preset_to_idle() {
  PresetRunController controller;
  TEST_ASSERT_TRUE(controller.startPreset(PresetCatalog::items()[0]));

  controller.update(30U, true, 58);

  TEST_ASSERT_TRUE(controller.stopConfirmed());
  TEST_ASSERT_EQUAL_INT(static_cast<int>(RunState::Idle),
                        static_cast<int>(controller.snapshot().state));
  TEST_ASSERT_NULL(controller.activePreset());
  TEST_ASSERT_FALSE(controller.outputCommand().fanOn);
  TEST_ASSERT_FALSE(controller.outputCommand().heaterOn);
  TEST_ASSERT_FALSE(controller.outputCommand().buzzerOn);
}

void test_manual_boost_profile_updates_without_active_preset() {
  PresetRunController controller;
  ProfileConfig profile;
  profile.mode = ProfileMode::Boost;
  profile.targetTempC = 55;
  profile.highTempC = 65;
  profile.durationMinutes = 120;
  profile.highPhaseMinutes = 30;

  TEST_ASSERT_TRUE(controller.startProfile(profile, "manual"));
  TEST_ASSERT_NULL(controller.activePreset());
  TEST_ASSERT_EQUAL_STRING("manual", controller.activeRunToken());

  controller.update(10U, true, 60);

  TEST_ASSERT_EQUAL_INT(static_cast<int>(RunState::Running),
                        static_cast<int>(controller.snapshot().state));
  TEST_ASSERT_TRUE(controller.outputCommand().fanOn);
  TEST_ASSERT_TRUE(controller.outputCommand().heaterOn);
  TEST_ASSERT_EQUAL_INT(65, controller.lastTargetTempC());
}

void test_manual_constant_profile_uses_base_target_without_active_preset() {
  PresetRunController controller;
  ProfileConfig profile;
  profile.mode = ProfileMode::Fixed;
  profile.targetTempC = 57;
  profile.durationMinutes = 60;

  TEST_ASSERT_TRUE(controller.startProfile(profile, "manual"));
  controller.update(10U, true, 50);

  TEST_ASSERT_NULL(controller.activePreset());
  TEST_ASSERT_EQUAL_STRING("manual", controller.activeRunToken());
  TEST_ASSERT_EQUAL_INT(57, controller.lastTargetTempC());
  TEST_ASSERT_TRUE(controller.outputCommand().heaterOn);
}

void test_manual_fluctuating_profile_switches_between_upper_and_lower_targets() {
  PresetRunController controller;
  ProfileConfig profile;
  profile.mode = ProfileMode::Fluctuating;
  profile.targetTempC = 57;
  profile.lowTempC = 52;
  profile.highTempC = 62;
  profile.durationMinutes = 60;
  profile.highPhaseMinutes = 10;
  profile.lowPhaseMinutes = 10;

  TEST_ASSERT_TRUE(controller.startProfile(profile, "manual"));
  controller.update(10U, true, 50);
  TEST_ASSERT_EQUAL_INT(62, controller.lastTargetTempC());

  controller.update(10U * 60U, true, 50);
  TEST_ASSERT_EQUAL_INT(52, controller.lastTargetTempC());
}

void test_manual_stop_clears_active_run_token() {
  PresetRunController controller;
  ProfileConfig profile;
  profile.mode = ProfileMode::Fixed;
  profile.targetTempC = 57;
  profile.durationMinutes = 60;

  TEST_ASSERT_TRUE(controller.startProfile(profile, "manual"));
  TEST_ASSERT_TRUE(controller.stopConfirmed());

  TEST_ASSERT_NULL(controller.activePreset());
  TEST_ASSERT_NULL(controller.activeRunToken());
  TEST_ASSERT_FALSE(controller.outputCommand().fanOn);
}

void test_manual_finish_acknowledge_clears_active_run_token() {
  PresetRunController controller;
  ProfileConfig profile;
  profile.mode = ProfileMode::Fixed;
  profile.targetTempC = 57;
  profile.durationMinutes = 1;

  TEST_ASSERT_TRUE(controller.startProfile(profile, "manual"));
  controller.update(60U, true, 50);
  controller.update(3U * 60U, true, 50);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(RunState::FinishedAlarm),
                        static_cast<int>(controller.snapshot().state));

  TEST_ASSERT_TRUE(controller.acknowledgeFinished());
  TEST_ASSERT_NULL(controller.activeRunToken());
  TEST_ASSERT_NULL(controller.activePreset());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_start_preset_enters_running_state_and_enables_fan);
  RUN_TEST(test_running_update_uses_temperature_control_to_turn_heater_on);
  RUN_TEST(test_invalid_ntc_keeps_heater_off_while_run_continues);
  RUN_TEST(test_finish_alarm_can_be_acknowledged_back_to_idle);
  RUN_TEST(test_confirmed_stop_returns_running_preset_to_idle);
  RUN_TEST(test_manual_boost_profile_updates_without_active_preset);
  RUN_TEST(test_manual_constant_profile_uses_base_target_without_active_preset);
  RUN_TEST(test_manual_fluctuating_profile_switches_between_upper_and_lower_targets);
  RUN_TEST(test_manual_stop_clears_active_run_token);
  RUN_TEST(test_manual_finish_acknowledge_clears_active_run_token);
  return UNITY_END();
}
