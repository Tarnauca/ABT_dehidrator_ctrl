# Requirements Analyst Review 001

Date: 2026-05-31

Scope:

- `docs/requirements.md`
- `docs/project-context.md`
- Relevant ADRs as context

Agent mode: read-only review.

## Summary

`docs/requirements.md` is a strong first baseline: IDs are stable, categories are clear, safety is first-class, and future/deferred items are separated. The main gaps are not structural; they are about making several requirements more testable and adding requirements that existed in context/ADRs but were not yet captured directly.

## Findings

1. `REQ-FUNC-001` says manual on/off mode, but does not define whether manual controls heater and fan separately, or whether fan is forced on when heater is on.
2. `REQ-FUNC-005` says pause/resume is supported, but does not define pause behavior.
3. `REQ-FUNC-006` says stop/cancel requires confirmation, but does not specify resulting behavior.
4. `REQ-FUNC-003` is directionally good but ambiguous. It should define the initial fluctuating algorithm.
5. `REQ-SAFE-002` should clarify whether heater forced-off threshold is `>75 C` or `>=75 C`.
6. `REQ-SAFE-006` should clarify whether the 5-minute no-temperature-rise rule means continuous heater ON time or accumulated heater ON time.
7. `REQ-SAFE-007` may false-trigger after heater shutoff due to thermal inertia; it needs a grace period or explicit interpretation.
8. Startup self-checks are underspecified.
9. Finish alarm should include the 3-minute fan cooldown sequence before alarm.
10. Secondary serial behavior should explicitly say output-only/no commands.
11. Persistence requirements should capture the user-confirmed resume flow after power loss/brown-out.
12. Testing requirements are useful but broad and should later gain acceptance criteria or traceability to safety cases.

## Recommendations

- Add explicit pause, resume, stop/cancel, and finish sequence requirements.
- Clarify fluctuating mode with an initial time-based alternating low/high algorithm.
- Clarify temperature threshold semantics and fault timing semantics.
- Add startup self-check requirements.
- Add interrupted-run resume confirmation requirement.
- Add secondary telemetry output-only requirement.
- Add warning behavior and fault/warning code requirements.
- Add requirement change rules.

## Decisions From User Follow-Up

The user agreed with these clarifications:

1. Heater forced-off threshold is `>75 C`, while `75 C` remains the maximum allowed setpoint.
2. No-temperature-rise detection uses accumulated heater ON command time.
3. Heater-stuck-ON detection should include a 2-minute post-heater-off grace period before evaluating the `3 C over 5 min` rule.
4. Finish alarm starts only after the 3-minute cooldown completes.

## Required Updates

Implemented in `docs/requirements.md` on branch `docs/requirements-review`.

## Open Questions

- Exact default fluctuating phase timing remains TBD.
- Safety acceptance tests should be added during the test-plan refinement phase.

## Files Changed

None by the review agent. Follow-up edits were made by Codex after user confirmation.
