# System Architect Review: Logging Abstraction 001

Date: 2026-06-01

Scope:

- `include/dehydrator/logging/*`
- `src/main.cpp`
- `test/test_logging/*`
- `docs/backlog.md`
- Architecture, requirements, coding standards, and ADR context

Agent mode: read-only review.

## Summary

The logging abstraction is architecturally sound for this stage. It uses fixed caller-owned storage, fixed formatting buffers, no Arduino `String`, no dynamic allocation, and supports USB plus secondary serial mirroring without introducing command input. Arduino-specific code is isolated to `src/main.cpp`, which is acceptable for the current composition shell.

## Findings

1. Formatter return values in `src/main.cpp` were ignored, so truncated future log lines could still be emitted as if valid.
2. Public formatter APIs did not document or guard whether `type` and `detail` may be null.
3. `LogDispatcher::writeLine()` assumed a non-null line.
4. `ArduinoSerialLogSink` lives in `main.cpp`. This is acceptable for the shell, but should move to the hardware-facing layer once hardware adapters are introduced.

## Recommendations

- Emit a stable warning line when formatting fails, otherwise skip if even the warning cannot fit.
- Guard or document nullable public API inputs.
- Keep the fixed-array dispatcher design.
- Move Arduino serial sink into the hardware adapter layer in a later hardware-structure branch.

## Required Updates

Implemented in this branch:

- Formatter null tokens emit `null`.
- Dispatcher ignores null lines.
- `main.cpp` emits stable `WARN code=log_truncated ...` fallback lines on formatting failure.
- Native tests cover null token handling and null dispatcher writes.

## Open Questions

None for this branch.

## Files Changed

None by the review agent. Follow-up edits were made by Codex after review.
