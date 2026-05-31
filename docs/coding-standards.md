# Coding Standards

Status: draft baseline.

## General Style

- Use clear, modest C++.
- Prefer simple classes, structs, and enums over complex inheritance.
- Use interfaces only where they protect hardware boundaries or improve testability.
- Keep Arduino-specific calls in hardware adapter modules.
- Keep control logic independent from Arduino APIs where practical.

## Naming And Units

- Use English identifiers and comments.
- Include units in variable names where useful, such as `targetTempC`, `durationMinutes`, `intervalMs`.
- Use stable English fault/warning codes.
- LCD user-facing text is Romanian.

## Memory

- No dynamic allocation in application code.
- Do not use `new`, `delete`, `malloc`, or `free`.
- Avoid Arduino `String`.
- Prefer fixed-size buffers, string literals, `snprintf`, and flash-stored constants where appropriate.
- Avoid hidden heap use in dependencies where reasonably possible.

## Timing

- Avoid blocking delays longer than 100 ms during normal runtime logic.
- Use `millis()`-based timing and a simple cooperative scheduler/event loop.
- Startup/init delays may be allowed if short, deliberate, documented, and safe.

## Safety Logic

- Safety checks must be easy to read and test.
- Add concise comments where timing/fault logic is non-obvious.
- Hard faults may override relay minimum timing and force outputs OFF.
- No heater command may be issued unless fan command is ON.

## Configuration

- Centralize hardware pins, active levels, addresses, and serial settings.
- Separate configuration by category where practical:
  - hardware,
  - safety,
  - control timing,
  - UI timing,
  - logging,
  - calibration,
  - presets.

## EEPROM

- Do not write EEPROM in fast loops.
- Write only on confirmed setting changes, lifecycle events, or scheduled run-state checkpoints.
- Use versioning and validation/checksum.
- Load defaults on invalid config and emit a warning.
