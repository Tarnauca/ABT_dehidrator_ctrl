# Safety Reviewer Review 001: Relay Output Adapter

Date: 2026-06-02

Scope:
- `include/dehydrator/hardware/RelayOutputs.h`
- `include/dehydrator/interfaces/DigitalOutput.h`
- `test/test_relay_outputs/test_relay_outputs.cpp`
- `docs/backlog.md`
- `docs/hardware.md`
- `docs/test-plan.md`
- `docs/logbook.md`

## Summary

The relay adapter structure was good, but the first draft had two
safety-relevant hardware edge cases: active-low relay startup could pulse ON,
and heater could be energized if the fan relay pin was unassigned.

## Findings

### High: active-low relay startup could briefly energize outputs

The first implementation configured relay pins as outputs before driving their
inactive levels. On AVR-style GPIO, enabling output while the output latch is
low can briefly energize active-low relay boards.

Resolution:
- Changed `DigitalOutput::configureOutput()` to accept an initial pin level.
- `RelayOutputs::begin()` now passes the inactive level during output
  configuration.
- Added tests for active-high and active-low startup initial levels.

### High: partial pin assignment could violate heater/fan invariant

If the heater relay pin was assigned but the fan relay pin was unassigned, a
logical `fanOn=true, heaterOn=true` command could still write heater ON while
no fan relay output existed.

Resolution:
- `RelayOutputs::apply()` now allows heater ON only when the fan relay pin is
  assigned.
- Added a mixed-assignment test proving heater is not energized when fan pin is
  unassigned.

### Medium: active-low ON behavior was under-tested

The first tests covered active-low OFF levels but not active-low ON ordering.

Resolution:
- Added active-low ON test proving fan low is written before heater low.

### Low: documentation needed precision

Hardware notes needed to explain startup preloading and partial-assignment
behavior after the fixes.

Resolution:
- Updated hardware notes and logbook.

## Verification

After applying review findings:
- `platformio test -e native`: 116 tests passed
- `platformio run -e megaatmega2560`: success

## Verdict

Ready to commit after review findings were fixed and verification passed.
