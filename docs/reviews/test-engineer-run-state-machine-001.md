# Test Engineer Review 001: Run State Machine

Date: 2026-06-02

Scope:
- `include/dehydrator/domain/RunStateMachine.h`
- `test/test_run_state_machine/test_run_state_machine.cpp`
- `docs/backlog.md`
- `docs/test-plan.md`
- `docs/logbook.md`

## Summary

The run state machine implementation was structurally sound, but the first test
set did not fully lock down safety-relevant lifecycle behavior. The review
recommended adding tests before committing the branch.

## Findings

### Medium: hard-fault acknowledgement before new run was not directly tested

`start()` only accepts `Idle`, and `fault()` moves the machine to `Fault`, so
the implementation should block a new run until `acknowledgeFault()` returns to
idle. The original tests did not prove that behavior.

Resolution:
- Added `test_new_run_is_blocked_until_fault_is_acknowledged`.
- Updated `REQ-SAFE-010` traceability to show state-machine coverage.

### Medium: fault transition was tested only from paused state

The architecture says hard faults may transition from any runtime state to
`Fault`. The original tests only entered fault from `Paused`.

Resolution:
- Added fault transition tests from `Running`, `Paused`, `FinishCooldown`, and
  `FinishedAlarm`.
- Each test checks heater/fan output policy off, resume disabled, and fault
  alarm policy active.

### Low: confirmed stop accepted several states but only running was tested

`stopConfirmed()` accepts `Running`, `Paused`, `FinishCooldown`, and
`FinishedAlarm`. The original tests only covered `Running`.

Resolution:
- Added stop tests from `Paused`, `FinishCooldown`, and `FinishedAlarm`.
- Added rejection test from `Idle`.

### Low: zero-delta timing boundary was not documented by tests

The nominal cooldown boundary was covered, but a scheduler `update(0)` boundary
was not tested.

Resolution:
- Added zero-delta tests for `Running` and `FinishCooldown`.

## Verification

After applying the review findings:
- `platformio test -e native`: 58 tests passed
- `platformio run -e megaatmega2560`: success

## Verdict

Ready to commit after rerunning native tests and Mega2560 build successfully.
