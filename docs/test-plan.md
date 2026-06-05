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
- `test_fault_detector`: primary thermistor invalid, over-temperature, no-temperature-rise, heater-stuck-ON, stuck button, watchdog reset.
- `test_profile_engine`: fixed target, boost phase, and fluctuating low/high phase target generation.
- `test_run_timer`: duration limits up to 99 h 0 min, pause/resume, finish timing, and 3-minute finish cooldown timing.
- `test_log_formatter`: stable English event/state/warning/fault records that are human-readable and parseable.
- `test_persistence_validation`: EEPROM version/checksum validation, defaulting, resume snapshots, faulted non-resumable state.
- `test_preset_catalog`: built-in preset validation once preset values are defined.
- `test_user_profile_store`: user-profile EEPROM slot save/load/clear/checksum handling.
- `test_save_prompt_controller`: `Da / Nu / Renunta` flow for unsaved manual edits.
- `test_user_profile_controllers`: slot browsing and saved-profile action menus.

## Test Fakes

Native tests should use simple fakes:

- `FakeClock`: deterministic `millis()`-style time control.
- `FakeSensorReader`: primary thermistor/secondary temp-RH values, validity flags, RH availability.
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
- Primary thermistor invalid fault.
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
| REQ-FUNC-015 | Unit | Fluctuating algorithm starts with upper target and alternates upper/lower phases | `test_profile_engine` phase boundary tests | Passing |
| REQ-FUNC-011 | Unit | Pause turns heater/fan OFF and suspends time | `test_run_state_machine` with scheduler-provided time delta | Passing |
| REQ-FUNC-012 | Unit | Resume continues from paused profile point | `test_run_state_machine` elapsed-time tests | Passing |
| REQ-FUNC-013 | Unit | Confirmed stop turns outputs OFF and clears/disables resume | `test_run_state_machine` stop transition tests | Passing |
| REQ-FUNC-014 | Unit | Finish enters 3-minute cooldown before alarm | `test_run_state_machine` cooldown transition tests | Passing |
| REQ-SAFE-001 | Unit | Reject or clamp setpoints above 75 C | `test_profile_engine` validation test | Passing |
| REQ-SAFE-002 | Unit | Heater may remain governed at 75 C but is forced OFF at 76 C | `test_temperature_control` boundary tests | Passing |
| REQ-SAFE-003 | Unit | Hard fault at 80 C and above | `test_fault_detector` over-temperature boundary tests | Passing |
| REQ-SAFE-004 | Unit/Bench | Heater command cannot be ON while fan command is OFF | Temperature-control policy tests passing; command sanitizer exists; relay adapter bench check still pending | Implemented |
| REQ-SAFE-005 | Unit | Primary thermistor invalid triggers hard fault | `test_fault_detector` validity and plausible-range tests | Passing |
| REQ-SAFE-006 | Unit | No 2 C rise within 5 min accumulated heater ON time triggers hard fault | `test_fault_detector` accumulated heater-ON timing tests | Passing |
| REQ-SAFE-007 | Unit | After 2 min grace, 3 C rise over 5 min heater OFF triggers stuck-ON fault | `test_fault_detector` heater-OFF grace and rise tests | Passing |
| REQ-SAFE-008 | Unit | Button active for 30 s triggers hard fault | `test_fault_detector` stuck-button timing tests | Passing |
| REQ-SAFE-009 | Unit/Bench | Hard fault forces heater/fan OFF immediately | `test_run_state_machine` output policy tests; relay bench check still pending | Implemented |
| REQ-SAFE-010 | Unit/UI | Hard fault requires acknowledgement before new run | `test_run_state_machine` acknowledgement tests; UI flow still pending | Implemented |
| REQ-SAFE-011 | Unit | Watchdog reset during run is non-resumable fault context | `test_fault_detector` watchdog-reset input covered; resume/persistence behavior pending | Implemented |
| REQ-SAFE-014 | Unit | Startup self-check validates sensors/config/input/output-safe-state | `test_control_state_machine`, fakes | Implemented |
| REQ-HW-002 | Unit/Bench | NTC thermistor is converted as the primary control temperature sensor | `test_ntc_sensor`; Arduino analog adapter builds; real divider/calibration bench check pending | Implemented |
| REQ-HW-003 | Unit/Bench | DHT22/AM2302-class sensor provides secondary temperature/RH telemetry when available | `test_temp_rh_reader`; concrete `DHT` adapter builds; bench check pending | Implemented |
| REQ-HW-004 | Unit/Bench | Heater and fan relay outputs translate logical commands to pins | `test_relay_outputs`; relay bench check pending | Implemented |
| REQ-HW-005 | Unit/Bench | Relay polarity is configurable | `test_relay_outputs` active-high/active-low tests; relay bench check pending | Implemented |
| REQ-HW-006 | Unit/Bench | LCD is 4x20 and driven through I2C backpack | `test_lcd_status_view`; Mega build with `LiquidCrystal_I2C`; LCD bench check pending | Implemented |
| REQ-HW-007 | Unit/Bench | LCD backlight uses dedicated output pin | `test_alarm_outputs`; backlight FET bench check pending | Implemented |
| REQ-HW-008 | Bench | Encoder and pushbutton provide user input events | Mega build with `Encoder` and `Bounce2`; serial event bench check pending | Implemented |
| REQ-HW-009 | Unit/Bench | Piezo buzzer is used for finish/fault alarms | `test_alarm_outputs`; buzzer bench check pending | Implemented |
| REQ-UI-004 | Unit/Bench | LCD shall show RH when the secondary temp/RH sensor is present and functional | `test_temp_rh_reader` plus firmware integration with concrete DHT adapter; real sensor bench check pending | Implemented |
| REQ-UI-008 | Unit/Bench | Encoder rotation navigates menu items | `test_menu_controller`, `test_lcd_menu_view`; bench encoder/menu check pending | Implemented |
| REQ-UI-009 | Unit/Bench | Encoder short press selects or confirms | `test_menu_controller`; bench button/menu check pending | Implemented |
| REQ-UI-010 | Unit/Bench | Long press has no assigned UI action in current scope | `test_test_mode_controller`, `test_preset_select_controller`; bench long-press no-op check pending | Implemented |
| REQ-UI-011 | Unit/Bench | Every menu-like screen exposes `Inapoi` as the last selectable entry | `test_menu_controller`, `test_preset_select_controller`, `test_test_mode_controller`, `test_manual_program_controller`, LCD view tests; bench menu check pending | Implemented |
| REQ-UI-012 | Unit/Bench | Selecting `Inapoi` returns one UI level up | Menu, preset, test, and manual-program controller tests; bench navigation check pending | Implemented |
| REQ-UI-013 | Unit/Bench | Menu-like screens use line 1 as section title and lower lines for items | `test_lcd_menu_view`, `test_lcd_preset_view`, `test_lcd_test_view`, `test_lcd_manual_program_view`; LCD bench check pending | Implemented |
| REQ-UI-014 | Unit/Bench | LCD temperatures use `°C` consistently | `test_lcd_status_view`, `test_lcd_preset_view`, `test_lcd_manual_program_view`; LCD bench check pending | Implemented |
| REQ-UI-029 | Unit/Bench | LCD temperatures render with one decimal place | `test_lcd_status_view`, `test_lcd_preset_view`, `test_lcd_manual_program_view`, `test_lcd_test_view`; LCD bench check pending | Implemented |
| REQ-UI-015 | Unit/Bench | Compact LCD durations use `Xh Ym` in preset, status, and parameter views | `test_lcd_status_view`, `test_lcd_preset_view`, `test_lcd_manual_program_view`; LCD bench check pending | Implemented |
| REQ-FUNC-001 | Unit/Bench | Test mode supports direct on/off control shell | `test_test_mode_controller`, `test_lcd_test_view`; bench test-mode check pending | Implemented |
| REQ-FUNC-004 | Unit/Bench | Built-in preset selection shell is available from the menu | `test_preset_select_controller`, `test_lcd_preset_view`; bench preset check pending | Implemented |
| REQ-FUNC-016 | Unit/Bench | Confirming a preset starts the associated run profile from idle | `test_preset_run_controller`; preset start bench check pending | Implemented |
| REQ-FUNC-010 | Unit/Bench | Test-mode heat request forces fan ON within safety constraints | `test_test_mode_controller`; bench test-mode toggle check pending | Implemented |
| REQ-FUNC-017 | Unit/Bench | Manual program mode supports `Constant`, `Boost`, and `Fluctuant` mode selection | `test_manual_program_controller`, `test_lcd_manual_program_view`; bench manual-program check pending | Implemented |
| REQ-FUNC-018 | Unit/Bench | Manual constant mode exposes temperature and duration | `test_manual_program_controller`, `test_lcd_manual_program_view`; bench manual-program check pending | Implemented |
| REQ-FUNC-019 | Unit/Bench | Manual boost mode applies boost target first and enforces boost edit limits | `test_profile_engine`, `test_manual_program_controller`, `test_preset_run_controller`; bench manual-program check pending | Implemented |
| REQ-FUNC-020 | Unit/Bench | Manual fluctuating mode edits absolute upper/lower temperatures and phase durations with limits | `test_profile_engine`, `test_manual_program_controller`, `test_lcd_manual_program_view`; bench manual-program check pending | Implemented |
| REQ-FUNC-021 | Unit/Bench | Manual editor exposes `Salveaza` before `Inapoi` and supports 10-slot save flow | `test_manual_program_controller`, `test_lcd_manual_program_view`, `test_user_profile_store`, new UI wiring; bench save flow pending | Implemented |
| REQ-FUNC-022 | Unit/Bench | Unsaved manual `Start` asks `Da / Nu / Renunta` and auto-starts after confirmed save | `test_save_prompt_controller`; integration logic covered by native build and pending bench flow | Implemented |
| REQ-FUNC-023 | Unit/Bench | Unsaved manual `Inapoi` asks `Da / Nu / Renunta` and discards only on confirmed `Nu` | `test_save_prompt_controller`, `test_manual_program_controller`; bench flow pending | Implemented |
| REQ-FUNC-024 | Unit/Bench | Main menu exposes `Programe utilizator` with all 10 slots | `test_menu_controller`, `test_lcd_menu_view`, `test_user_profile_controllers`; bench browse pending | Implemented |
| REQ-FUNC-025 | Unit/Bench | Occupied/vacant saved profiles expose the correct actions | `test_user_profile_controllers`, new LCD/detail wiring; bench browse/edit/delete pending | Implemented |
| REQ-FUNC-026 | Unit/Bench | Main menu order and dynamic stop/pause/resume visibility match the product flow | `test_menu_controller`, `test_lcd_menu_view`; bench menu hierarchy check pending | Implemented |
| REQ-FUNC-027 | Unit/Bench | `Setari` exposes `Testare` as a submenu instead of a top-level main-menu entry | Native UI wiring plus LCD/menu tests; bench settings navigation pending | Implemented |
| REQ-FUNC-028 | Unit/Bench | `Testare` shows NTC temperature, AM2302 temperature, and AM2302 humidity before the output toggles | `test_test_mode_controller`, `test_lcd_test_view`; bench LCD/sensor check pending | Implemented |
| REQ-FUNC-029 | Unit/Bench | Invalid `Testare` sensor rows show a compact error string instead of a value | `test_lcd_test_view`; bench invalid-sensor display check pending | Implemented |
| REQ-FUNC-030 | Unit/Bench | `Setari` exposes `Calibrare NTC` before `Testare` | `test_settings_menu_controller`; bench settings navigation pending | Implemented |
| REQ-FUNC-031 | Unit/Bench | `Calibrare NTC` edits offset/scale and exposes save/restore/back actions | `test_ntc_calibration_controller`, `test_lcd_ntc_calibration_view`; bench calibration-editor check pending | Implemented |
| REQ-UI-016 | Unit/Bench | Status summary page shows program, Temp/RH, elapsed time, and remaining time | `test_lcd_status_view`; LCD bench check pending | Implemented |
| REQ-UI-017 | Unit/Bench | Finished state can be acknowledged from the status screen with short press | `test_preset_run_controller`; status acknowledgement bench check pending | Implemented |
| REQ-UI-018 | Unit/Bench | User-profile slot list shows all 10 slots as `Profil N` or `Profil N (nedef.)` | `test_user_profile_controllers`, `test_lcd_user_profile_slot_view`; LCD bench check pending | Implemented |
| REQ-UI-019 | Unit/Bench | Saving over an occupied slot requires overwrite confirmation | `test_confirm_replace_run_controller`; bench overwrite-confirm flow pending | Implemented |
| REQ-UI-020 | Unit/Bench | Deleting an occupied slot requires confirmation | `test_confirm_replace_run_controller`; bench delete-confirm flow pending | Implemented |
| REQ-UI-021 | Unit/Bench | Saving one profile automatically closes the slot list and resumes the triggering flow | Native UI wiring; bench save-return flow pending | Implemented |
| REQ-UI-023 | Unit/Bench | Encoder rotation on status cycles between the available status pages | Native UI wiring plus LCD/status bench check pending | Implemented |
| REQ-UI-024 | Unit/Bench | Additional status pages show program parameters and output states line-by-line | `test_lcd_status_view`; LCD bench check pending | Implemented |
| REQ-UI-025 | Review/Bench | Status uses compact program labels when the full source label would overflow `Program:` line width | Code review plus LCD bench check pending | Implemented |
| REQ-UI-026 | Unit/Bench | `Oprire program` requires explicit confirmation before stopping the run | Native UI wiring; bench stop-confirm flow pending | Implemented |
| REQ-UI-027 | Unit/Bench | `Pauza program` appears only in `RULARE` and resumes through the existing resume flow | `test_menu_controller`, `test_lcd_menu_view`, `test_preset_run_controller`; bench pause/resume flow pending | Implemented |
| REQ-PERSIST-001 | Unit | Calibration defaults are explicit and persistable | `test_config` thermistor calibration default tests plus `test_ntc_calibration_store` | Implemented |
| REQ-UI-005 | Unit/Bench | Status summary shows blinking play/pause indicator top-right in `RULARE`/`PAUZA`, blank otherwise | `test_lcd_status_view`; LCD bench test pending | Implemented |
| REQ-UI-006 | Unit/Bench | Finish alarm starts after cooldown | State machine plus buzzer/backlight bench check | Planned |
| REQ-UI-007 | Unit/Bench | Fault alarm continues until acknowledgement | State machine plus buzzer/backlight bench check | Planned |
| REQ-LOG-004 | Unit | Events, parameters, outputs, warnings, faults are logged | `FakeLogSink` | Planned |
| REQ-LOG-005 | Unit | Periodic state log interval is 5 s | `FakeClock`, `FakeLogSink` | Planned |
| REQ-LOG-008 | Unit | English codes in logs, compact Romanian LCD messages | `test_log_formatter`, UI view tests | Planned |
| REQ-LOG-009 | Unit | Logged temperatures render with one decimal place | `test_log_formatter` | Passing |
| REQ-PERSIST-004 | Unit | Resume snapshots no more often than 15 min except lifecycle events | `FakeClock`, `FakePersistentStore` | Planned |
| REQ-PERSIST-005 | Unit | EEPROM version/checksum validation | `test_persistence_validation` | Planned |
| REQ-PERSIST-009 | Unit | Power loss/brown-out offers resume only after confirmation | `test_control_state_machine`, `FakePersistentStore` | Planned |
| REQ-PERSIST-010 | Unit | Watchdog reset does not allow resume | `test_control_state_machine`, `FakePersistentStore` | Planned |
| REQ-PERSIST-011 | Unit/Bench | EEPROM stores 10 user-defined manual profiles separately from resume state | `test_user_profile_store`; EEPROM bench save/load pending | Implemented |
| REQ-PERSIST-012 | Unit | Invalid/corrupt user-profile records are treated as vacant | `test_user_profile_store` checksum corruption case | Passing |
| REQ-PERSIST-013 | Unit | Saving one user profile updates only the selected slot layout | `test_user_profile_store`; byte-wise EEPROM adapter design review | Implemented |
| REQ-PERSIST-014 | Review/Unit | User-profile storage remains separate from interrupted-run persistence | `docs/architecture.md`, `test_user_profile_store` scope, code review | Implemented |
| REQ-PERSIST-015 | Unit | EEPROM persists user NTC offset/scale in a dedicated validated schema | `test_ntc_calibration_store`; EEPROM bench save/load pending | Implemented |

## Manual Bench Tests

Before connecting real heater power:

- Verify relay polarity with safe low-voltage load.
- Verify fan relay behavior.
- Verify buzzer and backlight FET.
- Verify LCD 4x20 layout and top-right `Play/Pause` activity symbol behavior on the summary page.
- Verify encoder rotation, short press, `Inapoi` navigation, and stuck-button detection.
- Verify `Salveaza`, `Da / Nu / Renunta`, overwrite, delete, and `Programe utilizator` browse/edit/start flows.
- Verify `Setari -> Testare` navigation and the dynamic visibility/order of `Oprire program` and `Reluare program`.
- Verify `Setari -> Calibrare NTC -> Inapoi` navigation and confirm the next entry starts from `Calibrare NTC`, not from `Inapoi`.
- Verify `Testare` shows the three sensor rows first and renders `Eroare` per row when a sensor reading is invalid.
- Verify `Calibrare NTC` can change `Offset` in 0.1 C steps and `Scala` in 0.01 steps, save them, power-cycle, and reload the same values.
- Verify primary thermistor analog readings and calibration visibility.
- Verify DHT22/AM2302 temperature/RH reporting and warning on disconnect.
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

- A manual `Constant` run can be configured, started, paused, resumed, finished, and acknowledged while preserving the required output behavior.
- A manual `Boost` run starts with the boost target, then continues at the base target after the boost phase expires.
- A manual `Fluctuant` run starts at `Tsup` and alternates with `Tinf` using the configured phase durations.
- A fluctuating run alternates between configured low/high phase targets.
- Starting a manual run while another run is active offers `Da`/`Nu` replacement confirmation.
- Manual profiles can be saved into one of 10 EEPROM-backed user slots, overwritten only after confirmation, and later started or edited from `Programe utilizator`.
- Hard fault scenarios force heater/fan OFF immediately and prevent new run start until acknowledgement.
- Power loss/brown-out during active run offers resume only after user confirmation.
- Watchdog reset during active run does not allow resume.
- Logs are emitted on USB and secondary serial in the required structured English format.
- LCD UI remains Romanian, fits 4x20 constraints, and shows the top-right `Play/Pause` activity symbol only on the summary page when applicable.
