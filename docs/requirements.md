# Requirements Draft

Status: draft baseline from discovery conversation. Requirement IDs are stable and must not be reused. If a requirement is removed, mark it retired instead of reusing its ID.

## Requirement Change Rules

- Requirement IDs are stable.
- Retired IDs shall not be reused.
- Material behavior changes should be noted in the branch or review that introduced them.
- Tests should reference requirement IDs where practical.

## Functional Requirements

- REQ-FUNC-001: The controller shall support a direct-output test mode for relay and output bring-up.
- REQ-FUNC-002: The controller shall support fixed temperature and duration mode.
- REQ-FUNC-003: The controller shall support fluctuating temperature mode with reference temperature metadata, absolute upper/lower target temperatures, cycle timing, and duration.
- REQ-FUNC-004: The controller shall support built-in presets only in the initial scope.
- REQ-FUNC-005: The controller shall support pause and resume for active programs.
- REQ-FUNC-006: The controller shall support normal user stop/cancel with confirmation.
- REQ-FUNC-007: The controller shall clear resume state automatically after normal finish.
- REQ-FUNC-008: The controller shall support duration values up to 99 h 0 min. Internal and editable representations may use `HH:MM`, while compact LCD views may use `Xh Ym` when space is limited.
- REQ-FUNC-009: Temperature settings shall use Celsius with 1 C step.
- REQ-FUNC-010: In test mode, the user may control heater operation only within safety constraints; if heater is requested ON, fan shall also be commanded ON.
- REQ-FUNC-017: The controller shall support a manual configurable program with selectable `Constant`, `Boost`, and `Fluctuant` modes.
- REQ-FUNC-018: Manual `Constant` mode shall provide editable temperature and duration.
- REQ-FUNC-019: Manual `Boost` mode shall provide editable base temperature, total duration, boost temperature delta from 0 C to 20 C in 5 C steps, and boost duration in 5 min steps. The boost phase shall be first, shall not exceed 50% of the total duration, and shall be followed by constant operation at the base temperature.
- REQ-FUNC-020: Manual `Fluctuant` mode shall provide editable reference temperature, total duration, absolute upper/lower temperatures within +/-10 C of the reference temperature in 1 C steps, and upper/lower phase durations from 5 min to 20 min in 1 min steps.
- REQ-FUNC-021: Manual-program editing shall provide `Salveaza` before `Inapoi` and shall support storing the current manual profile into one of 10 user-profile slots.
- REQ-FUNC-022: If `Start` is selected while the current manual profile has unsaved changes, the controller shall ask `Da / Nu / Renunta` for saving first. `Da` shall continue to slot selection and then start automatically after save; `Nu` shall start without saving; `Renunta` shall return to the editor.
- REQ-FUNC-023: If `Inapoi` is selected while the current manual profile has unsaved changes, the controller shall ask `Da / Nu / Renunta` for saving first. `Da` shall continue to slot selection and then return to the previous menu after save; `Nu` shall discard unsaved changes and return; `Renunta` shall return to the editor.
- REQ-FUNC-024: The main menu shall provide a `Programe utilizator` entry for browsing 10 saved manual-profile slots, including vacant slots.
- REQ-FUNC-025: Saved user profiles shall support read-only inspection plus `Pornire`, `Editeaza`, `Sterge`, and `Inapoi` actions when occupied. Vacant slots shall indicate `Liber` and shall still allow entering the editor through `Editeaza`.
- REQ-FUNC-026: The main menu shall order entries as `Oprire program`, `Pauza program`, `Reluare program`, `Programe presetate`, `Programe utilizator`, `Program manual`, `Setari`, `Inapoi`, while hiding the run-control entries when they are not applicable.
- REQ-FUNC-027: `Setari` shall expose `Testare` and `Inapoi` as a dedicated submenu instead of showing `Testare` in the main menu.
- REQ-FUNC-028: `Testare` shall show the latest NTC temperature, AM2302 temperature, and AM2302 humidity as the first read-only entries before any output toggles.
- REQ-FUNC-029: If a `Testare` sensor reading is invalid, the corresponding row shall show a short error indication instead of a value.
- REQ-FUNC-011: Pause shall command heater OFF and fan OFF immediately, suspend program timer/profile progression, and keep the active run resumable.
- REQ-FUNC-012: Resume from pause shall continue the same profile from the paused point.
- REQ-FUNC-013: Confirmed user stop/cancel shall command heater OFF and fan OFF immediately, shall not run cooldown, and shall clear or mark resume state non-resumable.
- REQ-FUNC-014: Normal program finish shall command heater OFF, run fan for a fixed 3-minute cooldown, command fan OFF, then issue the finish alarm.
- REQ-FUNC-015: The fluctuating mode algorithm shall start with the upper-temperature phase, then alternate time-based upper/lower phases until the total duration expires.
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
- REQ-UI-005: The main status screen shall show a blinking top-right play symbol while a run is `RULARE`, a blinking top-right pause symbol while a run is `PAUZA`, and no status-corner symbol in other lifecycle states.
- REQ-UI-006: Finish alarm shall use buzzer and blinking LCD backlight after the 3-minute finish cooldown completes.
- REQ-UI-007: Fault alarm shall use buzzer, blinking LCD backlight, and a compact Romanian fault message until user acknowledgement.
- REQ-UI-008: Encoder rotation shall navigate/change values.
- REQ-UI-009: Encoder short press shall select/confirm.
- REQ-UI-010: Encoder long press has no required navigation role in the current scope and may remain unused or reserved for future behavior.
- REQ-UI-011: Every menu-like LCD screen shall provide a dedicated one-level-up entry (e.g. `Back`) as the last selectable entry.
- REQ-UI-012: Selecting the dedicated one-level-up entry (e.g. `Back`) shall navigate exactly one level up in the UI hierarchy.
- REQ-UI-013: Menu-like LCD screens shall use the first line as the current section title and the remaining visible lines for selectable entries.
- REQ-UI-014: Temperature displayed on the LCD shall use the `°C` suffix consistently where temperature is shown.
- REQ-UI-015: Compact LCD duration display shall use the `Xh Ym` format where the UI shows preset summary values, status elapsed/remaining time, or program parameters.
- REQ-UI-016: The main status screen summary page shall show `Program: <program>`, `Temp:` with primary temperature, `RH:` when available, `Timp scurs:`, and `Timp ramas:` on the four LCD lines.
- REQ-UI-017: When the finish alarm state is active on the status screen, a short press shall acknowledge completion and return the controller to idle.
- REQ-UI-018: User-profile slot lists shall show all 10 slots plus `Inapoi`; occupied slots shall be labeled as `Profil N`, while vacant slots shall be labeled as `Profil N (nedef.)`.
- REQ-UI-019: Saving over an occupied user-profile slot shall require explicit `Da / Nu` overwrite confirmation.
- REQ-UI-020: Deleting one occupied user-profile slot shall require explicit `Da / Nu` confirmation.
- REQ-UI-021: After saving one manual profile, the `Salveaza profil` slot list shall close automatically and return to the screen flow that triggered the save action.
- REQ-UI-022: If a menu-like screen is exited through its dedicated one-level-up entry (e.g. `Back`), the next entry into that screen shall start at the first actionable item, not at the one-level-up entry.
- REQ-UI-023: While the main status screen is active, encoder rotation shall cycle between logical status pages without opening the menu.
- REQ-UI-024: Additional status pages shall show active-program parameters and logical output states, using one parameter or output per LCD line where practical.
- REQ-UI-025: The status screen may use compact program labels when the full source label would not fit after the `Program:` prefix on one 20-character line.
- REQ-UI-026: Selecting `Oprire program` from the main menu shall require explicit `Da / Nu` confirmation before the run is stopped.
- REQ-UI-027: `Pauza program` shall appear only while a run is in `RULARE`, and its resume flow shall reuse the same `Reluare program` behavior as other resumable interruptions.

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
- REQ-PERSIST-011: EEPROM shall store 10 user-defined manual profiles separately from interrupted-run resume storage.
- REQ-PERSIST-012: Each stored user-profile slot shall include versioning and validation/checksum so invalid/corrupt records are treated as vacant rather than executable.
- REQ-PERSIST-013: Saving one user-defined profile shall update only the selected EEPROM slot bytes rather than rewriting unrelated slots.
- REQ-PERSIST-014: User-profile storage and interrupted-run resume storage shall remain separate concerns in code and schema.

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
