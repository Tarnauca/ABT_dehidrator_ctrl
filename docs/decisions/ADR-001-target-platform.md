# ADR-001: Target Platform

## Status

Accepted.

## Context

The project originally considered Arduino Uno/Nano. The intended firmware includes safety logic, LCD UI, encoder input, serial logs, secondary telemetry, EEPROM state, testing-oriented structure, and future extensibility.

Uno/Nano would force unnecessary memory and I/O constraints. The user does not expect to use those boards for this project.

## Decision

Use Arduino Mega2560 as the initial and only target platform.

Arduino Uno/Nano compatibility is out of scope.

## Consequences

- More flash, SRAM, serial ports, and I/O pins are available.
- Architecture can remain clear and testable without squeezing into ATmega328 constraints.
- Secondary serial telemetry is practical.
- Board is larger and slightly more expensive.
- Firmware should still respect embedded constraints.
