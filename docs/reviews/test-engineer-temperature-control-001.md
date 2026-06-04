# Test Engineer Review 001: Hysteresis Temperature Control

Date: 2026-06-02

Scope:
- `include/dehydrator/config/RuntimeConfig.h`
- `include/dehydrator/domain/TemperatureControl.h`
- `test/test_temperature_control/test_temperature_control.cpp`
- `test/test_config/test_config.cpp`
- `docs/backlog.md`
- `docs/test-plan.md`
- `docs/logbook.md`

## Summary

The hysteresis temperature controller was functionally sound, but the first
draft disabled the project-agreed production relay minimum timing by default
and slightly overstated full-system coverage for the heater/fan invariant.

## Findings

### Medium: default relay minimum timing contradicted project docs

The controller supported minimum ON/OFF timing, but the first default
configuration used `0 s` for both values. Project context and risk mitigation
notes already call for 10 s minimum heater ON/OFF timing.

Resolution:
- Changed `CONTROL.minHeaterOnSeconds` to `10`.
- Changed `CONTROL.minHeaterOffSeconds` to `10`.
- Added config tests for both defaults.

### Medium: REQ-SAFE-004 wording overstated full-system coverage

The temperature-control layer blocks heater requests when fan/control permission
is missing, and the existing command sanitizer also forces heater OFF if fan is
OFF. The hardware relay adapter and bench test are still future work.

Resolution:
- Updated the test-plan row to say temperature-control policy tests are covered,
  the command sanitizer exists, and relay adapter bench verification remains
  pending.

### Low: config test missed relay timing defaults

The first config test covered only the 75 C force-off threshold.

Resolution:
- Added `test_control_minimum_relay_timing_matches_baseline`.

### Low: over-temperature reason was hidden when run policy also blocked heat

When fan/control permission blocked heat and NTC temperature was above the
force-off threshold, the first implementation reported only the policy block.

Resolution:
- `TemperatureControl` now reports `forcedOffByTemperature` independently of
  policy blocking.
- Added a test for combined policy block plus over-temperature.

## Verification

After applying the review findings:
- `platformio test -e native`: 73 tests passed
- `platformio run -e megaatmega2560`: success

## Verdict

Ready to commit after rerunning native tests and Mega2560 build successfully.
