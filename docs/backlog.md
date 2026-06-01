# Backlog

This backlog is a living plan. Keep items focused enough that each branch can map to one small group of related changes.

## Phase 0: Project Baseline

- [x] Inspect current repository and preserve existing setup knowledge.
- [x] Create durable project context and discovery log.
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
- [ ] Add hardware config placeholders.
- [x] Keep firmware buildable on Mega2560.

## Phase 5: Core Logic

- [ ] Implement profile engine.
- [ ] Implement hysteresis heater control.
- [ ] Implement safety/fault detector.
- [ ] Implement run state machine.
- [ ] Add unit tests for each module.

## Phase 6: Hardware Adapters

- [ ] Add PT50 analog reader and calibration model.
- [ ] Add AHT-like sensor adapter.
- [ ] Add relay output adapter with configurable polarity.
- [ ] Add LCD, encoder, buzzer, and backlight adapters.

## Phase 7: UI And Presets

- [ ] Document Romanian 4x20 menu flow.
- [ ] Implement status screen with bottom-right heartbeat.
- [ ] Implement preset selection and run configuration.
- [ ] Add built-in presets after source values are confirmed.

## Phase 8: Verification And Safety Review

- [ ] Run unit tests.
- [ ] Run PlatformIO build.
- [ ] Create manual bench test checklist.
- [ ] Review risks and safety behavior before real heater testing.
