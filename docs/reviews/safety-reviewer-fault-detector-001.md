# Safety Reviewer Review 001: Fault Detector

Date: 2026-06-02

Scope:
- `include/dehydrator/domain/FaultDetector.h`
- `include/dehydrator/config/RuntimeConfig.h`
- `test/test_fault_detector/test_fault_detector.cpp`
- `test/test_config/test_config.cpp`
- `docs/backlog.md`
- `docs/test-plan.md`
- `docs/logbook.md`

## Summary

The initial fault detector structure and first-fault latching approach were
good, but the first draft did not fully match two safety-critical timing
requirements. The no-rise detector used continuous heater-ON time rather than
accumulated heater-ON command time, and stuck-heater detection could expire
after the first monitoring window.

## Findings

### High: no-rise logic contradicted accumulated heater-ON time

The first implementation reset no-rise timing whenever heater command was OFF.
That meant relay cycling could indefinitely avoid the no-temperature-rise hard
fault, contradicting `REQ-SAFE-006`.

Resolution:
- Changed no-rise tracking to accumulate only while heater command is ON, but
  keep accumulated time across heater-OFF intervals.
- Reset no-rise baseline/timing after the required 2 C rise is observed.
- Added a relay-cycled accumulation test.

### High: stuck-heater detection expired after the first window

The first implementation checked the 3 C rise only while the monitor elapsed
time was less than or equal to the first 5-minute window. After that, future
updates could never fault.

Resolution:
- Added 5-minute monitoring-window rebaseline behavior.
- Added a test proving a later 3 C rise after rebaseline still faults.

### Medium: stuck-heater monitoring started from idle

The first implementation started heater-OFF monitoring even if the heater had
never previously been commanded ON.

Resolution:
- Added state so stuck-heater monitoring begins only after the heater was
  previously commanded ON and then OFF.
- Added an idle/no-prior-heater test to prevent false positives.

### Medium: docs temporarily overclaimed fault-detector completion

The test-plan and backlog were marked complete before the safety timing
semantics were corrected.

Resolution:
- Corrected the implementation and tests before commit.
- Kept Phase 5 completion because the corrected detector is now covered by
  native tests.

### Low: watchdog reset coverage wording needed precision

The detector covers a watchdog-reset-during-run input, but persistence and
resume behavior are future integration work.

Resolution:
- Updated the test-plan wording to say detector input is covered while
  resume/persistence behavior remains pending.

## Verification

After applying the review findings:
- `platformio test -e native`: 93 tests passed
- `platformio run -e megaatmega2560`: success

## Verdict

Ready to commit after review findings were fixed and verification passed.
