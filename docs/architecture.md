# Architecture Draft

Status: draft baseline. This document describes the intended firmware shape before implementation.

## Architecture Goals

- Keep safety-critical logic readable and testable.
- Separate pure decision logic from Arduino hardware access.
- Keep hardware libraries replaceable through project-owned interfaces.
- Keep the code understandable for a learning project.
- Avoid over-engineering.

## Proposed Runtime Shape

The firmware should use a simple cooperative scheduler/event loop. Each task should return quickly and use `millis()`-based timing.

Example task groups:

- Sensor sampling.
- Control update.
- Fault detection.
- Output application.
- LCD update.
- Alarm update.
- Serial logging.
- EEPROM checkpoint handling.

No normal runtime task should block for more than 100 ms.

## Proposed Module Boundaries

Pure logic modules:

- `ControlStateMachine`: run states, pause/resume, finish, stop, fault transitions.
- `ProfileEngine`: fixed and fluctuating temperature targets over time.
- `TemperatureControl`: hysteresis and relay minimum timing decisions.
- `FaultDetector`: PT50 validity, over-temperature, no-rise, stuck-heater, stuck-input checks.
- `RunTimer`: elapsed/remaining time and pause behavior.
- `LogFormatter`: stable structured serial lines.

Hardware-facing adapters:

- `Pt50AnalogSensor`.
- `AhtSensorAdapter`.
- `RelayOutputs`.
- `LcdDisplay`.
- `EncoderInput`.
- `BuzzerAlarm`.
- `BacklightOutput`.
- `EepromStore`.
- `SerialLogSink`.

Configuration:

- Hardware pins, relay polarity, LCD address, and serial ports.
- Safety limits and fault thresholds.
- Control timing and hysteresis.
- UI and heartbeat timing.
- Logging intervals.
- Calibration defaults.
- Built-in presets.

## Interfaces

Application logic should depend on project-owned interfaces, not directly on third-party Arduino libraries.

Example conceptual interfaces:

```cpp
class SensorReader;
class OutputController;
class Clock;
class Display;
class LogSink;
class PersistentStore;
```

Exact names and shapes are TBD during implementation.

## Testing Approach

Pure logic modules should be testable with fake sensors, fake outputs, and fake clocks. Arduino-specific adapters may need board/manual tests.

PlatformIO should eventually provide:

- Mega2560 firmware build/upload target.
- Native test target for pure logic if practical.

## Architecture Rules

- No dynamic allocation in application code.
- Avoid Arduino `String`.
- Avoid long blocking delays.
- Never command heater ON unless fan is ON.
- Hard fault overrides normal relay timing and forces outputs OFF.
- Dev environment/tooling changes require audit and explicit user approval.
