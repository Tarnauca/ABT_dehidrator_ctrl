# Development Logbook

This logbook preserves the process flow of the project: user questions,
Codex actions, agent calls, review outputs, commits, merges, and the reasoning
that led to important decisions.

It is intentionally narrative and learning-oriented. It does not replace ADRs,
requirements, review files, backlog items, or Git history. Instead, it links
those artifacts together so the development story can be revisited later.

## Entry Template

```md
### YYYY-MM-DD - Short Title

**Context**
What was happening before this entry.

**Trigger**
What caused the action or decision.

**Participants**
- User
- Codex
- Agent: Safety Reviewer/Test Engineer
- Agent: Safety Reviewer/Test Engineer
- Agent: Test Engineer
- Agent: Test Engineer
- Agent: ...

**Actions**
- What changed or was reviewed.

**Agent Calls**
- Agent name, purpose, and review file if any.

**Decisions**
- Decision, rationale, alternatives, and follow-up.

**Commits / Branches**
- Branch:
- Commit:
- Merge commit:

**Verification**
- Tests/builds run and result.

**Links**
- Related docs, reviews, ADRs, requirements, backlog items, or files.
```

## Entries

### 2026-06-04 - Menu Navigation Rules Made Durable

**Context**
The UI/menu bring-up had evolved through bench testing. The code already used
explicit `Inapoi` entries in the main menu, preset screen, and manual mode, but
some higher-level docs still described long-press back/cancel behavior from an
older draft.

**Trigger**
The user asked to make sure the updated menu behavior and related requirements
were documented so later phases will build on the correct navigation model.

**Participants**
- User
- Codex

**Actions**
- Updated requirements to define `Inapoi` as the final entry on every menu-like
  LCD screen.
- Documented that selecting `Inapoi` returns exactly one level up.
- Updated LCD/UI notes so the title stays on the first line and menu entries
  occupy lines two through four.
- Clarified that long press currently has no required navigation role.
- Updated the test plan so menu-navigation verification follows the implemented
  short-press and `Inapoi` rules.
- Adjusted hardware notes to describe the encoder button behavior consistently.

**Decisions**
- The project now treats explicit `Inapoi` navigation as a durable UI rule, not
  just an implementation detail.
- Compact LCD preset summaries use `Xh Ym`, while the underlying duration model
  may still use `HH:MM` where a structured editable value is more natural.
- LCD temperatures should always render with `°C` where temperature is shown.

**Commits / Branches**
- Branch: `main` worktree, not yet committed
- Commit: pending
- Merge commit: pending

**Verification**
- No code change in this documentation pass.
- Existing automated coverage for menu, preset, manual, and LCD formatting
  remains the current verification base.

**Links**
- `docs/requirements.md`
- `docs/test-plan.md`
- `docs/user-ui-ro.md`
- `docs/hardware.md`

### 2026-06-04 - Preset Selection Starts A Real Run

**Context**
The UI already had a preset-selection shell, but confirming a preset only
logged the choice and returned to status. The lifecycle state machine,
profile engine, and hysteresis controller already existed separately.

**Trigger**
After stabilizing menu navigation, the next requested phase was to connect
`Pornire preset` to a real run path.

**Participants**
- User
- Codex

**Actions**
- Added a pure `PresetRunController` to bridge preset choice, run lifecycle,
  profile target selection, and heater/fan command generation.
- Wired `main.cpp` so preset confirmation starts a real run from idle.
- Updated the status screen to show Romanian lifecycle labels from the active
  run instead of the fixed `INACTIV` placeholder.
- Added periodic run advancement and output application in the cooperative loop.
- Extended periodic state logs with run state, active preset token, and logical
  heater/fan status.
- Added finish acknowledgement from the status screen with a short press.

**Decisions**
- This slice uses the existing pure `RunStateMachine`, `ProfileEngine`, and
  `TemperatureControl` modules rather than creating a second simplified run path.
- Relay outputs are applied through the existing adapter, while heater/fan pins
  remain harmlessly unassigned on the current bench configuration.
- The structured log line stayed compact by shortening heater/fan field names
  to `h` and `f` instead of making the log buffer expansion do all the work.

**Commits / Branches**
- Branch: pending
- Commit: pending
- Merge commit: pending

**Verification**
- Added native tests for the preset-run controller.
- Reworked logging tests to cover run-state fields in the periodic state line.
- Mega2560 build was rechecked after AVR C++ compatibility fixes.

**Links**
- `include/dehydrator/app/PresetRunController.h`
- `src/main.cpp`
- `docs/requirements.md`
- `docs/test-plan.md`
- `docs/user-ui-ro.md`

### 2026-05-31 - Discovery And Agentic Workflow Baseline

**Context**
The project started as a working PlatformIO blink stub for a food dehydrator
controller. The user wanted to understand agentic development before creating
agents or code.

**Trigger**
The user asked how to start with agents, what agent types should exist, how to
reuse them, and how agentic mode development works.

**Participants**
- User
- Codex

**Actions**
- Conducted a long requirements and workflow discovery session.
- Identified the project as a serious hobby dehydrator controller with
  product-minded safety recommendations.
- Established that durable repo docs are needed because chat/session memory is
  not reliable across devcontainer rebuilds.
- Created initial documentation baseline with project context, discovery log,
  requirements, risks, test plan, architecture, hardware notes, UI notes, ADRs,
  backlog, coding standards, agent workflow, and reusable agent definitions.

**Decisions**
- Use Arduino Mega2560, not Uno/Nano.
- Use PlatformIO.
- Treat PT50 as the primary control temperature sensor.
- Use AHT-like sensor for secondary temp/RH telemetry and warning-only failure.
- Use Romanian LCD UI, English logs/code/docs.
- Use branch/commit workflow.
- Use hybrid agent invocation: Codex recommends agents at checkpoints, user can
  invoke agents manually.
- Persist project memory in repo documentation.

**Commits / Branches**
- Branch: `docs-project-context-and-agent-workflow`
- Commit: `1187b30 Document project context and agent workflow`
- Commit: `643541b Mark project baseline backlog complete`
- Merge commit: `474875b Merge project context and agent workflow baseline`

**Verification**
- `platformio run` passed for `megaatmega2560` after running outside the
  sandbox so PlatformIO could access its cache.

**Links**
- `docs/project-context.md`
- `docs/project-discovery-log.md`
- `docs/agents.md`
- `docs/backlog.md`
- `docs/decisions/ADR-001-target-platform.md`
- `docs/decisions/ADR-002-development-workflow.md`
- `docs/decisions/ADR-003-documentation-and-agent-workflow.md`
- `docs/decisions/ADR-004-firmware-architecture-style.md`
- `docs/decisions/ADR-005-control-and-safety-baseline.md`

### 2026-05-31 - Requirements Review Becomes Durable

**Context**
The initial requirements draft existed, but the user wanted the Requirements
Analyst to review it.

**Trigger**
The user invoked: Requirements Analyst on `docs/requirements.md`.

**Participants**
- User
- Codex
- Agent: Requirements Analyst
- Agent: Safety Reviewer

**Actions**
- Spawned a Requirements Analyst review agent.
- Requirements review found more issues than the four open questions first
  summarized by Codex.
- The user noticed the full review had more detail and asked whether the review
  was persisted.
- Codex clarified that spawned-agent results are not automatically persisted.
- Added `docs/reviews/requirements-analyst-001.md`.
- Updated requirements with clearer pause/resume, stop, finish cooldown,
  warning behavior, startup checks, telemetry output-only behavior, and
  persistence/resume rules.
- Spawned Safety Reviewer after requirements changed.
- Added `docs/reviews/safety-reviewer-001.md`.
- Updated project context and risk log based on safety review findings.

**Decisions**
- Agent reviews that matter should be saved under `docs/reviews/`.
- Heater forced-off threshold is greater than 75 C, while 75 C remains the max
  allowed setpoint.
- No-temperature-rise detection uses accumulated heater ON command time.
- Heater-stuck-ON detection starts measuring after a 2-minute post-heater-off
  grace period.
- Finish alarm starts only after the 3-minute cooldown.

**Commits / Branches**
- Branch: `docs/requirements-review`
- Commit: `90244e9 Refine requirements after agent reviews`
- Merge commit: `615e0d0 Merge requirements review updates`

**Verification**
- Documentation-only branch. Review consistency was checked through
  Requirements Analyst and Safety Reviewer outputs.

**Links**
- `docs/requirements.md`
- `docs/risks.md`
- `docs/project-context.md`
- `docs/reviews/requirements-analyst-001.md`
- `docs/reviews/safety-reviewer-001.md`

### 2026-05-31 - Architecture And Test Plan Review

**Context**
The architecture and test plan were high-level drafts. Before coding, the user
wanted architecture readiness.

**Trigger**
The user invoked System Architect on `docs/architecture.md`,
`docs/requirements.md`, and ADRs, then invoked Test Engineer on
`docs/test-plan.md` and requirements.

**Participants**
- User
- Codex
- Agent: System Architect
- Agent: Test Engineer

**Actions**
- Spawned System Architect review.
- Persisted review as `docs/reviews/system-architect-001.md`.
- Expanded architecture with runtime state model, proposed source layout,
  module boundaries, data flow, safety invariant enforcement, fault detector
  inputs, configuration ownership, interface responsibilities, and native-test
  implications.
- Spawned Test Engineer review.
- Persisted review as `docs/reviews/test-engineer-001.md`.
- Expanded the test plan with native test groups, test fakes, requirement
  coverage matrix, safety boundary tests, and acceptance criteria.

**Decisions**
- `RelayOutputs` should defensively reject or sanitize heater ON while fan OFF.
- Finish cooldown is its own state.
- Logging should eventually fan out through a simple dispatcher with no dynamic
  allocation.
- Native tests should pass before firmware upload for affected pure logic once
  implementation begins.
- First code branch should create a testable skeleton plus one sample unit test.

**Commits / Branches**
- Branch: `docs/architecture-review`
- Commit: `9d43100 Refine architecture and test plan after reviews`
- Merge commit: `6d13e3f Merge architecture and test plan review`

**Verification**
- Documentation-only branch. Architecture and test-plan consistency were
  checked through agent reviews.

**Links**
- `docs/architecture.md`
- `docs/test-plan.md`
- `docs/reviews/system-architect-001.md`
- `docs/reviews/test-engineer-001.md`

### 2026-06-01 - First Testable Firmware Skeleton

**Context**
The project still contained the PlatformIO blink starter. The architecture
called for pure logic, interfaces, and native tests.

**Trigger**
The user created `feature/testable-firmware-skeleton` and approved editing
`platformio.ini` after audit.

**Participants**
- User
- Codex

**Actions**
- Added PlatformIO `native` environment.
- Removed old Uno/Nano environments because ADR-001 dropped them from scope.
- Added minimal documented public headers for runtime state, clock, and output
  command abstraction.
- Added first native Unity test.
- Updated coding standards to require Doxygen-style comments for public headers
  after the user asked for more explicit code documentation.
- Updated backlog after the skeleton work.

**Decisions**
- Public headers shall use Doxygen-style comments for public types, functions,
  params, return values, safety assumptions, timing assumptions, units, and side
  effects where applicable.
- Code and corresponding documentation should move together in the same branch.
- Use semantic commit messages going forward.

**Commits / Branches**
- Branch: `feature/testable-firmware-skeleton`
- Commit: `eee9b6d feat(doamin): add testable firmware skeleton`
- Merge commit: `1171251 Merge testable firmware skeleton`
- Note: commit scope contains a typo, `doamin`; history was not rewritten after
  push/merge.

**Verification**
- `platformio test -e native` passed.
- `platformio run -e megaatmega2560` passed.

**Links**
- `platformio.ini`
- `include/dehydrator/domain/RunState.h`
- `include/dehydrator/interfaces/Clock.h`
- `include/dehydrator/interfaces/OutputController.h`
- `test/test_domain_skeleton/test_domain_skeleton.cpp`
- `docs/coding-standards.md`

### 2026-06-01 - Backlog Updates Become Part Of Agent Workflow

**Context**
The backlog was being updated manually after branches, but the workflow did not
yet require agents or Codex to check backlog impact.

**Trigger**
The user asked whether agent descriptions should instruct agents to update the
backlog whenever a feature is implemented.

**Participants**
- User
- Codex

**Actions**
- Discussed whether this belonged in the current feature branch or a dedicated
  docs/process branch.
- User explored the idea of parking work with `stash` versus committing a
  feature branch before switching.
- Added backlog-impact rule to agent workflow, Documentation Maintainer, and
  project context.

**Decisions**
- Branches that complete, start, split, remove, or materially change planned
  work must check/update `docs/backlog.md`, or explicitly state that no backlog
  update was needed.
- Documentation Maintainer checks backlog impact before merge.

**Commits / Branches**
- Branch: `docs/backlog-agent-workflow`
- Commit: `20fc4d7 docs(agents): require backlog impact checks`
- Merge commit: `4f965c5 Merge backlog agent workflow update`

**Verification**
- Documentation-only branch.

**Links**
- `docs/agents.md`
- `docs/agents/documentation-maintainer.md`
- `docs/project-context.md`

### 2026-06-01 - Cooperative Scheduler Shell

**Context**
The firmware still used a blocking blink loop. Requirements and coding
standards called for non-blocking `millis()` timing.

**Trigger**
After the testable skeleton was merged, Codex recommended the next branch:
`feature/cooperative-scheduler-shell`.

**Participants**
- User
- Codex

**Actions**
- Added documented `PeriodicTask` helper.
- Replaced blocking `delay()` blink loop with cooperative LED and state-log
  periodic tasks.
- Added native tests for timing behavior.
- Updated backlog to mark scheduler shell complete.

**Decisions**
- Use small cooperative scheduler helpers rather than an RTOS.
- Periodic tasks are due on first call so startup state can be emitted without
  waiting for an interval.

**Commits / Branches**
- Branch: `feature/cooperative-scheduler-shell`
- Commit: `de872fc feat(scheduler): Implement scheduling mechanism`
- Merge commit: `f7b0f14 Merge cooperative scheduler shell`

**Verification**
- `platformio test -e native`: 7 tests passed.
- `platformio run -e megaatmega2560`: success.

**Links**
- `include/dehydrator/app/PeriodicTask.h`
- `test/test_periodic_task/test_periodic_task.cpp`
- `src/main.cpp`
- `docs/backlog.md`

### 2026-06-01 - Logging Abstraction And Secondary Serial Scope

**Context**
The scheduler shell emitted logs directly through `Serial.print()`. Phase 4
still needed logging abstraction and mirrored secondary serial output.

**Trigger**
Codex recommended `feature/logging-abstraction` because logging should be
abstracted before more firmware logic depends on direct serial calls.

**Participants**
- User
- Codex
- Agent: System Architect

**Actions**
- Added `LogSink`, `LogDispatcher`, and `LogFormatter`.
- Wired USB `Serial` and secondary `Serial1` as mirrored structured-log sinks.
- Added native logging tests for formatting, truncation detection, null token
  handling, null dispatcher writes, and fan-out.
- Updated backlog to mark logging abstraction complete.
- Ran System Architect review after implementation because logging affected
  architecture boundaries and secondary serial behavior.
- Persisted review as `docs/reviews/system-architect-logging-001.md`.
- Fixed review findings by adding stable `WARN code=log_truncated` fallback
  lines, safe null token handling, and null line guarding.
- User questioned whether the sink approach was over-engineered because the
  second UART may later become Modbus RTU.
- Codex clarified that the current mechanism is log mirroring only, not a
  future telemetry protocol abstraction.
- Added architecture note that Modbus RTU or another gateway protocol must be a
  separate adapter, not a `LogSink`.

**Decisions**
- `LogFormatter` and `LogSink` are useful for testability and structured logs.
- `LogDispatcher` is acceptable as a simple log mirror for bring-up.
- Do not add routing/filtering/protocol abstraction now.
- If `Serial1` later becomes Modbus RTU, it must be removed from log mirroring
  and implemented as a separate protocol adapter.

**Commits / Branches**
- Branch: `feature/logging-abstraction`
- Commit: `a67c745 feat(logging): add mirrored log dispatcher`
- Merge commit: `bf70d3b Merge logging abstraction`

**Verification**
- `platformio test -e native`: 14 tests passed.
- `platformio run -e megaatmega2560`: success.

**Links**
- `include/dehydrator/logging/LogSink.h`
- `include/dehydrator/logging/LogDispatcher.h`
- `include/dehydrator/logging/LogFormatter.h`
- `test/test_logging/test_logging.cpp`
- `docs/reviews/system-architect-logging-001.md`
- `docs/architecture.md`

### 2026-06-01 - Hardware Config Placeholders

**Context**
Phase 4 still had one open item: hardware config placeholders. The scheduler
and logging shell still owned constants such as baud rate, task intervals, log
buffer sizes, and the built-in LED pin directly in `main.cpp`.

**Trigger**
Codex selected the next backlog item after the logbook workflow was merged:
centralize early hardware/runtime configuration before adding more firmware
logic.

**Participants**
- User
- Codex

**Actions**
- Added documented config headers for hardware placeholders and runtime
  settings.
- Moved scheduler, serial, logging, and status LED constants out of `main.cpp`
  and into config headers.
- Added native config tests to keep key defaults visible.
- Updated backlog to mark hardware config placeholders complete.
- User asked why hardware pins were grouped into a struct.
- Codex explained the design rationale and captured it as a reusable learning
  note.

**Decisions**
- Placeholder wiring values are centralized but must be reviewed before real
  heater/fan hardware is connected.
- Relay polarity is represented in config early, even though relay adapters are
  not implemented yet.
- Runtime constants used by the current shell are centralized now so future
  modules do not grow scattered local constants.
- Hardware pins are grouped in a `HardwarePins` struct because pins are a
  coherent category of wiring assumptions, separate from relay polarity and
  bus/address settings.

**Commits / Branches**
- Branch: `feature/hardware-config-placeholders`
- Commit: `badf255 feat(config): add hardware placeholders`
- Merge commit: `9964a2b Merge hardware config placeholders`

**Verification**
- `platformio test -e native`: 18 tests passed.
- `platformio run -e megaatmega2560`: success.

**Links**
- `include/dehydrator/config/HardwareConfig.h`
- `include/dehydrator/config/RuntimeConfig.h`
- `test/test_config/test_config.cpp`
- `src/main.cpp`
- `docs/backlog.md`
- `docs/learning/architecture-decisions.md`

### 2026-06-01 - Learning Notes Workflow

**Context**
The logbook captured process flow, but reusable learning explanations were
still embedded in the chat and mixed into chronological entries.

**Trigger**
The user asked to preserve explanations such as Git usage exercises and the
hardware pin struct rationale in a dedicated place for later review.

**Participants**
- User
- Codex

**Actions**
- Added `docs/learning/` as a study-oriented documentation area.
- Backfilled learning notes for Git workflow, architecture decisions, embedded
  C++, and testing.
- Updated agent workflow and Documentation Maintainer responsibilities so
  learning-note impact is checked when meaningful learning moments occur.

**Decisions**
- The logbook remains chronological.
- Learning notes explain reusable concepts.
- Meaningful user questions, design rationale, debugging lessons, workflow
  lessons, and agent/process lessons should trigger a learning-notes impact
  check.

**Commits / Branches**
- Branch: `docs/learning-notes-workflow`
- Commit: `1972817 docs(learning): add reusable learning notes`
- Merge commit: `0251714 merge(learning): merge learning notes workflow`

**Verification**
- Documentation-only change.

**Links**
- `docs/learning/README.md`
- `docs/learning/git-workflow.md`
- `docs/learning/architecture-decisions.md`
- `docs/learning/embedded-cpp.md`
- `docs/learning/testing.md`

### 2026-06-02 - Profile Engine

**Context**
Phase 5 core logic started after the firmware skeleton, scheduler, logging, and
configuration placeholders were in place. The next useful domain slice was the
profile engine because later temperature control and state-machine behavior
need a current target temperature.

**Trigger**
The user said "let's go" after the current status summary recommended
`feature/profile-engine`.

**Participants**
- User
- Codex

**Actions**
- Added a pure C++ `ProfileEngine` with fixed and fluctuating profile support.
- Added profile config and target result types using integer Celsius and minute
  durations.
- Added native tests for fixed profiles, fluctuating high/low cycles,
  completion, safety setpoint validation, max duration validation, and invalid
  phase durations.
- Ran Test Engineer review and added boundary tests/traceability improvements
  from the findings.
- Updated backlog and test-plan traceability.

**Decisions**
- Profile evaluation receives active elapsed seconds from the caller. It does
  not own clocks or pause/resume state.
- Fluctuating profiles start with the high-temperature phase, then alternate
  high/low phases.
- Profile validation rejects setpoints above 75 C and durations beyond 99 hours.
- In fluctuating mode, `targetTempC` is user-entered average metadata and must
  also obey the 75 C setpoint limit.
- Profile evaluation may still return a target when `complete == true`; later
  control/state-machine code should use the completion flag to stop control.

**Commits / Branches**
- Branch: `feature/profile-engine`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 35 tests passed.
- `platformio run -e megaatmega2560`: success.

**Links**
- `include/dehydrator/domain/ProfileEngine.h`
- `test/test_profile_engine/test_profile_engine.cpp`
- `docs/backlog.md`
- `docs/test-plan.md`
- `docs/reviews/test-engineer-profile-engine-001.md`

### 2026-06-02 - Run State Machine

**Context**
Phase 5 core logic had a tested profile engine. The next open backlog item was
the lifecycle state machine for start, pause, resume, finish cooldown, stop,
and hard fault behavior.

**Trigger**
The user approved proceeding to the next phase after the profile engine merge.

**Participants**
- User
- Codex

**Actions**
- Created `feature/run-state-machine`.
- Added a pure C++ `RunStateMachine` that owns run lifecycle transitions and
  scheduler-provided elapsed-time accounting.
- Added lifecycle-level output policy so the application coordinator can later
  combine run state with temperature control and hardware output adapters.
- Added native Unity tests for valid/invalid start, pause/resume, elapsed time
  suspension, normal finish cooldown, finish acknowledgement, confirmed stop,
  fault entry, and fault acknowledgement.
- Spawned a Test Engineer review and added tests for fault acknowledgement
  gating, fault transitions from multiple runtime states, stop transitions from
  non-running states, and zero-delta timing boundaries.
- Updated backlog and test-plan traceability.

**Agent Calls**
- Test Engineer reviewed the run state machine branch and found missing tests
  around hard-fault acknowledgement, fault transitions from runtime states,
  stop transition coverage, and timing boundaries.

**Decisions**
- Keep the state machine independent from Arduino APIs and hardware pins.
- Pass elapsed seconds into `update()` instead of reading a clock internally.
- Keep heater hysteresis and fault detection as separate modules; the state
  machine exposes whether heater control is allowed, not the final relay state.
- User-confirmed stop returns directly to idle with outputs off and no cooldown.
- Hard fault disables resume and exposes a fault alarm policy until
  acknowledgement.

**Commits / Branches**
- Branch: `feature/run-state-machine`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 48 tests passed.
- `platformio run -e megaatmega2560`: success.

**Links**
- `include/dehydrator/domain/RunStateMachine.h`
- `test/test_run_state_machine/test_run_state_machine.cpp`
- `docs/backlog.md`
- `docs/test-plan.md`
- `docs/reviews/test-engineer-run-state-machine-001.md`

### 2026-06-02 - Hysteresis Temperature Control

**Context**
After the run state machine was merged, Phase 5 still needed the heater control
logic that turns profile targets and PT50 readings into a logical heater
request.

**Trigger**
The user asked Codex to commit/merge the state machine and continue with the
next phase.

**Participants**
- User
- Codex

**Actions**
- Created `feature/hysteresis-temperature-control`.
- Added `TemperatureControl`, a pure C++ relay hysteresis controller.
- Added `ControlConfig` defaults for hysteresis, optional minimum heater
  on/off timing, and the 75 C force-off threshold.
- Added native Unity tests for hysteresis ON/OFF thresholds, the 75/76 C safety
  boundary, run-policy blocking, previous-command forced-off behavior, minimum
  relay timing, and safety override of minimum ON time.
- Added a config test for the force-off threshold.
- Spawned a Test Engineer review and fixed findings around 10 s production
  relay timing, REQ-SAFE-004 wording, config coverage, and over-temperature
  reason reporting when run policy also blocks heat.
- Updated backlog and test-plan traceability.

**Agent Calls**
- Test Engineer reviewed the hysteresis temperature control branch and found
  default relay timing/documentation mismatches plus one reason-reporting edge
  case.

**Decisions**
- Keep temperature control separate from the run state machine and hardware
  relay adapter.
- Treat `currentTempC > 75` as a control-level forced-off condition, while
  allowing exactly 75 C to be governed by normal hysteresis rather than labeled
  as a safety force-off.
- Use scheduler-provided elapsed seconds for optional minimum relay timing.
- Allow hard safety and run-state policy to override relay minimum timing.

**Commits / Branches**
- Branch: `feature/hysteresis-temperature-control`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 73 tests passed.
- `platformio run -e megaatmega2560`: success.

**Links**
- `include/dehydrator/domain/TemperatureControl.h`
- `include/dehydrator/config/RuntimeConfig.h`
- `test/test_temperature_control/test_temperature_control.cpp`
- `test/test_config/test_config.cpp`
- `docs/backlog.md`
- `docs/test-plan.md`
- `docs/reviews/test-engineer-temperature-control-001.md`

### 2026-06-02 - Safety Fault Detector

**Context**
After profile, run-state, and heater-control logic were merged, the remaining
Phase 5 core module was hard-fault detection.

**Trigger**
The user asked Codex to continue with the next phase after merging hysteresis
temperature control.

**Participants**
- User
- Codex

**Actions**
- Created `feature/fault-detector`.
- Added `SafetyConfig` defaults for PT50 plausible range, 80 C hard fault,
  no-temperature-rise timing, suspected stuck-heater timing, and stuck-button
  timing.
- Added `FaultDetector`, a pure C++ latched hard-fault detector.
- Added native Unity tests for PT50 invalid/out-of-range, 80 C over-temperature,
  no-rise fault timing, stuck-heater grace/rise timing, stuck button, watchdog
  reset during run, first-fault latching, and reset behavior.
- Added config tests for the agreed safety thresholds.
- Spawned a Safety Reviewer/Test Engineer review and fixed findings around
  accumulated heater-ON timing, stuck-heater monitoring lifetime, idle false
  positives, and watchdog coverage wording.
- Marked Phase 5 core logic and unit-test coverage complete in the backlog.

**Agent Calls**
- Safety Reviewer/Test Engineer reviewed the fault detector branch and found
  safety-relevant mismatches in no-rise accumulation and stuck-heater timing.

**Decisions**
- Latch the first hard fault until `reset()` so later symptoms cannot hide the
  original fault reason before user acknowledgement.
- Keep watchdog-reset-during-run as an explicit input for now; persistence and
  boot reset-cause storage remain future integration work.
- No-rise tracking accumulates heater-ON command time across relay cycles and
  resets only after the required temperature rise is observed.
- Stuck-heater monitoring starts only after the 2-minute heater-OFF grace
  period and only after the heater was previously commanded ON.
- Stuck-heater monitoring rebaselines after each 5-minute window without a
  fault, so detection does not silently expire after the first window.

**Commits / Branches**
- Branch: `feature/fault-detector`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 93 tests passed.
- `platformio run -e megaatmega2560`: success.

**Links**
- `include/dehydrator/domain/FaultDetector.h`
- `include/dehydrator/config/RuntimeConfig.h`
- `test/test_fault_detector/test_fault_detector.cpp`
- `test/test_config/test_config.cpp`
- `docs/backlog.md`
- `docs/test-plan.md`
- `docs/reviews/safety-reviewer-fault-detector-001.md`

### 2026-06-02 - PT50 Calibration And Reader

**Context**
Phase 5 core logic was complete. Phase 6 started with the primary PT50 sensor
path because it feeds control and safety logic.

**Trigger**
The user asked to proceed after Codex recommended starting Phase 6 with the
PT50 analog reader and calibration model.

**Participants**
- User
- Codex

**Actions**
- Created `feature/pt50-calibration-reader`.
- Added `CalibrationConfig` defaults for PT50 divider conversion, offset,
  scale, ADC range, and plausible temperature range.
- Added `Pt50SensorModel`, a pure ratiometric ADC-to-temperature converter.
- Added `AnalogInput`, a project-owned interface for raw ADC access.
- Added `Pt50Reader`, a thin wrapper that reads an ADC channel through
  `AnalogInput` and applies the PT50 model.
- Added native Unity tests for 0 C and 100 C conversion, calibration offset and
  scale, invalid ADC endpoints, plausible-range rejection, divider orientation,
  and reader channel usage.
- Spawned a Test Engineer review and fixed a safety-relevant near-rail ADC
  overflow path before commit.
- Updated hardware notes, backlog, and test-plan traceability.

**Agent Calls**
- Test Engineer reviewed PT50 conversion and reader behavior, focusing on ADC
  math, divider assumptions, overflow, validity handling, tests, and docs.

**Decisions**
- Keep PT50 conversion ratiometric because the divider is supplied from the same
  VCC used as ADC reference.
- Make divider orientation configurable because final wiring has not been
  confirmed.
- Use integer math with explicit 64-bit intermediate calculations for
  resistance and calibration scaling to avoid hidden overflow.
- Validate plausible temperature in a wide integer type before assigning the
  final `int16_t` temperature so near-rail ADC values cannot wrap into a
  valid-looking reading.
- Treat the 100 ohm fixed resistor and default divider orientation as
  placeholders that must be checked before real heater testing.
- Keep the concrete Arduino `analogRead()` adapter as a separate backlog item
  until wiring is confirmed.

**Commits / Branches**
- Branch: `feature/pt50-calibration-reader`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 106 tests passed.
- `platformio run -e megaatmega2560`: success.

**Links**
- `include/dehydrator/domain/Pt50SensorModel.h`
- `include/dehydrator/sensors/Pt50Reader.h`
- `include/dehydrator/interfaces/AnalogInput.h`
- `include/dehydrator/config/RuntimeConfig.h`
- `test/test_pt50_sensor/test_pt50_sensor.cpp`
- `test/test_config/test_config.cpp`
- `docs/hardware.md`
- `docs/backlog.md`
- `docs/test-plan.md`
- `docs/reviews/test-engineer-pt50-reader-001.md`

### 2026-06-02 - Relay Output Adapter

**Context**
Phase 6 hardware-adapter work had the PT50 interface-based reader merged. The
concrete PT50 Arduino adapter remains pending until wiring is confirmed, so the
next well-defined hardware slice was relay output handling.

**Trigger**
The user asked to proceed with the next phase after the PT50 branch was merged.

**Participants**
- User
- Codex

**Actions**
- Created `feature/relay-output-adapter`.
- Added `DigitalOutput`, a project-owned interface for MCU output pin access.
- Added `RelayOutputs`, an `OutputController` implementation for heater and
  fan relays.
- Added native Unity tests for relay polarity, startup force-off behavior,
  safe write ordering, unsafe command sanitization, fan-only operation, and
  unassigned placeholder pins.
- Spawned a Safety Reviewer/Test Engineer review and fixed findings around
  active-low startup pulse risk, mixed assigned/unassigned relay pins, and
  active-low ON coverage.
- Updated backlog, hardware notes, test-plan traceability, and the logbook.

**Agent Calls**
- Safety Reviewer/Test Engineer reviewed relay output safety, polarity,
  write ordering, unassigned pins, and documentation claims.

**Decisions**
- Keep Arduino `pinMode()`/`digitalWrite()` behind a `DigitalOutput` interface
  so relay behavior can be tested without hardware.
- Treat pin `255` as an unassigned placeholder and skip writes for it.
- Sequence heater/fan writes defensively: fan ON before heater ON, heater OFF
  before fan OFF.
- Sanitize unsafe commands in the relay adapter even though higher layers also
  enforce the heater/fan invariant.
- Preload inactive relay levels before configuring pins as outputs to avoid
  active-low startup pulses on AVR-style GPIO.
- Block heater ON when the fan relay pin is unassigned, even if the logical
  command requests fan ON.

**Commits / Branches**
- Branch: `feature/relay-output-adapter`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 116 tests passed.
- `platformio run -e megaatmega2560`: success.

**Links**
- `include/dehydrator/hardware/RelayOutputs.h`
- `include/dehydrator/interfaces/DigitalOutput.h`
- `test/test_relay_outputs/test_relay_outputs.cpp`
- `docs/backlog.md`
- `docs/hardware.md`
- `docs/test-plan.md`
- `docs/reviews/safety-reviewer-relay-output-adapter-001.md`

### 2026-06-02 - Alarm Output Adapter

**Context**
Phase 6 had relay outputs merged. The remaining LCD/encoder/AHT work depends
on library and wiring choices, but buzzer and LCD backlight outputs can use the
same digital-output abstraction as relays.

**Trigger**
The user approved continuing from the `feature/alarm-output-adapter` branch.

**Participants**
- User
- Codex

**Actions**
- Added active-level configuration for LCD backlight and buzzer outputs.
- Added `AlarmOutputs`, an `OutputController` implementation for buzzer and
  LCD backlight.
- Added native Unity tests for active-high and active-low startup levels,
  buzzer/backlight ON/OFF commands, placeholder pins, and ignoring heater/fan
  fields.
- Added config tests for default alarm output polarity.
- Updated backlog, hardware notes, test-plan traceability, and logbook.

**Decisions**
- Keep buzzer/backlight behind `DigitalOutput`, matching the relay adapter
  pattern.
- Preload inactive output levels during startup to avoid active-low pulses.
- Keep LCD and encoder adapters as separate pending work because they require
  more specific UI/input behavior and likely library choices.

**Commits / Branches**
- Branch: `feature/alarm-output-adapter`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 124 tests passed.
- `platformio run -e megaatmega2560`: success.

**Links**
- `include/dehydrator/hardware/AlarmOutputs.h`
- `include/dehydrator/config/HardwareConfig.h`
- `test/test_alarm_outputs/test_alarm_outputs.cpp`
- `test/test_config/test_config.cpp`
- `docs/backlog.md`
- `docs/hardware.md`
- `docs/test-plan.md`

### 2026-06-02 - AHT Interface-Based Reader

**Context**
The user asked to do the AHT part after the alarm output adapter was merged.
The exact AHT device/library is still not selected, so the implementation
focused on a testable interface-based reader.

**Trigger**
The user said "do the AHT part now".

**Participants**
- User
- Codex

**Actions**
- Created `feature/aht-sensor-adapter`.
- Added `AhtSensorDriver`, a project-owned interface for future AHT-like
  library adapters.
- Added `AhtReader`, a secondary telemetry reader that applies temperature and
  RH calibration offsets and validates plausible ranges.
- Added AHT calibration defaults to `CalibrationConfig`.
- Added native Unity tests for valid readings, invalid driver samples,
  temperature/RH offsets, temperature range rejection, RH above 100%, and
  negative calibrated RH.
- Added config tests for AHT calibration defaults.
- Updated backlog, hardware notes, test-plan traceability, and logbook.

**Decisions**
- Keep AHT as warning/telemetry-only in this slice; invalid AHT readings return
  `valid=false` and do not create hard faults.
- Use centi-Celsius and centi-percent RH at the driver boundary so a future
  concrete adapter can preserve precision without floating point.
- Leave the concrete Arduino AHT library adapter pending until the exact sensor
  library/device is confirmed.

**Commits / Branches**
- Branch: `feature/aht-sensor-adapter`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 133 tests passed.
- `platformio run -e megaatmega2560`: success.

**Links**
- `include/dehydrator/interfaces/AhtSensorDriver.h`
- `include/dehydrator/sensors/AhtReader.h`
- `include/dehydrator/config/RuntimeConfig.h`
- `test/test_aht_reader/test_aht_reader.cpp`
- `test/test_config/test_config.cpp`
- `docs/backlog.md`
- `docs/hardware.md`
- `docs/test-plan.md`

### 2026-06-02 - LCD And Encoder Bring-Up

**Context**
The user wants to deploy to the real Mega test device and check that the
existing cooperative logic does not block. They clarified that no heater or fan
is connected to the current test device, so bench risk is low for this slice.
They also approved using proven libraries, specifically `LiquidCrystal_I2C`,
instead of reimplementing common hardware drivers.

**Trigger**
The user asked what to do next and suggested adding LCD and encoder adapters for
real-device deployment. After the recommendation, the user said proven libraries
are acceptable.

**Participants**
- User
- Codex

**Actions**
- Created `feature/lcd-encoder-bringup`.
- Added Mega-only PlatformIO dependencies for `LiquidCrystal_I2C`, `Encoder`,
  and `Bounce2`.
- Added a project `CharacterDisplay` interface and a testable `LcdStatusView`
  for the initial Romanian 4x20 status screen.
- Added a bottom-right custom heartbeat glyph.
- Wired the Mega firmware to initialize the LCD, refresh the status screen
  cooperatively, scan encoder/button input cooperatively, and log input events.
- Added temporary bench pin defaults for encoder, buzzer, and LCD backlight
  while leaving heater/fan relays unassigned.
- Updated backlog, hardware notes, UI draft, test plan, and logbook.

**Decisions**
- Use proven Arduino libraries for physical LCD, encoder, and button debounce
  behavior, while keeping the project-owned display renderer testable without
  Arduino headers.
- Keep the first LCD screen minimal and status-focused; full menu and preset
  configuration remain separate Phase 7 work.
- Keep heater/fan relay pins unassigned during this bring-up because the test
  device has no heater/fan connected and wiring is not finalized.
- Use temporary UI bench pins in centralized hardware configuration so real
  wiring changes require one small config edit later.

**Commits / Branches**
- Branch: `feature/lcd-encoder-bringup`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 135 tests passed.
- `platformio run -e megaatmega2560`: success.
- `platformio run -e megaatmega2560 -t upload`: success after closing the
  serial monitor that was holding the port.
- User bench check: LCD status screen, heartbeat, and encoder/button serial
  events worked OK on the connected test device.
- Source scan found no project use of Arduino `String`, `new`, `delete`,
  `malloc`, or `free`.

**Links**
- `platformio.ini`
- `src/main.cpp`
- `include/dehydrator/interfaces/CharacterDisplay.h`
- `include/dehydrator/ui/LcdStatusView.h`
- `test/test_lcd_status_view/test_lcd_status_view.cpp`
- `include/dehydrator/config/HardwareConfig.h`
- `docs/backlog.md`
- `docs/hardware.md`
- `docs/user-ui-ro.md`
- `docs/test-plan.md`

### 2026-06-04 - PT50 Status Bring-Up

**Context**
After the LCD/encoder firmware was uploaded, the user confirmed that the status
screen, heartbeat, and input events worked OK on the test device. The next
useful hardware slice is to put a real PT50 ADC reading onto the LCD and
structured serial state logs.

**Trigger**
The user said the bench test was OK, asked to commit and merge, and then
proceed with the next step.

**Participants**
- User
- Codex

**Actions**
- Created `feature/sensor-status-bringup`.
- Added a concrete Arduino `analogRead()` adapter for the project
  `AnalogInput` interface.
- Added a cooperative 1-second PT50 sample task in the firmware shell.
- Updated LCD status rendering to show the latest valid PT50 temperature.
- Added compact bring-up state logs with PT50 temperature or `null` and raw ADC
  count.
- Added native log/config tests for the new state format and scheduler
  intervals.
- Restored the temporary LCD backlight pin to numeric `7U` because the shared
  hardware config must not depend on Arduino-only `LED_BUILTIN` in native tests.
- Updated backlog, hardware notes, test plan, and logbook.

**Decisions**
- Keep PT50 sampling separate from LCD refresh so ADC sampling, display updates,
  and input scanning remain independent cooperative tasks.
- Treat the Arduino analog adapter as implemented for bring-up, while keeping
  divider wiring and calibration confirmation as a pending bench task before
  heater-control use.
- Leave AHT concrete integration for a later slice because the exact AHT module
  and library still need confirmation.

**Commits / Branches**
- Branch: `feature/sensor-status-bringup`
- Commit: pending
- Merge commit: pending

**Verification**
- Initial `platformio test -e native` run exposed the Arduino-only
  `LED_BUILTIN` config issue; fixed by using numeric `7U`.
- `platformio test -e native`: 138 tests passed.
- `platformio run -e megaatmega2560`: success.
- `platformio run -e megaatmega2560 -t upload`: success.
- Short serial monitor capture showed expected bring-up records, including
  `STATE app=bringup ... pt50=93 adc=414` and
  `STATE app=bringup ... pt50=null adc=268`.
- Source scan found no project use of Arduino `String`, `new`, `delete`,
  `malloc`, or `free`.

**Links**
- `include/dehydrator/hardware/ArduinoAnalogInput.h`
- `src/main.cpp`
- `include/dehydrator/logging/LogFormatter.h`
- `docs/backlog.md`
- `docs/hardware.md`
- `docs/test-plan.md`

### 2026-06-04 - Concrete AHT Bring-Up

**Context**
PT50 calibration is postponed because the required hardware is not yet
available. The next useful hardware step is to stop treating AHT as an abstract
interface only and wire a real Arduino library into the existing LCD and log
pipeline.

**Trigger**
The user said calibration can wait and asked to proceed with AHT.

**Participants**
- User
- Codex

**Actions**
- Created `feature/aht-concrete-bringup`.
- Chose `Adafruit_AHTX0` as the concrete Arduino AHT20/AHT21 bring-up library.
- Added an Arduino AHT sensor driver that implements the project
  `AhtSensorDriver` interface.
- Added a 2-second AHT sample task beside the existing 1-second PT50 task.
- Wired RH into the LCD status snapshot.
- Extended the periodic bring-up state log to include AHT temperature and RH or
  stable `null` tokens when unavailable.
- Added startup sensor events for `aht_ready` and `aht_missing`.
- Updated backlog, hardware notes, test plan, and logbook.

**Decisions**
- Reuse the existing `AhtReader` abstraction instead of mixing Adafruit library
  calls directly into the UI/logging code.
- Treat missing AHT hardware as a non-fatal bring-up condition: the firmware
  logs `aht_missing` and keeps RH unavailable instead of faulting.
- Sample AHT every 2 seconds, matching the previously agreed slower cadence for
  the humidity sensor.

**Commits / Branches**
- Branch: `feature/aht-concrete-bringup`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 139 tests passed.
- `platformio run -e megaatmega2560`: success.
- `platformio run -e megaatmega2560 -t upload`: success.
- Short serial monitor capture on the current bench showed
  `EVENT type=sensor detail=aht_missing` followed by periodic
  `STATE app=bringup ... aht_t=null rh=null`, which matches the expected
  behavior when no AHT module is attached.
- Source scan found no project use of Arduino `String`, `new`, `delete`,
  `malloc`, or `free`.

**Links**
- `include/dehydrator/hardware/ArduinoAhtSensorDriver.h`
- `include/dehydrator/interfaces/AhtSensorDriver.h`
- `include/dehydrator/sensors/AhtReader.h`
- `src/main.cpp`
- `platformio.ini`

### 2026-06-04 - UI Menu Shell

**Context**
The LCD, encoder, PT50 path, and concrete AHT path are all alive on the bench.
The next vertical slice is to give the encoder a visible UI job before wiring
real dehydrator run configuration and presets.

**Trigger**
The user said "let's proceed" after the concrete AHT bring-up was merged.

**Participants**
- User
- Codex

**Actions**
- Created `feature/ui-menu-shell`.
- Added a pure `MenuController` for status/menu screen state, selection
  movement, short-press selection, and long-press back behavior.
- Added `LcdMenuView` for the Romanian 4x20 menu shell.
- Wired `main.cpp` so short press opens the menu, encoder rotation moves the
  highlighted item, short press logs selection, and long press returns to the
  status screen.
- Kept the bottom-right heartbeat active on the menu screen.
- Updated backlog, UI notes, test plan, and logbook.

**Decisions**
- Keep this slice as a navigation shell only; selecting a menu item logs the
  choice but does not yet launch full workflows.
- Keep menu labels Romanian on the LCD but use separate underscore tokens in
  logs so serial output remains stable and parseable.
- Reuse the existing LCD renderer interface rather than adding Arduino LCD calls
  directly to menu logic.

**Commits / Branches**
- Branch: `feature/ui-menu-shell`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 146 tests passed.
- `platformio run -e megaatmega2560`: success.
- `platformio run -e megaatmega2560 -t upload`: success.
- Short serial monitor capture showed normal startup logs after flashing:
  `aht_missing`, `boot`, and periodic `STATE app=bringup ...` records.
- Menu-specific encoder/button interaction is implemented and covered by native
  tests; manual bench exercise of menu navigation remains pending.

**Links**
- `include/dehydrator/ui/MenuController.h`
- `include/dehydrator/ui/LcdMenuView.h`
- `src/main.cpp`
- `docs/user-ui-ro.md`

### 2026-06-04 - Menu Edges And No-Wrap Behavior

**Context**
The first menu shell was usable on hardware, but the centered scrolling and
wraparound behavior felt chaotic when the list was short. The user wanted the
active marker to stay on the first visible line and the menu to stop at the
top and bottom instead of wrapping.

**Trigger**
The user reported that the rotary control itself was fine now and asked to
proceed with the previous request about menu behavior.

**Participants**
- User
- Codex

**Actions**
- Changed the menu controller so encoder rotation stops at the first and last
  entries instead of wrapping around.
- Changed the menu view so the selected item is always rendered on the first
  visible line, with following entries below it.
- Updated native tests to cover the no-wrap edge behavior and the first-line
  rendering layout.
- Updated the Romanian UI draft and backlog entry for the menu flow.

**Decisions**
- Prefer a stable first-line selection marker over centered scrolling for this
  short menu.
- Keep the navigation simple and predictable for the current 5-item list.

**Commits / Branches**
- Branch: `feature/menu-scroll-behavior`
- Commit: pending
- Merge commit: pending

**Verification**
- Code patched locally.
- PlatformIO test/build rerun was started but interrupted by the environment
  cache permissions issue before completion.

**Links**
- `include/dehydrator/ui/MenuController.h`
- `include/dehydrator/ui/LcdMenuView.h`
- `test/test_menu_controller/test_test_menu_controller.cpp`
- `test/test_lcd_menu_view/test_lcd_menu_view.cpp`
- `docs/user-ui-ro.md`

### 2026-06-04 - Preset Selection Shell

**Context**
The menu and manual mode shells were in place, but `Pornire preset` still did
not lead anywhere useful. The next step was to add a real preset-selection
screen with a starter catalog and a confirm action.

**Trigger**
The user asked to proceed with the next step after confirming the menu was OK.

**Participants**
- User
- Codex

**Actions**
- Added a small built-in preset catalog with starter values for `Mere`,
  `Ierburi`, `Jerky`, and `Iaurt`.
- Added `PresetSelectController` for navigation, confirm, and return-to-menu
  behavior.
- Added `LcdPresetView` for the Romanian 4x20 preset screen.
- Wired `main.cpp` so selecting `Pornire preset` opens the preset screen and
  confirming a preset logs the choice and returns to the status screen.
- Added native tests for the preset controller and preset view.
- Updated backlog and Romanian UI notes.

**Decisions**
- Keep the preset flow as a shell for now: the user can choose a preset and the
  firmware logs the selection, but the full run-start wiring remains a separate
  later slice.
- Keep the preset values centralized in a dedicated catalog so they can be
  revised from a PDF or bench results without changing UI logic.

**Commits / Branches**
- Branch: `feature/preset-run-config`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 164 tests passed.
- `platformio run -e megaatmega2560`: success.

**Links**
- `include/dehydrator/presets/PresetCatalog.h`
- `include/dehydrator/ui/PresetSelectController.h`
- `include/dehydrator/ui/LcdPresetView.h`
- `src/main.cpp`
- `docs/user-ui-ro.md`

### 2026-06-04 - Manual Mode Shell

**Context**
The menu shell is in place, but selecting `Mod manual` only logged a choice.
The next small vertical slice is to make that menu item open a real editable
screen with visible output state and the agreed heater/fan manual constraints.

**Trigger**
The user said "let's go" after the menu shell was merged.

**Participants**
- User
- Codex

**Actions**
- Created `feature/manual-mode-shell`.
- Added a pure `ManualModeController` that owns logical manual output commands.
- Added `LcdManualView` for the Romanian 4x20 manual screen.
- Wired `main.cpp` so selecting `Mod manual` opens the manual screen, encoder
  rotation switches selected field, short press toggles the selected output,
  and long press returns to the menu.
- Reused `sanitizeOutputCommand()` so heater/fan safety stays consistent with
  the rest of the codebase.
- Reflected manual logical output states on the status screen as well.
- Updated backlog, UI notes, test plan, and logbook.

**Decisions**
- Keep manual mode as a logical output shell for now; real relay writing is
  still separate hardware-adapter work.
- Use `Fan` and `Heat` as the only editable fields in this slice.
- Enforce the simple safety rule in manual mode immediately:
  heater ON forces fan ON, and fan OFF forces heater OFF.

**Commits / Branches**
- Branch: `feature/manual-mode-shell`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 152 tests passed.
- `platformio run -e megaatmega2560`: success.
- `platformio run -e megaatmega2560 -t upload`: success.
- Short serial monitor capture showed clean startup after flashing:
  `aht_missing`, `boot`, and periodic `STATE app=bringup ...` records.
- Manual-mode encoder/button interaction is implemented and covered by native
  tests; the quick human bench exercise of opening `Mod manual` and toggling
  `Fan`/`Heat` remains pending.

**Links**
- `include/dehydrator/ui/ManualModeController.h`
- `include/dehydrator/ui/LcdManualView.h`
- `src/main.cpp`
- `docs/user-ui-ro.md`
