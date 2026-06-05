# Project Context

This file is the durable project memory. New Codex sessions should read this file first, then `docs/backlog.md`, then check `git status` and continue from the active task.

## Product Goal

Build a safe, maintainable food dehydrator controller for hobby use, with product-minded recommendations documented separately where useful.

The device must work safely. Software safety is important, but product-minded recommendations should note where independent hardware protection is recommended.

## Target Platform

- MCU: Arduino Mega2560.
- Toolchain: PlatformIO with Arduino framework.
- Uno/Nano compatibility is explicitly out of scope.
- Existing environment: VS Code devcontainer running under WSL, PlatformIO installed and working, serial device passed into the container, SSH agent available for Git operations.

## Hardware Baseline

- Primary temperature sensor: NTC thermistor through voltage divider into analog input.
- Thermistor supply/reference: MCU VCC, no dedicated voltage reference currently.
- Secondary sensor: DHT22/AM2302-class temperature/RH sensor.
- Heater control: relay, on/off only.
- Fan control: relay, on/off only.
- Relay polarity: configurable; exact hardware behavior TBD.
- Display: 4x20 character LCD through I2C-to-parallel interface.
- Input: rotary encoder with pushbutton.
- Alarm: piezo buzzer.
- Backlight: LCD backlight controlled by a dedicated MCU pin driving a FET.
- Storage: MCU EEPROM only.
- Serial debug: USB serial.
- Secondary telemetry: additional hardware serial, output-only, mirrors structured logs by default. Possible physical transport: front-panel IrDA.
- Serial default: 115200 8N1, no flow control.

Product-minded recommendation: use an independent thermal fuse, thermal cutoff, or thermostat in the heater power path. Software must not be the only over-temperature protection.

## Operating Modes

- Test mode for direct fan/heater output bring-up.
- Preset mode using built-in profiles.
- Manual program mode with three variants:
  - `Constant`: temperature and duration.
  - `Boost`: base temperature, total duration, initial boost delta, and boost duration.
  - `Fluctuant`: reference temperature, total duration, absolute upper/lower targets, and upper/lower phase durations.
- Fixed temperature and duration profiles.
- Boost profiles that start with a higher target and then continue at the base target.
- Fluctuating temperature profiles with reference temperature metadata, absolute upper/lower targets, cycle timing, and duration.
- Manual profiles can be saved into 10 EEPROM-backed user slots, overwritten after confirmation, and later browsed from `Programe utilizator`.
- Humidity-based stop is deferred/future scope.
- Presets are built-in only, such as apple, herbs, jerky, yogurt. Exact values may be sourced later from a product manual PDF.

## Control And Safety Baseline

- User setpoints are limited to 75 C.
- If primary thermistor temperature exceeds 75 C, heater must be commanded OFF.
- Hard over-temperature fault threshold is 80 C measured by the primary thermistor.
- No heater operation is allowed unless fan is commanded ON.
- Heater control uses simple hysteresis and relay minimum on/off timing.
- Profile logic supplies the current target temperature; hysteresis control then
  decides heater ON/OFF around that target.
- Boost profiles start with the boost target and then continue as constant
  profiles after the boost phase duration expires.
- Fluctuating profiles start with the upper target and alternate upper/lower
  targets until the total duration expires.
- Heater relay minimum ON/OFF time: 10 s.
- Fan relay minimum ON/OFF time: 10 s, except hard fault or explicit stop may force OFF immediately.
- Primary thermistor/control update interval: 1 s.
- Secondary temp/RH sensor update interval: 2 s.
- Periodic serial state log interval: 5 s.
- Finish cooldown: heater OFF, fan ON for 3 minutes, then fan OFF and finish alarm.
- Pause: heater OFF and fan OFF immediately; timer/profile progression suspended.
- Resume: continues the same profile from where it paused.
- User stop/cancel: confirmation required, then heater OFF and fan OFF immediately; no cooldown.
- Hard fault: heater OFF and fan OFF immediately, LCD fault message, buzzer alarm, blinking backlight, acknowledgement required.

## Faults And Warnings

Hard faults:

- Primary thermistor missing, invalid, or out of range.
- Primary thermistor temperature at or above 80 C.
- Temperature does not rise by at least 2 C within 5 min of accumulated heater ON command time.
- Suspected heater stuck ON: after a 2 min post-heater-off grace period, primary thermistor rises by 3 C over 5 min while heater command is OFF.
- Pushbutton stuck active for 30 s.
- Watchdog reset during an active run.

Warnings:

- Secondary temp/RH sensor unavailable.
- Primary thermistor and secondary temp/RH temperature mismatch.
- EEPROM/config invalid; defaults are loaded.
- Encoder signal abnormal/chattering, unless later proven to be a hard blocking fault.

Warnings are shown/logged but do not require acknowledgement.

## UI And Logging

- LCD/user-facing UI language: Romanian only.
- LCD text should be ASCII-only Romanian by default because HD44780 diacritics are uncertain.
- LCD size: 4 lines x 20 characters.
- Main menu is product-oriented and dynamic: `Oprire program` and
  `Reluare program` appear only when applicable, while `Testare` lives under
  `Setari`.
- The main LCD status area is paged: rotary movement on the status screen
  cycles summary, parameter, and output pages without opening the menu.
- The status summary page shows `Program`, `Temp/RH`, `Timp scurs`, and
  `Timp ramas`; longer program names may be compacted to fit the 20-character
  line.
- UI navigation rule: the dedicated one-level-up entry (e.g. `Back`) of any
  menu-like screen must not remain selected across re-entry. When the screen is
  opened again, selection starts on the first actionable item.
- Bottom-right LCD cell is reserved for an always-running heartbeat symbol.
- Heartbeat should use a custom character if supported.
- Serial logs, code, comments, and docs are English.
- Serial logs should be human-readable but tool-friendly.
- Every discrete event/change should be logged: state changes, parameter changes, output changes, warnings, faults, user actions, resume/discard decisions.
- Temperature/RH/status should be reported periodically, not on every sensor read.
- Raw ADC values should appear only in verbose/debug output.
- Initial serial scope is logs only; no command parser in the baseline.
- Hard-fault buzzer/backlight alarm continues until user acknowledgement.

## Persistence

- EEPROM stores configuration/calibration and minimal interrupted-run state.
- EEPROM also stores 10 reusable manual user profiles as a separate concern from interrupted-run resume state.
- EEPROM writes must be minimized.
- Do not write in fast loops.
- Use versioning and validation/checksum.
- Invalid config loads defaults and logs/shows warning.
- Resume snapshots should be written at meaningful checkpoints and no more often than every 15 min during a run, except start, pause, resume, finish, or fault.
- Normal finish clears resume state automatically.
- Explicit cancel/discard should clear or mark resume state non-resumable.
- Hard fault retains diagnostic context but must not allow resume.
- Reset cause should be detected if feasible, logged at boot, and recorded in EEPROM carefully.

## Firmware Architecture Rules

- Use clear, modest C++.
- Separate pure control logic from Arduino hardware adapters.
- Hardware libraries must be wrapped behind project-owned interfaces.
- Use a simple cooperative scheduler/event loop, not an RTOS.
- Avoid blocking delays longer than 100 ms during normal runtime logic.
- No dynamic allocation in application code.
- Avoid Arduino `String`; use fixed buffers, string literals, `snprintf`, and flash-stored constants where appropriate.
- Centralize hardware pin assignments and polarity.
- Centralize configuration by category: hardware, safety, control timing, UI timing, logging, calibration, presets.
- Add comments for complex safety/control logic, but avoid noisy comments.

## Testing Direction

- Important logic must be testable without the Mega2560 connected.
- Use pure C++ modules for state machine, profiles, hysteresis, fault detection, timing, and formatting where practical.
- Use fakes/mocks for hardware-facing interfaces.
- PlatformIO should eventually support a Mega2560 firmware target and a native unit-test target if practical.

## Development Workflow

- Use branch/commit based workflow.
- First branch/commit/push may be performed by Codex to establish the baseline.
- After that, the user wants to perform Git operations manually for a while under Codex supervision.
- Prefer focused branches and focused commits.
- Before committing: check status, review diff, stage explicit paths, commit with a semantic message in the form `type(scope): short imperative summary`.
- Before Codex commits or merges, Codex must ask whether the user has changes to make first.
- Merge commits should use the project convention `merge(scope): short merge summary`.
- Documentation impact check is required before branch merge and when a commit clearly affects requirements, architecture, safety, tests, UI, persistence, hardware assumptions, or external interfaces.
- Branches that complete, start, split, remove, or materially change planned work must update `docs/backlog.md` in the same branch, or explicitly state that no backlog update was needed.
- Branches with meaningful agent reviews, workflow changes, milestone commits/merges, or user decisions that change direction must update `docs/logbook.md`, or explicitly state that no logbook update was needed.
- Interactions that produce reusable lessons must update `docs/learning/`, or explicitly state that no learning-note update was needed.

## Agentic Development

Reusable agent definitions live under `docs/agents/`.

Default mode is hybrid:

- Codex may recommend or trigger agent review at natural checkpoints.
- The user can manually request agents by name.
- Review agents are read-only by default.
- Worker agents may edit only when assigned a bounded ownership area.
- Dev environment changes require audit, summary, and explicit user confirmation before editing.
