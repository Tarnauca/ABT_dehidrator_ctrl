# Agent Workflow

Status: draft baseline.

## Purpose

Agents are reusable specialist roles. Codex acts as coordinator: it clarifies goals, decides when delegation is useful, integrates results, and keeps the project consistent.

## Invocation Style

The project uses a hybrid model:

- The user may manually request an agent by name.
- Codex may recommend or invoke relevant agents at natural checkpoints.
- Agents should be used for bounded tasks with clear inputs and expected outputs.

Example manual requests:

- `Run the Safety Reviewer on the current requirements.`
- `Ask the Test Engineer how to test the heater control module.`
- `Use the Dev Environment Auditor to inspect PlatformIO config.`

## Default Checkpoints

- After requirements changes: Requirements Analyst and Safety Reviewer.
- After architecture changes: System Architect and Test Engineer.
- Before modifying environment/tooling: Dev Environment Auditor.
- Before implementing a module: Firmware Developer with bounded ownership.
- After implementation: Test Engineer and relevant reviewer.
- Before branch merge: Documentation Maintainer and documentation impact check.

## Permission Model

- Requirements Analyst: review-only by default; may edit docs only when assigned.
- System Architect: review-only by default; may edit docs only when assigned.
- Safety Reviewer: review-only by default.
- Dev Environment Auditor: review-only by default; environment/tooling edits require explicit user confirmation.
- Firmware Developer: may edit code only when assigned bounded file/module ownership.
- Test Engineer: may edit tests only when assigned.
- Documentation Maintainer: may edit docs when assigned.

Workers are not alone in the codebase. They must not revert edits made by others and must adapt to existing changes.

## Standard Review Output

Use this format for review agents:

```text
Summary
Findings
Recommendations
Required Updates
Open Questions
Files Changed
```

## Standard Worker Output

Use this format for implementation agents:

```text
Summary
Implementation Notes
Verification
Required Updates
Open Questions
Files Changed
```

## Documentation Impact Check

Required before branch merge and whenever a commit clearly affects requirements, architecture, safety, tests, UI, persistence, hardware assumptions, or external interfaces.

Checklist:

- Backlog updated or confirmed unchanged.
- Requirements updated or confirmed unchanged.
- ADR needed/updated or confirmed unnecessary.
- Risk log updated or confirmed unchanged.
- Test plan updated or confirmed unchanged.
- UI/user docs updated or confirmed unchanged.
- Hardware/dev environment docs updated or confirmed unchanged.

## Backlog Impact Rule

Any branch that completes, starts, splits, removes, or materially changes planned work must check `docs/backlog.md`.

- If a backlog item is completed, mark it complete in the same branch.
- If new work is discovered, add or adjust backlog items in the same branch.
- If no backlog change is needed, say so in the final branch summary.
- The Documentation Maintainer is responsible for checking backlog impact before merge.

## Session Recovery

For a new Codex session:

1. Read `docs/project-context.md`.
2. Read `docs/backlog.md`.
3. Check `git status` and current branch.
4. Continue from the active backlog item.
