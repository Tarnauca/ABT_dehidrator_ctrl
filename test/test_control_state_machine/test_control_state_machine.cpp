#include <unity.h>

#include "dehydrator/domain/ControlStateMachine.h"
#include "dehydrator/domain/RunState.h"

using dehydrator::ControlStateMachine;
using dehydrator::ControlStartupInput;
using dehydrator::ControlStateSnapshot;
using dehydrator::RunState;

ControlStartupInput validStartupInput() {
  ControlStartupInput input;
  input.configValid = true;
  input.outputsSafe = true;
  input.primarySensorValid = true;
  input.buttonSafe = true;
  input.watchdogResetDuringRun = false;
  input.interruptedRunAvailable = false;
  return input;
}

void assertState(const ControlStateMachine& machine, RunState expected) {
  TEST_ASSERT_EQUAL(static_cast<int>(expected),
                    static_cast<int>(machine.snapshot().state));
}

void test_boot_to_idle_when_startup_checks_pass() {
  ControlStateMachine machine;
  machine.enterBoot();

  const auto result = machine.completeSelfCheck(validStartupInput());
  const ControlStateSnapshot snapshot = machine.snapshot();

  TEST_ASSERT_TRUE(result.passed);
  TEST_ASSERT_FALSE(result.hardFault);
  TEST_ASSERT_NULL(result.faultToken);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Idle),
                    static_cast<int>(snapshot.state));
  TEST_ASSERT_TRUE(snapshot.startupChecked);
  TEST_ASSERT_FALSE(snapshot.resumeOfferAvailable);
  TEST_ASSERT_FALSE(snapshot.hardFault);
  TEST_ASSERT_EQUAL_STRING("idle", machine.stateToken());
}

void test_startup_enters_resume_offer_when_snapshot_exists() {
  ControlStateMachine machine;
  ControlStartupInput input = validStartupInput();
  input.interruptedRunAvailable = true;

  const auto result = machine.completeSelfCheck(input);

  TEST_ASSERT_TRUE(result.passed);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::ResumeOffer),
                    static_cast<int>(machine.snapshot().state));
  TEST_ASSERT_TRUE(machine.snapshot().resumeOfferAvailable);
  TEST_ASSERT_EQUAL_STRING("resume_offer", machine.stateToken());
}

void test_bad_config_latches_fault() {
  ControlStateMachine machine;
  ControlStartupInput input = validStartupInput();
  input.configValid = false;

  const auto result = machine.completeSelfCheck(input);

  TEST_ASSERT_FALSE(result.passed);
  TEST_ASSERT_TRUE(result.hardFault);
  TEST_ASSERT_EQUAL_STRING("config_invalid", result.faultToken);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Fault),
                    static_cast<int>(machine.snapshot().state));
  TEST_ASSERT_EQUAL_STRING("fault", machine.stateToken());
}

void test_unsafe_outputs_latch_fault() {
  ControlStateMachine machine;
  ControlStartupInput input = validStartupInput();
  input.outputsSafe = false;

  const auto result = machine.completeSelfCheck(input);

  TEST_ASSERT_FALSE(result.passed);
  TEST_ASSERT_TRUE(result.hardFault);
  TEST_ASSERT_EQUAL_STRING("outputs_not_safe", result.faultToken);
  TEST_ASSERT_TRUE(machine.snapshot().hardFault);
}

void test_stuck_button_latches_fault() {
  ControlStateMachine machine;
  ControlStartupInput input = validStartupInput();
  input.buttonSafe = false;

  const auto result = machine.completeSelfCheck(input);

  TEST_ASSERT_FALSE(result.passed);
  TEST_ASSERT_TRUE(result.hardFault);
  TEST_ASSERT_EQUAL_STRING("button_stuck", result.faultToken);
}

void test_watchdog_reset_latches_fault() {
  ControlStateMachine machine;
  ControlStartupInput input = validStartupInput();
  input.watchdogResetDuringRun = true;

  const auto result = machine.completeSelfCheck(input);

  TEST_ASSERT_FALSE(result.passed);
  TEST_ASSERT_TRUE(result.hardFault);
  TEST_ASSERT_EQUAL_STRING("watchdog_reset_during_run", result.faultToken);
}

void test_resume_offer_can_be_dismissed() {
  ControlStateMachine machine;
  ControlStartupInput input = validStartupInput();
  input.interruptedRunAvailable = true;
  machine.completeSelfCheck(input);

  const bool dismissed = machine.dismissResumeOffer();

  TEST_ASSERT_TRUE(dismissed);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Idle),
                    static_cast<int>(machine.snapshot().state));
  TEST_ASSERT_FALSE(machine.snapshot().resumeOfferAvailable);
}

void test_fault_can_be_acknowledged_back_to_idle() {
  ControlStateMachine machine;
  ControlStartupInput input = validStartupInput();
  input.outputsSafe = false;
  machine.completeSelfCheck(input);

  const bool acknowledged = machine.acknowledgeFault();

  TEST_ASSERT_TRUE(acknowledged);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Idle),
                    static_cast<int>(machine.snapshot().state));
  TEST_ASSERT_FALSE(machine.snapshot().hardFault);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_boot_to_idle_when_startup_checks_pass);
  RUN_TEST(test_startup_enters_resume_offer_when_snapshot_exists);
  RUN_TEST(test_bad_config_latches_fault);
  RUN_TEST(test_unsafe_outputs_latch_fault);
  RUN_TEST(test_stuck_button_latches_fault);
  RUN_TEST(test_watchdog_reset_latches_fault);
  RUN_TEST(test_resume_offer_can_be_dismissed);
  RUN_TEST(test_fault_can_be_acknowledged_back_to_idle);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
