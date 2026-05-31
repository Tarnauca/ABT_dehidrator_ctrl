# ADR-003: Documentation And Agent Workflow

## Status

Accepted.

## Context

The user wants agentic development but also wants persistent project memory after devcontainer rebuilds or Codex session loss.

## Decision

Use repo documentation as the source of truth.

Maintain:

- `docs/project-context.md`
- `docs/project-discovery-log.md`
- `docs/backlog.md`
- requirements, risks, test plan, hardware, UI, coding standards, and ADRs
- reusable agent definitions in `docs/agents/`

Agent invocation is hybrid:

- user can manually invoke agents,
- Codex can recommend or trigger agents at natural checkpoints.

Documentation impact check is required before branch merge and when changes affect requirements, architecture, safety, tests, UI, persistence, hardware assumptions, or external interfaces.

## Consequences

- Future sessions can recover by reading docs.
- Agents have stable instructions.
- Documentation maintenance becomes part of the development workflow.
