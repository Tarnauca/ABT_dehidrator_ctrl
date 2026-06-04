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

## Runtime State Model

`ControlStateMachine` is the authoritative owner of the high-level run state.

Initial states:

- `Boot`: outputs are commanded OFF before initialization continues.
- `SelfCheck`: startup checks are performed before a run can start.
- `Idle`: no active run; outputs OFF.
- `ResumeOffer`: interrupted run state exists; user may resume or discard.
- `Running`: active drying program; fan commanded ON; heater controlled by profile/control/safety logic.
- `Paused`: active run is suspended; heater OFF; fan OFF; timer/profile progression suspended.
- `Stopping`: user-confirmed stop/cancel; heater OFF; fan OFF; resume state cleared or marked non-resumable.
- `FinishCooldown`: normal program finished; heater OFF; fan ON for fixed 3-minute cooldown.
- `FinishedAlarm`: cooldown complete; fan OFF; buzzer/backlight finish alarm active until acknowledged.
- `Fault`: hard fault active; heater OFF; fan OFF; fault alarm active until acknowledged.

Transitions must be explicit and testable. Hard faults may transition from any runtime state to `Fault`.

## Proposed Source Layout

The exact file names may evolve, but implementation should follow this separation:

```text
src/
  main.cpp                  Arduino setup/loop and wiring composition
  app/                      Application coordinator and scheduler
  domain/                   Pure logic: state, profiles, control, faults, timing
  hardware/                 Arduino/Mega2560 adapters
  config/                   Pin maps, constants, presets, calibration defaults
  logging/                  Formatters and log dispatch
include/
  dehydrator/               Public project headers shared by src and tests
test/
  test_domain_*/            Native/pure C++ tests where practical
```

Pure logic must not include Arduino-only headers unless a specific exception is documented.

## Proposed Module Boundaries

Pure logic modules:

- `ControlStateMachine`: run states, pause/resume, finish, stop, fault transitions.
- `ProfileEngine`: fixed and fluctuating temperature targets over time.
- `TemperatureControl`: hysteresis and relay minimum timing decisions.
- `FaultDetector`: PT50 validity, over-temperature, no-rise, stuck-heater, stuck-input checks.
- `RunTimer`: elapsed/remaining time and pause behavior.
- `LogFormatter`: stable structured serial lines.
- `LogDispatcher`: fan-out to USB debug and secondary telemetry sinks without dynamic allocation.

Hardware-facing adapters:

- `Pt50AnalogSensor`.
- `TempRhSensorAdapter`.
- `RelayOutputs`.
- `LcdDisplay`.
- `EncoderInput`.
- `BuzzerAlarm`.
- `BacklightOutput`.
- `EepromStore`.
- `SerialLogSink`.

## Data Flow

The application loop should follow a stable flow:

1. Read clock/time.
2. Sample sensors on configured intervals.
3. Update debounced input state.
4. Run fault detection.
5. Update state machine.
6. Compute profile target.
7. Compute heater/fan commands.
8. Apply safety invariants.
9. Write outputs.
10. Update UI/alarm/logging/persistence tasks.

Each step should use immutable input snapshots where practical so tests can reproduce decisions.

## Safety Invariant Enforcement

The heater/fan invariant is enforced at multiple layers:

- `ControlStateMachine` must not request heater operation in a state where fan is not required ON.
- `TemperatureControl` may request heater ON only when the current control context allows heat.
- A pure command-sanitizing step must force `heater=OFF` whenever `fan=OFF`.
- `RelayOutputs` must defensively reject or sanitize `heater=ON, fan=OFF` before writing pins.

Hard faults override relay minimum ON/OFF timing and force heater/fan OFF immediately.

## Fault Detector Inputs

`FaultDetector` should operate on explicit inputs:

- PT50 validity and filtered temperature.
- Secondary temp/RH sensor availability and optional temperature/RH.
- Current heater/fan commands.
- Previous heater/fan command history.
- Accumulated heater ON command time.
- Time since heater was last commanded OFF.
- Debounced button/encoder status.
- Reset cause/interrupted-run context.

No-temperature-rise detection uses accumulated heater ON command time. Heater-stuck-ON detection measures the 3 C rise starting after the 2-minute post-heater-off grace period.

Configuration:

- Hardware pins, relay polarity, LCD address, and serial ports.
- Safety limits and fault thresholds.
- Control timing and hysteresis.
- UI and heartbeat timing.
- Logging intervals.
- Calibration defaults.
- Built-in presets.

Proposed configuration ownership:

- `HardwareConfig`: pins, relay polarity, LCD I2C address, serial port selection, baud rate.
- `SafetyConfig`: 75 C setpoint limit, 80 C hard fault, no-rise and stuck-heater thresholds, input-stuck timing.
- `ControlConfig`: hysteresis, relay minimum ON/OFF times, sensor/control intervals.
- `UiConfig`: LCD dimensions, heartbeat interval, alarm blink timing.
- `LoggingConfig`: periodic state interval, verbose/raw ADC enable flag.
- `PersistenceConfig`: EEPROM schema version, checkpoint interval, validation settings.
- `CalibrationConfig`: default PT50 and secondary temp/RH calibration constants.
- `PresetCatalog`: built-in drying presets.

## Interfaces

Application logic should depend on project-owned interfaces, not directly on third-party Arduino libraries.

Example conceptual interfaces:

```cpp
class Clock;
class SensorReader;
class OutputController;
class LogSink;
class Display;
class PersistentStore;
```

Initial responsibilities:

- `Clock`: provides `millis()`-style monotonic time to pure logic and tests.
- `SensorReader`: provides latest PT50/secondary temp-RH readings and validity flags.
- `OutputController`: accepts logical heater/fan/backlight/buzzer commands.
- `LogSink`: writes already-formatted log lines to one destination.
- `Display`: writes LCD view updates without exposing LCD library details to domain logic.
- `PersistentStore`: loads/saves config, resume state, diagnostics, and reset cause data.

`LogDispatcher` may hold a fixed set of `LogSink` references or direct sink pointers. It must not allocate dynamically.

Secondary serial is currently used only as a mirrored structured-log sink for
early bring-up. If the port later becomes Modbus RTU or another gateway
protocol, that protocol must be implemented as a separate adapter rather than
as a `LogSink`.

## Testing Approach

Pure logic modules should be testable with fake sensors, fake outputs, and fake clocks. Arduino-specific adapters may need board/manual tests.

PlatformIO should eventually provide:

- Mega2560 firmware build/upload target.
- Native test target for pure logic if practical.

Native tests require domain modules to avoid Arduino-specific includes. `main.cpp` should remain a thin composition layer that wires real adapters to pure application logic.

## Architecture Rules

- No dynamic allocation in application code.
- Avoid Arduino `String`.
- Use fixed buffers for log/UI formatting; prefer `snprintf`.
- Avoid long blocking delays.
- Never command heater ON unless fan is ON.
- Hard fault overrides normal relay timing and forces outputs OFF.
- Secondary telemetry remains output-only.
- Dev environment/tooling changes require audit and explicit user approval.
