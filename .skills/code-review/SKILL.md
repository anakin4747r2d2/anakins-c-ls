---
name: code-review
description: Post-commit code review checklist. Use after every commit to verify cleanliness, atomicity, and consistency of the change before moving on.
---

## Code Review Workflow

After creating a commit, review the diff against the checklist below. For every violation found, fix it and create a new commit before continuing.

```sh
# Review what was just committed
git diff HEAD~1 HEAD
```

### Checklist

**Dead code**
The change must not leave behind any unreachable code, unused variables, unused imports, or functions that are no longer called.

**Comment validity**
Every comment that existed before the change still accurately describes the code it annotates. Comments made obsolete by the change are updated or removed.

**Codebase style**
Naming conventions, file structure, formatting, and patterns match what is already present in the codebase. No new conventions are introduced without justification.

**Guard clauses over nesting**
Where a condition gates the entire body of a block, a guard clause with an early return is used instead of wrapping the body in an `if`. Nesting depth is kept minimal.

**Extract functions for readability**
Any block of code that can be given a clear, descriptive name is extracted into its own function. Inline logic is kept only when it is trivially obvious.

**Atomic commit scope**
The commit contains exactly one logical change. Nothing unrelated has been staged alongside the intended change.

**No accidental inclusions**
The diff contains no debugging statements, commented-out experiments, whitespace-only reformats, or changes to unrelated files that crept in unintentionally.

**Reuse existing functionality**
Before introducing new logic, check whether the codebase already provides an equivalent utility, hook, component, or helper. New code defers to existing implementations wherever possible.

**No duplicated functionality**
The commit does not reimplement something that already exists elsewhere in the codebase, even if the existing implementation is in a different module or file.

**Simplest viable solution**
The implementation chosen is the simplest one that correctly satisfies the requirement. No unnecessary abstractions, generalisations, or indirections are added.

**Every new code path has a test**
Any behaviour added or changed by the commit is covered by at least one test. No extra functionality is introduced without a corresponding test.

### Fixing violations

Each fix is a separate commit following the atomic-commits workflow:

```
refactor: remove dead code left after <summary>
fix: update stale comment in <file>
refactor: extract <name> function for readability
```

Do not amend the original commit unless the atomic-commits squash rules explicitly allow it.
