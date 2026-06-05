# Manual Program Modes Review 001

Date: 2026-06-04

Branch: `feat/manual-program-modes`

Scope:

- Manual `Constant`, `Boost`, and `Fluctuant` modes.
- Dynamic manual-program LCD UI.
- `ProfileEngine` boost support.
- Manual profile execution through `PresetRunController`.
- Requirements, architecture, UI, test-plan, backlog, and logbook updates.

## Review Set

### Requirements / Documentation Review

Reviewer: requirements/documentation sub-agent.

Findings and resolution:

- `REQ-FUNC-019` did not explicitly state that `Boost` continues as constant
  operation after the boost phase. Resolved in `docs/requirements.md`.
- UI traceability rows did not include manual-program tests for `Inapoi`,
  title/row layout, and temperature formatting. Resolved in `docs/test-plan.md`.
- Acceptance tests did not mention manual `Constant`, `Boost`, and `Fluctuant`
  workflows. Resolved in `docs/test-plan.md`.
- Architecture wording still described only fixed/fluctuating profiles.
  Resolved in `docs/architecture.md`.
- Logbook had older simple-fluctuation wording that is now superseded. Resolved
  by adding an explicit superseded note instead of rewriting history.
- Direct-output test code/log names still used manual terminology. Resolved by
  renaming the active code path to `TestModeController`, `LcdTestView`, and
  `test_fan` / `test_heat` UI log tokens.

### Architecture Review

Reviewer: system-architecture sub-agent.

Findings and resolution:

- `Tsup` and `Tinf` could be inverted semantically around the reference while
  still satisfying generic `upper >= lower`. Resolved by constraining `Tsup`
  to reference...reference+10 C and `Tinf` to reference-10 C...reference in
  `ManualProgramController`.
- Pending manual replacement confirmation used the mutable manual controller
  state instead of a captured pending profile. Resolved by storing
  `pendingManualProgramProfile` when `Start` is pressed.
- Dynamic field lists were duplicated in controller and LCD view. Resolved by
  exposing `ManualProgramController::fieldCountForMode()` and `fieldAtIndex()`
  and using them from `LcdManualProgramView`.
- Manual-specific constraints are UI/controller-level rather than
  domain-level. Accepted and documented in `docs/architecture.md`: future
  non-UI callers must use the manual controller or an equivalent validator.

### Test Engineering Review

Reviewer: test-engineering sub-agent.

Findings and resolution:

- Added mode-selection edge tests for non-wrapping mode edits.
- Added boost lower-bound, upper-bound, 75 C target, boost-duration, and
  total-duration dependency tests.
- Added fluctuating `Tsup`/`Tinf` side-of-reference, +/-10 C, phase-duration,
  and reference-edit blocking tests.
- Added boost profile completion test.
- Added manual constant/fluctuating run tests in `PresetRunController`.
- Added manual run context cleanup tests for stop and finish acknowledgement.
- Added LCD end-of-list tests proving `Start` and `Inapoi` render correctly for
  all dynamic manual modes.

## Verification

- `platformio test -e native`: 222/222 passed.
- `platformio run -e megaatmega2560`: success.

## Follow-Up

- A lightweight app-level input-flow test for `main.cpp` replacement
  confirmation would still be useful later, once an application coordinator is
  extracted from `main.cpp`. It is not required for this branch because the
  pure controllers and run coordinator are covered.
