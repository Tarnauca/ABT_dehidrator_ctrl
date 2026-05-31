#include <unity.h>

#include "dehydrator/domain/RunState.h"
#include "dehydrator/interfaces/OutputController.h"

using dehydrator::OutputCommand;
using dehydrator::RunState;
using dehydrator::isHeaterFanOffState;
using dehydrator::sanitizeOutputCommand;

void test_fault_state_is_heater_fan_off_state() {
  TEST_ASSERT_TRUE(isHeaterFanOffState(RunState::Fault));
}

void test_running_state_is_not_intrinsically_heater_fan_off() {
  TEST_ASSERT_FALSE(isHeaterFanOffState(RunState::Running));
}

void test_heater_is_sanitized_off_when_fan_is_off() {
  OutputCommand command;
  command.heaterOn = true;
  command.fanOn = false;

  const OutputCommand sanitized = sanitizeOutputCommand(command);

  TEST_ASSERT_FALSE(sanitized.heaterOn);
  TEST_ASSERT_FALSE(sanitized.fanOn);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_fault_state_is_heater_fan_off_state);
  RUN_TEST(test_running_state_is_not_intrinsically_heater_fan_off);
  RUN_TEST(test_heater_is_sanitized_off_when_fan_is_off);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
