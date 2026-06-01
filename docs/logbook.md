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

**Decisions**
- Placeholder wiring values are centralized but must be reviewed before real
  heater/fan hardware is connected.
- Relay polarity is represented in config early, even though relay adapters are
  not implemented yet.
- Runtime constants used by the current shell are centralized now so future
  modules do not grow scattered local constants.

**Commits / Branches**
- Branch: `feature/hardware-config-placeholders`
- Commit: pending
- Merge commit: pending

**Verification**
- `platformio test -e native`: 18 tests passed.
- `platformio run -e megaatmega2560`: success.

**Links**
- `include/dehydrator/config/HardwareConfig.h`
- `include/dehydrator/config/RuntimeConfig.h`
- `test/test_config/test_config.cpp`
- `src/main.cpp`
- `docs/backlog.md`
