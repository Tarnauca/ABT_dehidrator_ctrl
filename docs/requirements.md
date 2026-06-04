# Requirements Draft

Status: draft baseline from discovery conversation. Requirement IDs are stable and must not be reused. If a requirement is removed, mark it retired instead of reusing its ID.

## Requirement Change Rules

- Requirement IDs are stable.
- Retired IDs shall not be reused.
- Material behavior changes should be noted in the branch or review that introduced them.
- Tests should reference requirement IDs where practical.

## Functional Requirements

- REQ-FUNC-001: The controller shall support manual on/off mode.
- REQ-FUNC-002: The controller shall support fixed temperature and duration mode.
- REQ-FUNC-003: The controller shall support fluctuating temperature mode with low/high temperature range, average temperature metadata, cycle timing, and duration.
- REQ-FUNC-004: The controller shall support built-in presets only in the initial scope.
- REQ-FUNC-005: The controller shall support pause and resume for active programs.
- REQ-FUNC-006: The controller shall support normal user stop/cancel with confirmation.
- REQ-FUNC-007: The controller shall clear resume state automatically after normal finish.
- REQ-FUNC-008: The controller shall support duration values up to 99 h 0 min. Internal and editable representations may use `HH:MM`, while compact LCD views may use `Xh Ym` when space is limited.
- REQ-FUNC-009: Temperature settings shall use Celsius with 1 C step.
- REQ-FUNC-010: In manual mode, the user may control heater operation only within safety constraints; if heater is requested ON, fan shall also be commanded ON.
- REQ-FUNC-011: Pause shall command heater OFF and fan OFF immediately, suspend program timer/profile progression, and keep the active run resumable.
- REQ-FUNC-012: Resume from pause shall continue the same profile from the paused point.
- REQ-FUNC-013: Confirmed user stop/cancel shall command heater OFF and fan OFF immediately, shall not run cooldown, and shall clear or mark resume state non-resumable.
- REQ-FUNC-014: Normal program finish shall command heater OFF, run fan for a fixed 3-minute cooldown, command fan OFF, then issue the finish alarm.
- REQ-FUNC-015: The initial fluctuating mode algorithm shall alternate time-based low-temperature and high-temperature phases around the configured profile range. Default phase timing is TBD and shall be configurable in code.
- REQ-FUNC-016: Selecting a built-in preset shall start the associated drying profile when the controller is idle and the preset profile is valid.

## Safety Requirements

- REQ-SAFE-001: The maximum user/program setpoint shall be 75 C.
- REQ-SAFE-002: The heater shall be forced OFF when the primary thermistor temperature is greater than 75 C.
- REQ-SAFE-003: A hard over-temperature fault shall occur when the primary thermistor temperature is at or above 80 C.
- REQ-SAFE-004: The heater shall never be commanded ON unless the fan is commanded ON.
- REQ-SAFE-005: Primary thermistor missing, invalid, or out-of-range condition shall trigger a hard fault.
- REQ-SAFE-006: If temperature does not rise by at least 2 C within 5 min of accumulated heater ON command time, the controller shall trigger a hard fault.
- REQ-SAFE-007: If primary thermistor temperature rises by 3 C over 5 min while heater command is OFF, measured from the end of a 2 min post-heater-off grace period, the controller shall trigger a suspected-heater-stuck-ON hard fault.
- REQ-SAFE-008: A pushbutton active continuously for 30 s shall trigger a hard fault.
- REQ-SAFE-009: During a hard fault, heater and fan shall be commanded OFF immediately.
- REQ-SAFE-010: A hard fault shall require user acknowledgement before another run can start.
- REQ-SAFE-011: Watchdog reset during an active run shall be treated as a hard fault/non-auto-resume condition.
- REQ-SAFE-012: Manual mode shall not bypass safety checks or limits.
- REQ-SAFE-013: Startup shall initialize outputs OFF before allowing any run.
- REQ-SAFE-014: Startup self-checks shall include primary thermistor plausibility, secondary temp/RH sensor availability, EEPROM config validation/defaulting, input stuck detection where feasible, interrupted-run detection, and commanded output-safe-state verification.
- REQ-SAFE-015: Warnings shall be shown/logged but shall not require acknowledgement and shall not block operation unless later promoted to hard faults.
- REQ-SAFE-016: The firmware shall define stable fault and warning codes for UI, logs, tests, and documentation.

Product-minded recommendation: add independent hardware thermal protection in the heater power path.

## Hardware Requirements

- REQ-HW-001: The initial target board shall be Arduino Mega2560.
- REQ-HW-002: The primary control temperature sensor shall be an NTC thermistor with configurable nominal resistance and Beta coefficient.
- REQ-HW-003: A DHT22/AM2302-class sensor shall provide secondary temperature/RH telemetry when available.
- REQ-HW-004: Heater and fan shall be controlled through relay outputs.
- REQ-HW-005: Relay polarity shall be configurable.
- REQ-HW-006: The LCD shall be 4 lines x 20 characters, driven through an I2C-to-parallel interface.
- REQ-HW-007: LCD backlight shall be controlled by a dedicated MCU pin through a FET.
- REQ-HW-008: User input shall use a rotary encoder with pushbutton.
- REQ-HW-009: A piezo buzzer shall be used for finish and fault alarms.
- REQ-HW-010: A secondary hardware serial output shall mirror structured logs by default.
- REQ-HW-011: The secondary serial interface shall be output-only in the current scope and shall not accept commands.

## UI Requirements

- REQ-UI-001: LCD user-facing text shall be Romanian.
- REQ-UI-002: LCD text shall be ASCII-only Romanian by default unless the exact LCD character set is verified.
- REQ-UI-003: Serial logs, source code, comments, and developer docs shall be English.
- REQ-UI-004: LCD shall show RH when the secondary temp/RH sensor is present and functional.
- REQ-UI-005: A bottom-right heartbeat symbol shall always indicate that the main loop is alive.
- REQ-UI-006: Finish alarm shall use buzzer and blinking LCD backlight after the 3-minute finish cooldown completes.
- REQ-UI-007: Fault alarm shall use buzzer, blinking LCD backlight, and a compact Romanian fault message until user acknowledgement.
- REQ-UI-008: Encoder rotation shall navigate/change values.
- REQ-UI-009: Encoder short press shall select/confirm.
- REQ-UI-010: Encoder long press has no required navigation role in the current scope and may remain unused or reserved for future behavior.
- REQ-UI-011: Every menu-like LCD screen shall provide `Inapoi` as the last selectable entry.
- REQ-UI-012: Selecting `Inapoi` shall navigate exactly one level up in the UI hierarchy.
- REQ-UI-013: Menu-like LCD screens shall use the first line as the current section title and the remaining visible lines for selectable entries.
- REQ-UI-014: Temperature displayed on the LCD shall use the `°C` suffix consistently where temperature is shown.
- REQ-UI-015: Compact LCD duration display shall use the `Xh Ym` format where the UI shows preset summary values.
- REQ-UI-016: During an active preset run, the status screen shall show the current Romanian lifecycle label, including at least idle, running, finish cooldown, and finished states.
- REQ-UI-017: When the finish alarm state is active on the status screen, a short press shall acknowledge completion and return the controller to idle.

## Logging And Diagnostics Requirements

- REQ-LOG-001: USB serial logs shall be emitted at 115200 8N1, no flow control.
- REQ-LOG-002: Secondary serial telemetry shall use the same settings by default.
- REQ-LOG-003: Logs shall be human-readable English and easy for tools to parse.
- REQ-LOG-004: Every event, parameter change, output change, warning, fault, and lifecycle transition shall be logged.
- REQ-LOG-005: Temperature/RH/status shall be logged periodically every 5 s during active operation.
- REQ-LOG-006: Raw ADC values shall be emitted only in verbose/debug mode.
- REQ-LOG-007: Reset cause shall be logged at boot when feasible.
- REQ-LOG-008: Stable English fault and warning codes shall be used in serial logs, while LCD messages shall use compact Romanian text.

## Persistence Requirements

- REQ-PERSIST-001: EEPROM shall store configuration/calibration data.
- REQ-PERSIST-002: EEPROM shall store minimal interrupted-run state.
- REQ-PERSIST-003: EEPROM writes shall be minimized and never performed in fast loops.
- REQ-PERSIST-004: Resume snapshots shall not be written more often than every 15 min during a run except major lifecycle events.
- REQ-PERSIST-005: EEPROM config/state shall include versioning and validation/checksum.
- REQ-PERSIST-006: Invalid config shall load defaults and emit a warning.
- REQ-PERSIST-007: Faulted run context may be retained for diagnostics but shall not allow resume.
- REQ-PERSIST-008: Reset cause/event shall be recorded in EEPROM carefully when feasible.
- REQ-PERSIST-009: After power loss or brown-out during an active run, the controller shall offer resume only after user confirmation.
- REQ-PERSIST-010: Watchdog-reset run context shall not allow automatic or user-confirmed resume without starting a new run.

## Testing And Verification Requirements

- REQ-TEST-001: Control/profile/fault logic shall be testable without connected hardware.
- REQ-TEST-002: Hardware access shall be wrapped behind project-owned interfaces.
- REQ-TEST-003: Unit tests shall cover state transitions, hysteresis, profile cycling, pause/resume, fault detection, and logging formatting where practical.
- REQ-TEST-004: Manual bench tests shall be documented before real heater testing.

## Deferred/Future Requirements

- REQ-FUTURE-001: Humidity-based stop is deferred.
- REQ-FUTURE-002: Serial command interface is deferred.
- REQ-FUTURE-003: UI calibration screen is deferred.
- REQ-FUTURE-004: Food preset values are TBD and may be sourced from a product manual PDF.
