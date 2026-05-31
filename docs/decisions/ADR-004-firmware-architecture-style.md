# ADR-004: Firmware Architecture Style

## Status

Accepted.

## Context

The firmware must be safe, maintainable, understandable, and testable. The user wants C++ but not over-engineered abstraction.

## Decision

Use clear, modest C++ with separation between pure logic and hardware adapters.

Core logic should be testable without Mega2560 hardware. Hardware libraries should be wrapped behind project-owned interfaces. Use a cooperative scheduler/event loop with `millis()` timing.

Forbid dynamic allocation in application code. Avoid Arduino `String`. Avoid blocking delays longer than 100 ms during normal runtime logic.

## Consequences

- Unit testing becomes practical.
- Hardware libraries can be changed later.
- The code remains embedded-friendly.
- Slightly more initial structure is required than a single `.ino` loop.
