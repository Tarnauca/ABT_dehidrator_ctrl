#include <unity.h>

#include "dehydrator/domain/ProfileEngine.h"
#include "dehydrator/domain/RunState.h"
#include "dehydrator/domain/RunStateMachine.h"

using dehydrator::ProfileConfig;
using dehydrator::ProfileMode;
using dehydrator::RunOutputPolicy;
using dehydrator::RunState;
using dehydrator::RunStateMachine;
using dehydrator::RunStateSnapshot;

ProfileConfig fixedProfile() {
  ProfileConfig profile;
  profile.mode = ProfileMode::Fixed;
  profile.targetTempC = 57;
  profile.durationMinutes = 10;
  return profile;
}

void assertOutputsOff(const RunStateMachine& machine) {
  const RunOutputPolicy policy = machine.outputPolicy();

  TEST_ASSERT_FALSE(policy.fanOn);
  TEST_ASSERT_FALSE(policy.heaterControlAllowed);
}

void assertFaultPolicy(const RunStateMachine& machine) {
  const RunStateSnapshot snapshot = machine.snapshot();
  const RunOutputPolicy policy = machine.outputPolicy();

  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Fault),
                    static_cast<int>(snapshot.state));
  TEST_ASSERT_FALSE(snapshot.resumeAllowed);
  TEST_ASSERT_FALSE(policy.fanOn);
  TEST_ASSERT_FALSE(policy.heaterControlAllowed);
  TEST_ASSERT_TRUE(policy.faultAlarmOn);
}

void test_starts_valid_profile_from_idle() {
  RunStateMachine machine;

  const bool started = machine.start(fixedProfile());
  const RunStateSnapshot snapshot = machine.snapshot();
  const RunOutputPolicy policy = machine.outputPolicy();

  TEST_ASSERT_TRUE(started);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Running),
                    static_cast<int>(snapshot.state));
  TEST_ASSERT_TRUE(snapshot.resumeAllowed);
  TEST_ASSERT_TRUE(snapshot.profileValid);
  TEST_ASSERT_TRUE(policy.fanOn);
  TEST_ASSERT_TRUE(policy.heaterControlAllowed);
}

void test_rejects_invalid_profile_start() {
  RunStateMachine machine;
  ProfileConfig profile = fixedProfile();
  profile.targetTempC = 76;

  const bool started = machine.start(profile);
  const RunStateSnapshot snapshot = machine.snapshot();

  TEST_ASSERT_FALSE(started);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Idle),
                    static_cast<int>(snapshot.state));
  assertOutputsOff(machine);
}

void test_rejects_start_while_already_running() {
  RunStateMachine machine;

  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  TEST_ASSERT_FALSE(machine.start(fixedProfile()));
}

void test_running_update_advances_active_elapsed_time() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));

  machine.update(42);

  TEST_ASSERT_EQUAL_UINT32(42, machine.snapshot().activeElapsedSeconds);
}

void test_zero_update_is_harmless_while_running() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  machine.update(30);

  machine.update(0);

  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Running),
                    static_cast<int>(machine.snapshot().state));
  TEST_ASSERT_EQUAL_UINT32(30, machine.snapshot().activeElapsedSeconds);
}

void test_pause_turns_outputs_off_and_keeps_resume_allowed() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));

  const bool paused = machine.pause();
  const RunStateSnapshot snapshot = machine.snapshot();

  TEST_ASSERT_TRUE(paused);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Paused),
                    static_cast<int>(snapshot.state));
  TEST_ASSERT_TRUE(snapshot.resumeAllowed);
  assertOutputsOff(machine);
}

void test_paused_update_does_not_advance_active_elapsed_time() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  machine.update(30);
  TEST_ASSERT_TRUE(machine.pause());

  machine.update(90);

  TEST_ASSERT_EQUAL_UINT32(30, machine.snapshot().activeElapsedSeconds);
}

void test_resume_continues_running_from_same_elapsed_time() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  machine.update(30);
  TEST_ASSERT_TRUE(machine.pause());

  const bool resumed = machine.resume();
  machine.update(15);

  TEST_ASSERT_TRUE(resumed);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Running),
                    static_cast<int>(machine.snapshot().state));
  TEST_ASSERT_EQUAL_UINT32(45, machine.snapshot().activeElapsedSeconds);
}

void test_profile_completion_enters_finish_cooldown() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));

  machine.update(10UL * 60UL);
  const RunStateSnapshot snapshot = machine.snapshot();
  const RunOutputPolicy policy = machine.outputPolicy();

  TEST_ASSERT_EQUAL(static_cast<int>(RunState::FinishCooldown),
                    static_cast<int>(snapshot.state));
  TEST_ASSERT_FALSE(snapshot.resumeAllowed);
  TEST_ASSERT_TRUE(policy.fanOn);
  TEST_ASSERT_FALSE(policy.heaterControlAllowed);
}

void test_finish_cooldown_runs_for_three_minutes_before_alarm() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  machine.update(10UL * 60UL);

  machine.update(RunStateMachine::FINISH_COOLDOWN_SECONDS - 1U);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::FinishCooldown),
                    static_cast<int>(machine.snapshot().state));

  machine.update(1);
  const RunOutputPolicy policy = machine.outputPolicy();

  TEST_ASSERT_EQUAL(static_cast<int>(RunState::FinishedAlarm),
                    static_cast<int>(machine.snapshot().state));
  TEST_ASSERT_FALSE(policy.fanOn);
  TEST_ASSERT_TRUE(policy.finishAlarmOn);
}

void test_zero_update_is_harmless_during_finish_cooldown() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  machine.update(10UL * 60UL);
  machine.update(30);

  machine.update(0);

  TEST_ASSERT_EQUAL(static_cast<int>(RunState::FinishCooldown),
                    static_cast<int>(machine.snapshot().state));
  TEST_ASSERT_EQUAL_UINT16(30, machine.snapshot().cooldownElapsedSeconds);
}

void test_finished_alarm_acknowledge_returns_to_idle() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  machine.update(10UL * 60UL);
  machine.update(RunStateMachine::FINISH_COOLDOWN_SECONDS);

  const bool acknowledged = machine.acknowledgeFinished();

  TEST_ASSERT_TRUE(acknowledged);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Idle),
                    static_cast<int>(machine.snapshot().state));
  assertOutputsOff(machine);
}

void test_confirmed_stop_returns_to_idle_without_cooldown() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  machine.update(30);

  const bool stopped = machine.stopConfirmed();
  const RunStateSnapshot snapshot = machine.snapshot();

  TEST_ASSERT_TRUE(stopped);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Idle),
                    static_cast<int>(snapshot.state));
  TEST_ASSERT_FALSE(snapshot.resumeAllowed);
  TEST_ASSERT_EQUAL_UINT32(0, snapshot.activeElapsedSeconds);
  assertOutputsOff(machine);
}

void test_confirmed_stop_from_paused_returns_to_idle() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  TEST_ASSERT_TRUE(machine.pause());

  const bool stopped = machine.stopConfirmed();

  TEST_ASSERT_TRUE(stopped);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Idle),
                    static_cast<int>(machine.snapshot().state));
  assertOutputsOff(machine);
}

void test_confirmed_stop_from_finish_cooldown_returns_to_idle() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  machine.update(10UL * 60UL);

  const bool stopped = machine.stopConfirmed();

  TEST_ASSERT_TRUE(stopped);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Idle),
                    static_cast<int>(machine.snapshot().state));
  assertOutputsOff(machine);
}

void test_confirmed_stop_from_finished_alarm_returns_to_idle() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  machine.update(10UL * 60UL);
  machine.update(RunStateMachine::FINISH_COOLDOWN_SECONDS);

  const bool stopped = machine.stopConfirmed();

  TEST_ASSERT_TRUE(stopped);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Idle),
                    static_cast<int>(machine.snapshot().state));
  assertOutputsOff(machine);
}

void test_confirmed_stop_is_rejected_from_idle() {
  RunStateMachine machine;

  TEST_ASSERT_FALSE(machine.stopConfirmed());
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Idle),
                    static_cast<int>(machine.snapshot().state));
}

void test_fault_from_paused_forces_outputs_off_and_disables_resume() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  TEST_ASSERT_TRUE(machine.pause());

  machine.fault();

  assertFaultPolicy(machine);
}

void test_fault_from_running_forces_outputs_off_and_disables_resume() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));

  machine.fault();

  assertFaultPolicy(machine);
}

void test_fault_from_finish_cooldown_forces_outputs_off_and_disables_resume() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  machine.update(10UL * 60UL);

  machine.fault();

  assertFaultPolicy(machine);
}

void test_fault_from_finished_alarm_forces_outputs_off_and_disables_resume() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  machine.update(10UL * 60UL);
  machine.update(RunStateMachine::FINISH_COOLDOWN_SECONDS);

  machine.fault();

  assertFaultPolicy(machine);
}

void test_new_run_is_blocked_until_fault_is_acknowledged() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  machine.fault();

  TEST_ASSERT_FALSE(machine.start(fixedProfile()));
  TEST_ASSERT_TRUE(machine.acknowledgeFault());
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
}

void test_fault_acknowledge_returns_to_idle() {
  RunStateMachine machine;
  TEST_ASSERT_TRUE(machine.start(fixedProfile()));
  machine.fault();

  const bool acknowledged = machine.acknowledgeFault();

  TEST_ASSERT_TRUE(acknowledged);
  TEST_ASSERT_EQUAL(static_cast<int>(RunState::Idle),
                    static_cast<int>(machine.snapshot().state));
  assertOutputsOff(machine);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_starts_valid_profile_from_idle);
  RUN_TEST(test_rejects_invalid_profile_start);
  RUN_TEST(test_rejects_start_while_already_running);
  RUN_TEST(test_running_update_advances_active_elapsed_time);
  RUN_TEST(test_zero_update_is_harmless_while_running);
  RUN_TEST(test_pause_turns_outputs_off_and_keeps_resume_allowed);
  RUN_TEST(test_paused_update_does_not_advance_active_elapsed_time);
  RUN_TEST(test_resume_continues_running_from_same_elapsed_time);
  RUN_TEST(test_profile_completion_enters_finish_cooldown);
  RUN_TEST(test_finish_cooldown_runs_for_three_minutes_before_alarm);
  RUN_TEST(test_zero_update_is_harmless_during_finish_cooldown);
  RUN_TEST(test_finished_alarm_acknowledge_returns_to_idle);
  RUN_TEST(test_confirmed_stop_returns_to_idle_without_cooldown);
  RUN_TEST(test_confirmed_stop_from_paused_returns_to_idle);
  RUN_TEST(test_confirmed_stop_from_finish_cooldown_returns_to_idle);
  RUN_TEST(test_confirmed_stop_from_finished_alarm_returns_to_idle);
  RUN_TEST(test_confirmed_stop_is_rejected_from_idle);
  RUN_TEST(test_fault_from_paused_forces_outputs_off_and_disables_resume);
  RUN_TEST(test_fault_from_running_forces_outputs_off_and_disables_resume);
  RUN_TEST(test_fault_from_finish_cooldown_forces_outputs_off_and_disables_resume);
  RUN_TEST(test_fault_from_finished_alarm_forces_outputs_off_and_disables_resume);
  RUN_TEST(test_new_run_is_blocked_until_fault_is_acknowledged);
  RUN_TEST(test_fault_acknowledge_returns_to_idle);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
