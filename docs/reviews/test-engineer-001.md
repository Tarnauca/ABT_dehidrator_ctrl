# Test Engineer Review 001

Date: 2026-05-31

Scope:

- `docs/test-plan.md`
- `docs/requirements.md`
- `docs/architecture.md`

Agent mode: read-only review.

## Summary

`docs/test-plan.md` was aligned with the requirements and architecture direction, but still too high-level for implementation. Before coding starts, it should become a traceable verification plan with requirement IDs mapped to unit, simulation, bench, and acceptance tests.

The strongest next improvement was to define the first native-testable modules and their fake dependencies: fake clock, fake sensors, fake outputs, fake persistence, and log capture.

## Findings

1. Acceptance tests were only a placeholder and should reference concrete requirement IDs.
2. Unit tests listed modules, but not specific behavior or state transitions.
3. Simulation tests mentioned fakes, but did not define required fake interfaces.
4. No-temperature-rise tests need accumulated heater ON time behavior.
5. Heater-stuck-ON tests need the 2-minute grace period and 3 C / 5 min rule.
6. Heater forced-OFF tests should cover both 75 C and 76 C.
7. Reset tests should distinguish power loss/brown-out resume offer from watchdog non-resume behavior.
8. Secondary serial verification should test output-only mirroring and no command handling.
9. Serial log formatting tests should include stable English fault/warning codes and parseable records.
10. EEPROM tests should cover validation, defaulting, snapshot interval limits, faulted non-resumable state, and boot-write caution.

## Recommendations

- Add a requirement coverage matrix.
- Define first native unit test groups.
- Define `FakeClock`, `FakeSensorReader`, `FakeOutputController`, `FakePersistentStore`, and `FakeLogSink`.
- Add boundary tests for safety values and timing thresholds.

## Decisions From Follow-Up

Accepted defaults:

1. Create the traceability matrix now as documentation before code exists.
2. Require native tests to pass before firmware upload for affected pure logic changes once implementation begins.
3. Make the first code branch create only the testable skeleton and one sample unit test before adding substantial behavior.

## Required Updates

Implemented in `docs/test-plan.md` on branch `docs/architecture-review`.

## Open Questions

None for this branch.

## Files Changed

None by the review agent. Follow-up edits were made by Codex after review.
