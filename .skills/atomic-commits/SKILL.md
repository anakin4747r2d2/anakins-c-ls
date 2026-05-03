---
name: atomic-commits
description: Atomic git commit workflow and best practices. Use whenever saving code changes, completing a task, or fixing a mistake introduced in the previous commit.
---

## Atomic Commits

Every meaningful change must be saved as a commit. Never leave the working tree dirty at the end of a task.

### What is an atomic commit?

An atomic commit contains exactly one logical change — it should be possible to revert it independently without breaking unrelated functionality.

- One feature addition = one commit
- One bug fix = one commit
- One refactor = one commit
- Do **not** bundle unrelated changes together

### Commit message format

Follow the Conventional Commits style used in this repo:

```
<type>: <short imperative summary>
```

| Type | When to use |
|------|-------------|
| `feat` | New feature or behaviour |
| `fix` | Bug fix |
| `refactor` | Code change that neither fixes a bug nor adds a feature |
| `test` | Adding or updating tests |
| `chore` | Tooling, deps, config |
| `ci` | CI/CD changes |
| `docs` | Documentation only |

Rules:
- Summary is lowercase, imperative mood ("add", not "added" or "adds")
- Max ~72 characters
- No period at the end

### When to squash instead of creating a new commit

If the change you are about to commit **corrects a mistake introduced in the immediately preceding commit** (e.g. a typo introduced moments ago is corrected, a broken implementation just committed now works), **squash it** into that commit rather than adding noise to the history.

Only squash when ALL of these are true:
1. The previous commit was made in this session (you authored it).
2. The previous commit has **not** been pushed to the remote.
3. The fix has no independent value as its own history entry.

```sh
# stage your fix, then amend the previous commit
git add <files>
git commit --amend --no-edit
```

If any condition is not met, create a new `fix:` commit instead.

### Workflow

1. Complete a logical unit of work (guided by the TDD skill where applicable).
2. Stage only the files relevant to that unit: `git add <files>` (avoid blanket `git add .`).
3. Check what you are about to commit: `git diff --staged`.
4. Determine whether this corrects the immediately preceding commit (see squash rules above).
   - Yes → `git commit --amend --no-edit`
   - No → `git commit -m "<type>: <summary>"`
5. Verify: `git log --oneline -5`
