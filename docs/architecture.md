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
It starts in `Boot`, runs a startup `SelfCheck`, and then transitions to the
normal idle/run lifecycle.

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
- `ControlStateMachine`: boot/self-check/resume-offer and the normal run lifecycle.
- `ProfileEngine`: fixed, boost, and fluctuating temperature targets over time.
- `PresetRunController`: bridges preset/manual profiles into the run lifecycle,
  keeps the active profile independent from the optional active preset pointer,
  and coordinates profile evaluation with hysteresis control.
- `UserProfileStore`: owns the EEPROM schema for 10 user-defined manual
  profiles, including slot occupancy, version, and checksum handling.
- `TemperatureControl`: hysteresis and relay minimum timing decisions.
- `FaultDetector`: primary thermistor validity, over-temperature, no-rise, stuck-heater, stuck-input checks.
- `RunTimer`: elapsed/remaining time and pause behavior.
- `LogFormatter`: stable structured serial lines.
- `LogDispatcher`: fan-out to USB debug and secondary telemetry sinks without dynamic allocation.

Hardware-facing adapters:

- `NtcAnalogSensor`.
- `TempRhSensorAdapter`.
- `RelayOutputs`.
- `LcdDisplay`.
- `EncoderInput`.
- `BuzzerAlarm`.
- `BacklightOutput`.
- `EepromStore`.
- `UserProfileStore`.
- `SerialLogSink`.

## Data Flow

The application loop should follow a stable flow:

1. Read clock/time.
2. Sample sensors on configured intervals.
3. Update debounced input state.
4. Run fault detection.
5. Update state machine.
6. Compute profile target from the active profile.
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

- Primary thermistor validity and filtered temperature.
- Secondary temp/RH sensor availability and optional temperature/RH.
- Startup configuration validity and commanded output-safe-state checks.
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
- UI blink timing and status indication behavior.
- Logging intervals.
- Calibration defaults.
- Built-in presets.

## Profile Model

Profiles are evaluated by pure logic before temperature control runs:

- Fixed profiles return the base target for the full active duration.
- Boost profiles return the boost/high target during the initial boost phase,
  then return the base target for the rest of the active duration.
- Fluctuating profiles return the upper target first, then alternate upper and
  lower targets according to configured phase durations.

`PresetRunController` must store the active `ProfileConfig` separately from
`activePreset`. Preset runs set both the active profile and the preset pointer;
manual runs set only the active profile and use a stable run token such as
`manual`. This prevents manual profiles from depending on built-in preset
storage.

Manual-program editing constraints are owned by `ManualProgramController`.
`ProfileEngine` validates generic profile safety and shape constraints, such as
maximum target temperature, valid total duration, boost duration not exceeding
half of total duration, and fluctuating low/high ordering. It intentionally does
not enforce UI-specific edit ranges such as manual `Tsup`/`Tinf` staying within
10 C of the reference temperature; callers that create manual profiles must use
the manual controller or an equivalent manual-profile validator.

`ManualProgramController` also owns the editor dirty/baseline state so `Nu`
from an unsaved `Inapoi` prompt can discard changes deterministically without
mixing UI persistence rules into `main.cpp`.

User-defined profile persistence is intentionally separate from interrupted-run
resume persistence. The current user-profile path stores only reusable manual
profiles, while resume storage remains a later concern for run snapshots and
reset recovery. This separation keeps the EEPROM schema easier to reason about
and avoids mixing "library of profiles" behavior with "recover active run"
behavior.

Proposed configuration ownership:

- `HardwareConfig`: pins, relay polarity, LCD I2C address, serial port selection, baud rate.
- `SafetyConfig`: 75 C setpoint limit, 80 C hard fault, no-rise and stuck-heater thresholds, input-stuck timing.
- `ControlConfig`: hysteresis, relay minimum ON/OFF times, sensor/control intervals.
- `UiConfig`: LCD dimensions, activity-indicator blink interval, alarm blink timing.
- `LoggingConfig`: periodic state interval, verbose/raw ADC enable flag.
- `PersistenceConfig`: EEPROM schema version, checkpoint interval, validation settings.
- `CalibrationConfig`: default primary thermistor and secondary temp/RH calibration constants.
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
- `SensorReader`: provides latest primary thermistor/secondary temp-RH readings and validity flags.
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
