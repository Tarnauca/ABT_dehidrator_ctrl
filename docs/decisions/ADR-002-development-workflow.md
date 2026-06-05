# ADR-002: Development Workflow

## Status

Accepted.

## Context

The user wants to learn professional software development practices while building the dehydrator controller. The project should have clean history, reviewable changes, and durable documentation.

## Decision

Use a branch/commit based workflow.

The first documentation baseline branch/commit/push may be performed by Codex. After that, the user will perform Git operations manually for a while under Codex supervision.

Use focused branches and commits. Before committing, run:

```bash
git status
git diff
git add <explicit paths>
git commit -m "<message>"
```

Before starting a new user request, check `git status`.

- If the worktree is clean, continue normally.
- If the worktree has uncommitted changes and the new request is clearly in the
  same change scope, it is acceptable to continue.
- If the worktree has uncommitted changes and the new request is not clearly in
  the same scope, stop and ask whether to finish/commit/merge the current work
  first or intentionally continue with mixed changes.

## Consequences

- The user learns Git workflow directly.
- Changes remain reviewable and reversible.
- Codex must summarize recommended Git commands clearly.
- Unrelated changes should not be committed accidentally.
- Scope drift should be caught before a second topic is mixed into the same
  branch by accident.
