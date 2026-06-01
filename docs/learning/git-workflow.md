# Git Workflow Learning Notes

## `main` And `origin`

`main` is the local primary branch. It is the branch checked out in your local
repository or devcontainer.

`origin` is the default remote repository name, usually GitHub.

Common meanings:

- `main`: local branch.
- `origin/main`: local record of the remote `main` branch from the last fetch/pull.
- `main...origin/main`: local `main` is tracking remote `origin/main`.

Useful status hints:

- `ahead`: local commits exist that are not pushed.
- `behind`: remote commits exist that are not pulled.
- no ahead/behind marker: local and remote tracking branches are in sync.

## Branches

A branch is an isolated line of work. Use branches to keep `main` stable while a
focused change is developed.

Recommended project prefixes:

- `docs/...`: documentation and workflow updates.
- `feature/...`: new firmware or project capability.
- `fix/...`: bug fixes.
- `chore/...`: maintenance.
- `experiment/...`: throwaway exploration.

## Small Branch Pattern

This is the common pattern used in this project, even for a small documentation
change:

```bash
git switch -c docs/update-backlog-after-skeleton
git status
git add docs/backlog.md
git commit -m "docs(backlog): update progress after firmware skeleton"
git push -u origin docs/update-backlog-after-skeleton

git switch main
git pull
git merge --no-ff docs/update-backlog-after-skeleton -m "Merge backlog update after firmware skeleton"
git push
git status
```

What happens:

1. `git switch -c docs/update-backlog-after-skeleton`
   Creates a new local branch and switches to it. The branch name says this is a
   documentation change related to backlog progress.

2. `git status`
   Shows the current branch and pending changes. This is the safety check before
   staging anything.

3. `git add docs/backlog.md`
   Stages only the intended file. This avoids accidentally committing unrelated
   files.

4. `git commit -m "..."`
   Saves a local checkpoint with a semantic commit message. `docs(backlog)`
   means this is a documentation change scoped to the backlog.

5. `git push -u origin docs/update-backlog-after-skeleton`
   Pushes the branch to GitHub and sets the local branch to track the remote
   branch. After `-u`, later pushes from that branch can usually be just
   `git push`.

6. `git switch main`
   Returns to the local stable branch.

7. `git pull`
   Updates local `main` from GitHub before merging. This reduces surprises if
   remote `main` changed.

8. `git merge --no-ff ... -m "..."`
   Merges the branch into `main` with an explicit merge commit. `--no-ff`
   preserves the branch as a visible unit of work in history. `-m` avoids opening
   the editor.

9. `git push`
   Pushes the updated `main` branch to GitHub.

10. `git status`
    Final safety check. The desired result is a clean `main` tracking
    `origin/main`.

Why use this pattern for small changes?

- It keeps `main` stable while work is in progress.
- It makes even small changes reviewable.
- It creates clean history with a visible branch merge.
- It makes it harder to commit unrelated files by accident.
- It reinforces the same workflow used for larger feature branches.

## Commit Parking Versus Stash Parking

If work is coherent enough to save, commit it on its branch. This is the
preferred way to park work.

If work is messy or temporary, stash it:

```bash
git stash push -m "WIP description"
```

Later:

```bash
git stash pop
```

Commit parking is easier to inspect and push as backup. Stash parking is useful
for quick context switches but is easier to forget and may conflict when popped.

## Merge Commits

This project usually merges branches with `--no-ff` so milestones remain visible
in history:

```bash
git merge --no-ff feature/example -m "merge(example): merge example feature"
```

The `-m` avoids opening the editor, which is helpful in this devcontainer setup.

Many teams leave merge commits in Git's default wording, such as
`Merge feature/example`. This project uses a semantic-style merge convention for
consistency:

```text
merge(scope): short merge summary
```

Example:

```bash
git merge --no-ff feature/logging-abstraction -m "merge(logging): merge logging abstraction"
```

This keeps merge commits visually distinct while still following the same
`type(scope): summary` shape as normal commits.

## Semantic Commits

Use:

```text
type(scope): summary
```

The summary should be short and imperative, as if completing the sentence
"This commit will ...".

Examples:

- `docs(workflow): add development logbook`
- `feat(logging): add mirrored log dispatcher`
- `feat(config): add hardware placeholders`
- `test(domain): add state transition tests`

Common types:

- `docs`
- `feat`
- `fix`
- `test`
- `refactor`
- `chore`
- `build`
- `ci`
