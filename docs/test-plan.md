# Test Plan

Status: draft baseline.

## Strategy

The firmware should be designed so core behavior can be tested without Mega2560 hardware connected. Arduino-specific code should be thin adapters around project-owned interfaces.

Once implementation begins, native tests for affected pure logic modules should pass before firmware upload. Hardware-only changes still require Mega2560 build and appropriate bench checks.

The first code branch should create the testable skeleton and one sample native unit test before adding substantial behavior.

## Unit Tests

Target pure C++ modules where practical:

- `test_control_state_machine`: `Boot`, `SelfCheck`, `Idle`, `ResumeOffer`, `Running`, `Paused`, `Stopping`, `FinishCooldown`, `FinishedAlarm`, and `Fault` transitions.
- `test_temperature_control`: hysteresis, relay minimum ON/OFF timing, heater forced OFF above 75 C, hard fault behavior handoff.
- `test_fault_detector`: PT50 invalid, over-temperature, no-temperature-rise, heater-stuck-ON, stuck button, watchdog reset.
- `test_profile_engine`: fixed target and fluctuating low/high phase target generation.
- `test_run_timer`: `HH:MM` duration limits, pause/resume, finish timing, 3-minute finish cooldown timing.
- `test_log_formatter`: stable English event/state/warning/fault records that are human-readable and parseable.
- `test_persistence_validation`: EEPROM version/checksum validation, defaulting, resume snapshots, faulted non-resumable state.
- `test_preset_catalog`: built-in preset validation once preset values are defined.

## Test Fakes

Native tests should use simple fakes:

- `FakeClock`: deterministic `millis()`-style time control.
- `FakeSensorReader`: PT50/AHT values, validity flags, RH availability.
- `FakeOutputController`: captures logical heater/fan/backlight/buzzer commands.
- `FakePersistentStore`: captures config, resume state, reset cause, and diagnostic writes.
- `FakeLogSink`: captures formatted log lines for exact or parseable checks.

Fakes should not require Arduino headers and should avoid dynamic allocation where practical.

## Simulation Tests

Use fake sensors, fake outputs, and fake clock to simulate:

- Normal fixed-temperature run.
- Fluctuating high/low cycle.
- Pause/resume.
- Normal finish and 3-minute cooldown.
- User stop/cancel.
- PT50 invalid fault.
- Over-temperature fault at 80 C.
- No-temperature-rise fault.
- Heater-stuck-ON fault.
- Watchdog/interrupted-run boot handling.
- Power-loss/brown-out resume offer versus watchdog non-resume behavior.
- Secondary serial log mirroring through captured log sinks.

## Requirement Coverage Matrix

Status values:

- Planned: test case identified but not implemented.
- Implemented: automated or manual test exists.
- Passing: test was run successfully.
- Deferred: intentionally postponed.

| Requirement | Test Type | Test Case | Fake/Bench Method | Status |
| --- | --- | --- | --- | --- |
| REQ-FUNC-002 | Unit | Fixed mode returns one target for the profile duration | `test_profile_engine` with active elapsed time input | Passing |
| REQ-FUNC-003 | Unit | Fluctuating mode alternates low/high phases | `test_profile_engine` with active elapsed time input | Passing |
| REQ-FUNC-008 | Unit | Profile duration accepts `99:00` and rejects invalid duration | `test_profile_engine` duration validation tests | Passing |
| REQ-FUNC-015 | Unit | Initial fluctuating algorithm starts high and alternates high/low phases | `test_profile_engine` phase boundary tests | Passing |
| REQ-FUNC-011 | Unit | Pause turns heater/fan OFF and suspends time | `test_run_state_machine` with scheduler-provided time delta | Passing |
| REQ-FUNC-012 | Unit | Resume continues from paused profile point | `test_run_state_machine` elapsed-time tests | Passing |
| REQ-FUNC-013 | Unit | Confirmed stop turns outputs OFF and clears/disables resume | `test_run_state_machine` stop transition tests | Passing |
| REQ-FUNC-014 | Unit | Finish enters 3-minute cooldown before alarm | `test_run_state_machine` cooldown transition tests | Passing |
| REQ-SAFE-001 | Unit | Reject or clamp setpoints above 75 C | `test_profile_engine` validation test | Passing |
| REQ-SAFE-002 | Unit | Heater may remain governed at 75 C but is forced OFF at 76 C | `test_temperature_control` boundary tests | Passing |
| REQ-SAFE-003 | Unit | Hard fault at 80 C and above | `test_fault_detector` over-temperature boundary tests | Passing |
| REQ-SAFE-004 | Unit/Bench | Heater command cannot be ON while fan command is OFF | Temperature-control policy tests passing; command sanitizer exists; relay adapter bench check still pending | Implemented |
| REQ-SAFE-005 | Unit | PT50 invalid triggers hard fault | `test_fault_detector` validity and plausible-range tests | Passing |
| REQ-SAFE-006 | Unit | No 2 C rise within 5 min accumulated heater ON time triggers hard fault | `test_fault_detector` accumulated heater-ON timing tests | Passing |
| REQ-SAFE-007 | Unit | After 2 min grace, 3 C rise over 5 min heater OFF triggers stuck-ON fault | `test_fault_detector` heater-OFF grace and rise tests | Passing |
| REQ-SAFE-008 | Unit | Button active for 30 s triggers hard fault | `test_fault_detector` stuck-button timing tests | Passing |
| REQ-SAFE-009 | Unit/Bench | Hard fault forces heater/fan OFF immediately | `test_run_state_machine` output policy tests; relay bench check still pending | Implemented |
| REQ-SAFE-010 | Unit/UI | Hard fault requires acknowledgement before new run | `test_run_state_machine` acknowledgement tests; UI flow still pending | Implemented |
| REQ-SAFE-011 | Unit | Watchdog reset during run is non-resumable fault context | `test_fault_detector` watchdog-reset input covered; resume/persistence behavior pending | Implemented |
| REQ-SAFE-014 | Unit | Startup self-check validates sensors/config/input/output-safe-state | `test_control_state_machine`, fakes | Planned |
| REQ-HW-002 | Unit/Bench | PT50 is converted as the primary control temperature sensor | `test_pt50_sensor` conversion/reader tests; real divider bench check pending | Implemented |
| REQ-PERSIST-001 | Unit | Calibration defaults are explicit and later persistable | `test_config` PT50 calibration default tests; EEPROM persistence pending | Implemented |
| REQ-UI-005 | Bench | Heartbeat visible bottom-right and runs in all states | LCD bench test | Planned |
| REQ-UI-006 | Unit/Bench | Finish alarm starts after cooldown | State machine plus buzzer/backlight bench check | Planned |
| REQ-UI-007 | Unit/Bench | Fault alarm continues until acknowledgement | State machine plus buzzer/backlight bench check | Planned |
| REQ-LOG-004 | Unit | Events, parameters, outputs, warnings, faults are logged | `FakeLogSink` | Planned |
| REQ-LOG-005 | Unit | Periodic state log interval is 5 s | `FakeClock`, `FakeLogSink` | Planned |
| REQ-LOG-008 | Unit | English codes in logs, compact Romanian LCD messages | `test_log_formatter`, UI view tests | Planned |
| REQ-PERSIST-004 | Unit | Resume snapshots no more often than 15 min except lifecycle events | `FakeClock`, `FakePersistentStore` | Planned |
| REQ-PERSIST-005 | Unit | EEPROM version/checksum validation | `test_persistence_validation` | Planned |
| REQ-PERSIST-009 | Unit | Power loss/brown-out offers resume only after confirmation | `test_control_state_machine`, `FakePersistentStore` | Planned |
| REQ-PERSIST-010 | Unit | Watchdog reset does not allow resume | `test_control_state_machine`, `FakePersistentStore` | Planned |

## Manual Bench Tests

Before connecting real heater power:

- Verify relay polarity with safe low-voltage load.
- Verify fan relay behavior.
- Verify buzzer and backlight FET.
- Verify LCD 4x20 layout and heartbeat.
- Verify encoder rotation, short press, long press, and stuck-button detection.
- Verify PT50 analog readings and calibration visibility.
- Verify AHT temperature/RH reporting and warning on disconnect.
- Verify serial logs on USB and secondary serial.
- Verify secondary serial is output-only in current firmware scope.

## Hardware Safety Tests

Before real unattended use:

- Confirm heater is not forced OFF at exactly 75 C solely by the safety threshold, and is forced OFF above 75 C.
- Confirm hard fault at 80 C and above using simulated or controlled sensor input where possible.
- Confirm outputs OFF on reset.
- Confirm fault acknowledgement is required.
- Confirm no heater ON without fan ON.
- Confirm finish cooldown is time-based 3 minutes.
- Confirm no-temperature-rise and heater-stuck-ON rules using simulated sensor input before real heater power.

## Acceptance Tests

Acceptance tests should reference requirement IDs from `docs/requirements.md`.

Initial acceptance criteria:

- A fixed-temperature run can be started, paused, resumed, finished, and acknowledged while preserving the required output behavior.
- A fluctuating run alternates between configured low/high phase targets.
- Hard fault scenarios force heater/fan OFF immediately and prevent new run start until acknowledgement.
- Power loss/brown-out during active run offers resume only after user confirmation.
- Watchdog reset during active run does not allow resume.
- Logs are emitted on USB and secondary serial in the required structured English format.
- LCD UI remains Romanian, fits 4x20 constraints, and shows heartbeat bottom-right.
