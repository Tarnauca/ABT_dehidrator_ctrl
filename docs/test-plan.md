# Test Plan

Status: draft baseline.

## Strategy

The firmware should be designed so core behavior can be tested without Mega2560 hardware connected. Arduino-specific code should be thin adapters around project-owned interfaces.

## Unit Tests

Target pure C++ modules where practical:

- Control state machine.
- Fixed and fluctuating profile engine.
- Hysteresis temperature control.
- Relay minimum on/off timing.
- Fault detector.
- Pause/resume timing.
- EEPROM validation helpers.
- Serial log formatting.
- Preset validation.

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

## Hardware Safety Tests

Before real unattended use:

- Confirm heater OFF above 75 C.
- Confirm hard fault at 80 C using simulated or controlled sensor input where possible.
- Confirm outputs OFF on reset.
- Confirm fault acknowledgement is required.
- Confirm no heater ON without fan ON.
- Confirm finish cooldown is time-based 3 minutes.

## Acceptance Tests

Acceptance tests should reference requirement IDs from `docs/requirements.md` once implementation exists.
