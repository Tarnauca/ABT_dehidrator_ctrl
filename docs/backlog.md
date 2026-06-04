# Backlog

This backlog is a living plan. Keep items focused enough that each branch can map to one small group of related changes.

## Phase 0: Project Baseline

- [x] Inspect current repository and preserve existing setup knowledge.
- [x] Create durable project context and discovery log.
- [x] Create narrative development logbook and process-tracking rule.
- [x] Create reusable learning notes and learning-note workflow rule.
- [x] Create initial requirements, risks, test plan, hardware, UI, coding standards, ADRs, and agent definitions.
- [x] Commit and push the first documentation baseline.

## Phase 1: Requirements Baseline

- [ ] Incorporate the user's full draft requirements when available.
- [x] Convert requirements into stable IDs.
- [x] Review requirements with Requirements Analyst and Safety Reviewer.
- [x] Add initial acceptance criteria and traceability notes.

## Phase 2: Architecture

- [x] Define module boundaries for control, profiles, faults, hardware adapters, UI, logging, persistence, and scheduler.
- [x] Define configuration layout.
- [x] Define test strategy for pure logic and hardware adapters.
- [x] Review architecture against ADRs and safety requirements.

## Phase 3: PlatformIO And Test Setup

- [x] Audit current PlatformIO targets.
- [x] Propose simplification to Mega2560 plus native test target.
- [x] Ask approval before editing `platformio.ini`.
- [x] Add first pure C++ unit test example.

## Phase 4: Firmware Skeleton

- [x] Add minimal domain/interface skeleton.
- [x] Replace blink stub with cooperative scheduler skeleton.
- [x] Add logging abstraction and mirrored serial output.
- [x] Add hardware config placeholders.
- [x] Keep firmware buildable on Mega2560.

## Phase 5: Core Logic

- [x] Implement profile engine.
- [x] Implement hysteresis heater control.
- [x] Implement safety/fault detector.
- [x] Implement run state machine.
- [x] Implement startup self-check and boot state handling.
- [x] Add unit tests for each module.

## Phase 6: Hardware Adapters

- [x] Add primary thermistor conversion model and interface-based reader.
- [x] Add concrete Arduino thermistor analog adapter for bring-up.
- [ ] Confirm NTC divider wiring and calibration on real hardware.
- [x] Add interface-based secondary temp/RH reader.
- [x] Add concrete Arduino DHT22/AM2302 sensor adapter for bring-up.
- [ ] Confirm real DHT22/AM2302 module compatibility and bench readings.
- [x] Add relay output adapter with configurable polarity.
- [x] Add buzzer and backlight alarm output adapter.
- [x] Add LCD and encoder bring-up adapters.

## Phase 7: UI And Presets

- [x] Document Romanian 4x20 menu flow.
- [x] Implement initial status screen with bottom-right heartbeat.
- [x] Implement initial encoder-driven menu shell.
- [x] Implement initial manual mode screen shell.
- [x] Split the original manual shell into `Mod manual` program setup and `Testare` direct-output bring-up.
- [x] Implement preset selection and run configuration shell.
- [x] Wire preset selection into the real run lifecycle and status screen.
- [ ] Add built-in presets after source values are confirmed.

## Phase 8: Verification And Safety Review

- [x] Run unit tests.
- [ ] Run PlatformIO build.
- [ ] Create manual bench test checklist.
- [ ] Review risks and safety behavior before real heater testing.
