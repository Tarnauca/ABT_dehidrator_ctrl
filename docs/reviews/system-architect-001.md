# System Architect Review 001

Date: 2026-05-31

Scope:

- `docs/architecture.md`
- `docs/requirements.md`
- `docs/decisions/*.md`
- `docs/project-context.md`

Agent mode: read-only review.

## Summary

Architecture is directionally ready and consistent with ADR-004/ADR-005: pure logic versus hardware adapters, cooperative scheduler, no dynamic allocation, no long blocking delays, testability, and Mega2560-only scope are aligned.

The main gap was that `docs/architecture.md` was still high-level. Before firmware skeleton work, it needed sharper module ownership, data flow, interface responsibilities, configuration layout, and PlatformIO/native test implications.

## Findings

1. `ControlStateMachine` was listed but did not define the authoritative state model.
2. `TemperatureControl` owned hysteresis and relay timing, but architecture did not state where the heater/fan safety invariant is enforced.
3. `FaultDetector` was broad and needed explicit inputs and time-baseline semantics.
4. Configuration categories were listed but no concrete config ownership was proposed.
5. Interfaces were conceptual and needed an initial minimal set.
6. Native testing was mentioned but not supported by a source layout.
7. Architecture rules forbade heap/delay problems but did not mention fixed buffers for logging/UI formatting.
8. Secondary serial mirroring required multi-sink logging clarity.

## Recommendations

- Define a concrete runtime state model.
- Add a proposed source tree.
- Define minimal interface responsibilities.
- Clarify safety-invariant enforcement points.
- Define configuration ownership.
- Clarify native-testable layout.
- Clarify multi-sink logging.
- Add fixed-buffer/no-heap formatting guidance.

## Decisions From Follow-Up

Accepted defaults:

1. `RelayOutputs` should defensively reject or sanitize `heater=ON, fan=OFF`.
2. Finish cooldown should be represented as its own `ControlStateMachine` state.
3. Logging should fan out through a simple `LogDispatcher` to USB and secondary serial sinks, without dynamic allocation.

## Required Updates

Implemented in `docs/architecture.md` on branch `docs/architecture-review`.

## Open Questions

None for this branch.

## Files Changed

None by the review agent. Follow-up edits were made by Codex after review.
