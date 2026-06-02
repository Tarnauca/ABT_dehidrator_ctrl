#include <unity.h>

#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/domain/TemperatureControl.h"

using dehydrator::RunOutputPolicy;
using dehydrator::TemperatureControl;
using dehydrator::TemperatureControlInput;
using dehydrator::TemperatureControlOutput;
using dehydrator::config::ControlConfig;

ControlConfig controlConfig() { return dehydrator::config::CONTROL; }

ControlConfig immediateControlConfig() {
  ControlConfig config = controlConfig();
  config.minHeaterOnSeconds = 0;
  config.minHeaterOffSeconds = 0;
  return config;
}

RunOutputPolicy runningPolicy() {
  RunOutputPolicy policy;
  policy.fanOn = true;
  policy.heaterControlAllowed = true;
  return policy;
}

TemperatureControlInput inputAt(int16_t currentTempC, int16_t targetTempC) {
  TemperatureControlInput input;
  input.currentTempC = currentTempC;
  input.targetTempC = targetTempC;
  input.runPolicy = runningPolicy();
  input.deltaSeconds = 1;
  return input;
}

void test_heater_turns_on_at_or_below_lower_hysteresis_threshold() {
  TemperatureControl control;

  const TemperatureControlOutput output =
      control.update(immediateControlConfig(), inputAt(56, 57));

  TEST_ASSERT_TRUE(output.heaterOn);
  TEST_ASSERT_TRUE(control.heaterOn());
}

void test_heater_stays_off_above_lower_hysteresis_threshold() {
  TemperatureControl control;

  const TemperatureControlOutput output =
      control.update(immediateControlConfig(), inputAt(57, 57));

  TEST_ASSERT_FALSE(output.heaterOn);
}

void test_heater_stays_on_inside_hysteresis_band() {
  TemperatureControl control;
  TEST_ASSERT_TRUE(
      control.update(immediateControlConfig(), inputAt(56, 57)).heaterOn);

  const TemperatureControlOutput output =
      control.update(immediateControlConfig(), inputAt(56, 57));

  TEST_ASSERT_TRUE(output.heaterOn);
}

void test_heater_turns_off_at_target_temperature() {
  TemperatureControl control;
  TEST_ASSERT_TRUE(
      control.update(immediateControlConfig(), inputAt(56, 57)).heaterOn);

  const TemperatureControlOutput output =
      control.update(immediateControlConfig(), inputAt(57, 57));

  TEST_ASSERT_FALSE(output.heaterOn);
}

void test_temperature_at_75_is_not_safety_forced_off() {
  TemperatureControl control;
  TEST_ASSERT_TRUE(
      control.update(immediateControlConfig(), inputAt(74, 75)).heaterOn);

  const TemperatureControlOutput output =
      control.update(immediateControlConfig(), inputAt(75, 75));

  TEST_ASSERT_FALSE(output.heaterOn);
  TEST_ASSERT_FALSE(output.forcedOffByTemperature);
}

void test_temperature_above_75_forces_heater_off() {
  TemperatureControl control;
  TEST_ASSERT_TRUE(
      control.update(immediateControlConfig(), inputAt(74, 75)).heaterOn);

  const TemperatureControlOutput output =
      control.update(immediateControlConfig(), inputAt(76, 75));

  TEST_ASSERT_FALSE(output.heaterOn);
  TEST_ASSERT_TRUE(output.forcedOffByTemperature);
}

void test_heater_is_blocked_when_fan_is_off() {
  TemperatureControl control;
  TemperatureControlInput input = inputAt(50, 57);
  input.runPolicy.fanOn = false;

  const TemperatureControlOutput output =
      control.update(immediateControlConfig(), input);

  TEST_ASSERT_FALSE(output.heaterOn);
  TEST_ASSERT_TRUE(output.blockedByRunPolicy);
}

void test_heater_is_blocked_when_control_is_not_allowed() {
  TemperatureControl control;
  TemperatureControlInput input = inputAt(50, 57);
  input.runPolicy.heaterControlAllowed = false;

  const TemperatureControlOutput output =
      control.update(immediateControlConfig(), input);

  TEST_ASSERT_FALSE(output.heaterOn);
  TEST_ASSERT_TRUE(output.blockedByRunPolicy);
}

void test_policy_block_forces_previous_heater_command_off() {
  TemperatureControl control;
  TEST_ASSERT_TRUE(
      control.update(immediateControlConfig(), inputAt(50, 57)).heaterOn);
  TemperatureControlInput input = inputAt(50, 57);
  input.runPolicy.fanOn = false;

  const TemperatureControlOutput output =
      control.update(immediateControlConfig(), input);

  TEST_ASSERT_FALSE(output.heaterOn);
  TEST_ASSERT_FALSE(control.heaterOn());
}

void test_minimum_off_time_delays_turn_on() {
  TemperatureControl control;
  ControlConfig config = immediateControlConfig();
  config.minHeaterOffSeconds = 5;

  TemperatureControlOutput output = control.update(config, inputAt(50, 57));
  TEST_ASSERT_FALSE(output.heaterOn);
  TEST_ASSERT_TRUE(output.waitingForMinOffTime);

  TemperatureControlInput later = inputAt(50, 57);
  later.deltaSeconds = 4;
  output = control.update(config, later);

  TEST_ASSERT_TRUE(output.heaterOn);
}

void test_minimum_on_time_delays_normal_turn_off() {
  TemperatureControl control;
  ControlConfig config = immediateControlConfig();
  config.minHeaterOnSeconds = 5;
  TEST_ASSERT_TRUE(control.update(config, inputAt(50, 57)).heaterOn);

  TemperatureControlOutput output = control.update(config, inputAt(57, 57));
  TEST_ASSERT_TRUE(output.heaterOn);
  TEST_ASSERT_TRUE(output.waitingForMinOnTime);

  TemperatureControlInput later = inputAt(57, 57);
  later.deltaSeconds = 4;
  output = control.update(config, later);

  TEST_ASSERT_FALSE(output.heaterOn);
}

void test_safety_force_off_overrides_minimum_on_time() {
  TemperatureControl control;
  ControlConfig config = immediateControlConfig();
  config.minHeaterOnSeconds = 30;
  TEST_ASSERT_TRUE(control.update(config, inputAt(50, 57)).heaterOn);

  const TemperatureControlOutput output = control.update(config, inputAt(76, 57));

  TEST_ASSERT_FALSE(output.heaterOn);
  TEST_ASSERT_TRUE(output.forcedOffByTemperature);
}

void test_temperature_force_off_is_reported_when_policy_also_blocks_heat() {
  TemperatureControl control;
  TemperatureControlInput input = inputAt(76, 57);
  input.runPolicy.fanOn = false;

  const TemperatureControlOutput output =
      control.update(immediateControlConfig(), input);

  TEST_ASSERT_FALSE(output.heaterOn);
  TEST_ASSERT_TRUE(output.blockedByRunPolicy);
  TEST_ASSERT_TRUE(output.forcedOffByTemperature);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_heater_turns_on_at_or_below_lower_hysteresis_threshold);
  RUN_TEST(test_heater_stays_off_above_lower_hysteresis_threshold);
  RUN_TEST(test_heater_stays_on_inside_hysteresis_band);
  RUN_TEST(test_heater_turns_off_at_target_temperature);
  RUN_TEST(test_temperature_at_75_is_not_safety_forced_off);
  RUN_TEST(test_temperature_above_75_forces_heater_off);
  RUN_TEST(test_heater_is_blocked_when_fan_is_off);
  RUN_TEST(test_heater_is_blocked_when_control_is_not_allowed);
  RUN_TEST(test_policy_block_forces_previous_heater_command_off);
  RUN_TEST(test_minimum_off_time_delays_turn_on);
  RUN_TEST(test_minimum_on_time_delays_normal_turn_off);
  RUN_TEST(test_safety_force_off_overrides_minimum_on_time);
  RUN_TEST(test_temperature_force_off_is_reported_when_policy_also_blocks_heat);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
